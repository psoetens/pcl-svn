#include "proctor/scanning_model_source.h"

#include <cstdio>
#include <vector>
#include <sstream>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <pcl/pcl_base.h>
#include <pcl/filters/filter.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/ply_io.h>

#include "proctor/config.h"

namespace pcl
{

  namespace proctor
  {

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

    IndicesPtr
    randomSubset(int n, int r)
    {
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

    void
    ScanningModelSource::loadModels()
    {
      int max_models = 1814;
      srand(0);
      IndicesPtr model_subset = randomSubset(max_models, Config::num_models);

      for (int mi = 0; mi < Config::num_models; mi++) {
        int id = (*model_subset)[mi];
        std::stringstream path;
        FILE *file;
        int vertices, faces, edges;

        // Put model in map
        Model* new_model = new Model;
        std::stringstream ss;
        ss << name_ << id;
        std::string new_id = ss.str();

        // read mesh
        new_model->id = id;
        path << dir_ << "/" << id / 100 << "/m" << id << "/m" << id << ".off";
        file = fopen(path.str ().c_str (), "r");

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
          if (fscanf (file, "3 %lld %lld %lld\n", (long long int*)&f[j][1], (long long int*)&f[j][2], (long long int*)&f[j][3]) != 3) 
          {
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
        path.str ("");
        path << dir_ << "/" << id / 100 << "/m" << id << "/m" << id << "_info.txt";
        file = fopen(path.str ().c_str (), "r");
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

        models_[new_id] = *new_model;
      } // end for over models
    }

    void
    ScanningModelSource::getModelIDs(std::vector<std::string> &output)
    {
      std::map<std::string, Model>::iterator it;
      output.clear();

      for ( it=models_.begin() ; it != models_.end(); it++ ) {
        output.push_back((*it).first);
      }

      sort(output.begin(), output.end());
    }

    PointCloud<PointNormal>::Ptr
    ScanningModelSource::getTrainingModel(std::string model_id)
    {
      PointCloud<PointNormal>::Ptr full_cloud(new PointCloud<PointNormal>());

      for (int ti = 0; ti < theta_count; ti++) {
        for (int pi = 0; pi < phi_count; pi++) {
          float theta = Scanner::theta_start + ti * Scanner::theta_step;
          float phi = Scanner::phi_start + pi * Scanner::phi_step;

          *full_cloud += *Scanner::getCloudCached(theta, phi, models_[model_id]);
          flush(cout << '.');
        }
      }
      cout << endl;

      // Remove NaNs
      PointCloud<PointNormal>::Ptr dense_cloud (new PointCloud<PointNormal>());

      std::vector<int> index;
      pcl::removeNaNFromPointCloud<PointNormal>(*full_cloud, *dense_cloud, index);

      PointCloud<PointNormal>::Ptr cloud_subsampled (new PointCloud<PointNormal> ());
      VoxelGrid<PointNormal> subsampling_filter;
      subsampling_filter.setInputCloud(dense_cloud);
      subsampling_filter.setLeafSize(0.01, 0.01, 0.01);
      subsampling_filter.filter(*cloud_subsampled);

      return cloud_subsampled;
    }

    PointCloud<PointNormal>::Ptr
    ScanningModelSource::getTestModel(std::string model_id)
    {
      boost::uniform_real<float> theta_u(theta_min, theta_max);
      boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > theta_gen(rng_, theta_u);

      boost::uniform_real<float> phi_u(phi_min, phi_max);
      boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > phi_gen(rng_, phi_u);

      float theta = theta_gen();
      float phi = phi_gen();

      PointCloud<PointNormal>::Ptr test_scan = Scanner::getCloudCached(theta, phi, models_[model_id]);
      return test_scan;
    }
  }

}
