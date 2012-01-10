#include "proctor/scanning_model_source.h"

#include <cstdio>
#include <vector>
#include <sstream>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <pcl/pcl_base.h>

#include "proctor/config.h"

namespace pcl {

  namespace proctor {

    const float theta_start = M_PI / 12;
    const float theta_step = 0.0f;
    const int theta_count = 1;
    const float phi_start = 0.0f;
    const float phi_step = M_PI / 6;
    const int phi_count = 12;
    const float theta_min = 0.0f;
    const float theta_max = M_PI / 6;
    const float phi_min = 0.0f;
    const float phi_max = M_PI * 2;

    IndicesPtr randomSubset(int n, int r) {
      IndicesPtr subset (new std::vector<int>());
      std::vector<int> bag (n);
      for (int i = 0; i < n; i++) bag[i] = i;
      int edge = n;
      subset->resize(r);
      for (int i = 0; i < r; i++) {
        int pick = rand() % edge;
        (*subset)[i] = bag[pick];
        bag[pick] = bag[--edge];
      }
      return subset;
    }

    void ScanningModelSource::loadModels() {
      int max_models = 1814;
      srand(0);
      IndicesPtr model_subset = randomSubset(max_models, Config::num_models);

      for (int mi = 0; mi < Config::num_models; mi++) {
        int id = (*model_subset)[mi];
        char path[256];
        FILE *file;
        int vertices, faces, edges;

        // Put model in map
        Model* new_model = new Model;
        std::stringstream ss;
        ss << name << id;
        std::string new_id = ss.str();

        // read mesh
        new_model->id = id;
        snprintf(path, sizeof(path), "%s/%d/m%d/m%d.off", dir.c_str(), id / 100, id, id);
        file = fopen(path, "r");

        if (file == NULL) {
          cerr << "Could not find " << path << endl;
          exit(-1);
        }

        int line = 1;

        // read header
        if (fscanf(file, "OFF\n%d %d %d\n", &vertices, &faces, &edges) != 3) {
          cerr << "invalid OFF header in file " << path << endl;
          exit(-1);
        } else {
          line += 2;
        }

        // read vertices
        vtkFloatArray *fa = vtkFloatArray::New();
        fa->SetNumberOfComponents(3);
        float (*v)[3] = reinterpret_cast<float (*)[3]>(fa->WritePointer(0, 3 * vertices));
        for (int j = 0; j < vertices; j++) {
          if (fscanf(file, "%f %f %f\n", &v[j][0], &v[j][1], &v[j][2]) != 3) {
            cerr << "invalid vertex in file " << path << " on line " << line << endl;
            exit(-1);
          } else {
            ++line;
          }
        }
        vtkPoints *p = vtkPoints::New();
        p->SetData(fa); fa->Delete();

        // read faces
        vtkCellArray *ca = vtkCellArray::New();
        vtkIdType (*f)[4] = reinterpret_cast<vtkIdType (*)[4]>(ca->WritePointer(faces, 4 * faces));
        for (int j = 0; j < faces; j++) {
          f[j][0] = 3; // only supports triangles...
          if (fscanf(file, "3 %lld %lld %lld\n", &f[j][1], &f[j][2], &f[j][3]) != 3) {
            cerr << "invalid face in file " << path << " on line " << line << endl;
            exit(-1);
          } else {
            ++line;
          }
        }

        fclose(file);

        new_model->mesh = vtkPolyData::New(); // lives forever
        new_model->mesh->SetPoints(p); p->Delete();
        new_model->mesh->SetPolys(ca); ca->Delete();

        // read metadata
        snprintf(path, sizeof(path), "%s/%d/m%d/m%d_info.txt", dir.c_str(), id / 100, id, id);
        file = fopen(path, "r");
        while (!feof(file)) {
          char buf[256];
          if (fgets(buf, sizeof(buf), file) != NULL) {
            if (!strncmp("center: ", buf, 8)) {
              if (sscanf(buf, "center: (%f,%f,%f)\n", &new_model->cx, &new_model->cy, &new_model->cz) != 3) {
                cerr << "invalid centroid in file " << path << endl;
                cerr << buf;
                exit(-1);
              }
            } else if (!strncmp("scale: ", buf, 7)) {
              if (sscanf(buf, "scale: %f\n", &new_model->scale) != 1) {
                cerr << "invalid scale in file " << path << endl;
                cerr << buf;
                exit(-1);
              }
            }
          }
        } // end while over info lines
        fclose(file);

        models[new_id] = *new_model;
      } // end for over models
    }

    void ScanningModelSource::getModelIDs(std::vector<std::string> &output) {
      std::map<std::string, Model>::iterator it;
      output.clear();

      for ( it=models.begin() ; it != models.end(); it++ ) {
        output.push_back((*it).first);
      }

      sort(output.begin(), output.end());
    }

    PointCloud<PointNormal>::Ptr ScanningModelSource::getTrainingModel(std::string model_id) {
      PointCloud<PointNormal>::Ptr full_cloud(new PointCloud<PointNormal>());

      for (int ti = 0; ti < theta_count; ti++) {
        for (int pi = 0; pi < phi_count; pi++) {
          *full_cloud += *Scanner::getCloudCached(-1, ti, pi, models[model_id]);
          flush(cout << '.');
        }
      }
      cout << endl;

      return full_cloud;
    }

    PointCloud<PointNormal>::Ptr ScanningModelSource::getTestModel(std::string model_id) {
      const float theta_scale = (theta_max - theta_min) / RAND_MAX;
      const float phi_scale = (phi_max - phi_min) / RAND_MAX;

      int theta = theta_min + rand() * theta_scale + 5;
      int phi = phi_min + rand() * phi_scale + 5;

      PointCloud<PointNormal>::Ptr test_scan = Scanner::getCloudCached(-1, theta, phi, models[model_id]);
      return test_scan;
    }
  }

}
