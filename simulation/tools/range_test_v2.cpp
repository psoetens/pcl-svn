/**
 * This program tests the range image likelihood library.
 *
 * You can move around using
 *  w - forward
 *  a - backward
 *  s - left
 *  d - right
 *  q - up
 *  z - down
 *
 * The mouse controls the direction of movement
 *
 *  Esc - quit
 *
 */

#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <GL/gl.h>

#include <GL/glu.h>
#include <GL/glut.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include "pcl/common/common.h"

#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <pcl/surface/gp3.h>


#include <pcl/io/vtk_lib_io.h>

#include "pcl/simulation/camera.hpp"
#include "pcl/simulation/model.hpp"
#include "pcl/simulation/scene.hpp"
#include "pcl/simulation/range_likelihood.hpp"

#include <pcl/console/print.h>
#include <pcl/console/parse.h>
#include <pcl/console/time.h>

// RangeImage:
#include <pcl/range_image/range_image_planar.h>

// Pop-up viewer
#include <pcl/visualization/cloud_viewer.h>



using namespace Eigen;
using namespace pcl;
using namespace pcl::console;

using namespace std;

uint16_t t_gamma[2048];


Scene::Ptr scene_;
Camera::Ptr camera_;
RangeLikelihood::Ptr range_likelihood_;
int window_width_;
int window_height_;
bool paused_;
bool write_file_;


void
printHelp (int argc, char **argv)
{
  print_error ("Syntax is: %s <options>\n", argv[0]);
  print_info ("  where options are:\n");
  print_info ("        -method X = type of input files [0=stack of pcd files, 1=obj,vtk,ply]\n");
  print_info ("        -map    X = type of input files [path to map file(s)]\n");
//  print_value ("%s", default_field_name.c_str ()); print_info (")\n");
//  print_info ("                     -min X = lower limit of the filter (default: ");
//  print_value ("%f", default_min); print_info (")\n");
//  print_info ("                     -max X = upper limit of the filter (default: ");
//  print_value ("%f", default_max); print_info (")\n");
//  print_info ("                     -inside X = keep the points inside the [min, max] interval or not (default: ");
//  print_value ("%d", default_inside); print_info (")\n");
}

void wait()
{
      std::cout << "Press enter to continue";
      getchar();
      std::cout << "\n\n";
}

void apply_camera_transform(const Camera & camera)
{
  glRotatef(rad_to_deg(-camera.roll()),1.0,0.0,0.0);
  glRotatef(rad_to_deg(-camera.pitch()),0.0,1.0,0.0);
  glRotatef(rad_to_deg(-camera.yaw()),0.0,0.0,1.0);
  glTranslatef(-camera.x(), -camera.y(), -camera.z());
}



void draw_grid(int w, int h, double size)
{
  glBegin(GL_LINES);
  for (int y=0; y<h; ++y) {
      glVertex3f(-size, ((float)y/h-0.5)*2.0 * size, 0.0);
      glVertex3f( size, ((float)y/h-0.5)*2.0 * size, 0.0);
  }

  for (int x=0; x<w; ++x) {
      glVertex3f(((float)x/w-0.5)*2.0 * size, -size, 0.0);
      glVertex3f(((float)x/w-0.5)*2.0 * size,  size, 0.0);
  }
  glEnd();
}

void draw_axis(double x, double y, double z, double yaw, double pitch, double roll, double size)
{
  glPushMatrix();
  glPushAttrib(GL_CURRENT_BIT);

  glTranslatef(x, y, z);

  glRotatef(rad_to_deg(yaw),  0., 0., 1.);
  glRotatef(rad_to_deg(pitch),0., 1., 0.);
  glRotatef(rad_to_deg(roll), 1., 0., 0.);

  glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0); glVertex3f(0.0,0.0,0.0); glVertex3f(size*1.0,0.0,0.0);
    glColor3f(0.0,1.0,0.0); glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,size*1.0,0.0);
    glColor3f(0.0,0.0,1.0); glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,0.0,size*1.0);
  glEnd();

  glPopAttrib();
  glPopMatrix();
}

void draw_cube()
{
  const float light_position[] = {1.0f, 1.0f, 1.0f};
  const float light_color[] = {1.0f, 1.0f, 1.0f};

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_color);
  glColor3f(1.0f,0.0f,0.0f);
  glEnable(GL_LIGHT0);
  glutSolidCube(1.0);
  glPopAttrib();
}

void display_depth_image(const float* depth_buffer)
{
  int npixels = range_likelihood_->width() * range_likelihood_->height();
  uint8_t* depth_img = new uint8_t[npixels * 3];

  for (int i=0; i<npixels; i++) {
            float zn = 0.7;
            float zf = 20.0;
            float d = depth_buffer[i];
            float z = -zf*zn/((zf-zn)*(d - zf/(zf-zn)));
            float b = 0.075;
            float f = 580.0;
            uint16_t kd = static_cast<uint16_t>(1090 - b*f/z*8);
            if (kd < 0) kd = 0;
            else if (kd>2047) kd = 2047;

            int pval = t_gamma[kd];
            int lb = pval & 0xff;
            switch (pval>>8) {
                case 0:
                    depth_img[3*i+0] = 255;
                    depth_img[3*i+1] = 255-lb;
                    depth_img[3*i+2] = 255-lb;
                    break;
                case 1:
                    depth_img[3*i+0] = 255;
                    depth_img[3*i+1] = lb;
                    depth_img[3*i+2] = 0;
                    break;
                case 2:
                    depth_img[3*i+0] = 255-lb;
                    depth_img[3*i+1] = 255;
                    depth_img[3*i+2] = 0;
                    break;
                case 3:
                    depth_img[3*i+0] = 0;
                    depth_img[3*i+1] = 255;
                    depth_img[3*i+2] = lb;
                    break;
                case 4:
                    depth_img[3*i+0] = 0;
                    depth_img[3*i+1] = 255-lb;
                    depth_img[3*i+2] = 255;
                    break;
                case 5:
                    depth_img[3*i+0] = 0;
                    depth_img[3*i+1] = 0;
                    depth_img[3*i+2] = 255-lb;
                    break;
                default:
                    depth_img[3*i+0] = 0;
                    depth_img[3*i+1] = 0;
                    depth_img[3*i+2] = 0;
                    break;

            }

  }

  //  glRasterPos2i(-1,-1);
  //  glDrawPixels(range_likelihood_->width(), range_likelihood_->height(), GL_LUMINANCE, GL_FLOAT, range_likelihood_->depth_buffer());

  glRasterPos2i(-1,-1);
  glDrawPixels(range_likelihood_->width(), range_likelihood_->height(), GL_RGB, GL_UNSIGNED_BYTE, depth_img);

  delete [] depth_img;
}

void display() {
  glViewport(range_likelihood_->width(), 0, range_likelihood_->width(), range_likelihood_->height());
  
  float* reference = new float[range_likelihood_->row_height() * range_likelihood_->col_width()];
  const float* depth_buffer = range_likelihood_->depth_buffer();
  // Copy one image from our last as a reference.
  for (int i=0, n=0; i<range_likelihood_->row_height(); ++i) {
    for (int j=0; j<range_likelihood_->col_width(); ++j) {
      reference[n++] = depth_buffer[i*range_likelihood_->width() + j];
    }
  }

  std::vector<Eigen::Isometry3d, Eigen::aligned_allocator<Eigen::Isometry3d> > poses;
  std::vector<float> scores;
  int n = range_likelihood_->rows()*range_likelihood_->cols();
  for (int i=0; i<n; ++i) {
    Camera camera(*camera_);
    camera.move(0.0,i*0.02,0.0);
    poses.push_back(camera.pose());
  }
  float* depth_field =NULL;
  bool do_depth_field =false;
  range_likelihood_->compute_likelihoods(reference, poses, scores,depth_field,do_depth_field);
//  range_likelihood_->compute_likelihoods(reference, poses, scores);
  delete [] reference;
  delete [] depth_field;

  std::cout << "score: ";
  for (size_t i=0; i<scores.size(); ++i) {
    std::cout << " " << scores[i];
  }
  std::cout << std::endl;

  // Draw the depth image
  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  glColorMask(true, true, true, true);
  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, range_likelihood_->width(), range_likelihood_->height());
  //glViewport(0, 0, range_likelihood_->width(), range_likelihood_->height());

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //glRasterPos2i(-1,-1);
  //glDrawPixels(range_likelihood_->width(), range_likelihood_->height(), GL_LUMINANCE, GL_FLOAT, range_likelihood_->depth_buffer());
  display_depth_image(range_likelihood_->depth_buffer());
  glutSwapBuffers();
  
  if (write_file_){
    //camera_width_in 640;
    //camera_height_in 480
    //float  camera_fx_in= 576.09757860;
    //float camera_fy_in=	    576.09757860;
    //float camera_cx_in= 321.06398107; 
    //float camera_cy_in= 242.97676897;
    
    range_likelihood_->addNoise();
    pcl::RangeImagePlanar rangeImage;
    range_likelihood_->getRangeImagePlanar(rangeImage);
    std::cout << rangeImage << " ---  range image info\n";

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_out (new pcl::PointCloud<pcl::PointXYZRGB>);
    range_likelihood_->getPointCloud(pc_out);
    
    pcl::PCDWriter writer;
    writer.write ("simulated_range_image.pcd", *pc_out, false);  
    cout << "finished writing file\n";
    
    pcl::visualization::CloudViewer viewer ("Simple Cloud Viewer");
    viewer.showCloud (pc_out);

    // Problem: vtk and opengl dont seem to play very well together
    // vtk seems to misbehavew after a little while and wont keep the window on the screen

    // method1: kill with [x] - but eventually it crashes:
    //while (!viewer.wasStopped ()){
    //}    
    
    // method2: eventually starts ignoring cin and pops up on screen and closes almost 
    // immediately
    //  cout << "enter 1 to cont\n";
    //  cin >> pause;
    //  viewer.wasStopped ();
    
    // method 3: if you interact with the window with keys, the window is not closed properly
    // TODO: use pcl methods as this time stuff is probably not cross playform
    struct timespec t;
    t.tv_sec = 10;
    //t.tv_nsec = (time_t)(20000000); // short sleep
    t.tv_nsec = (time_t)(0);  // long sleep - normal speed
    nanosleep(&t, NULL);
    write_file_ =0;
  }
}

// Handle normal keys
void on_keyboard(unsigned char key, int x, int y)
{
  double speed = 0.1;

  if (key == 27)
    exit(0);
  else if (key == 'w' || key == 'W')
    camera_->move(speed,0,0);
  else if (key == 's' || key == 'S')
    camera_->move(-speed,0,0);
  else if (key == 'a' || key == 'A')
    camera_->move(0,speed,0);
  else if (key == 'd' || key == 'D')
    camera_->move(0,-speed,0);
  else if (key == 'q' || key == 'Q')
    camera_->move(0,0,speed);
  else if (key == 'z' || key == 'Z')
    camera_->move(0,0,-speed);
  else if (key == 'p' || key == 'P')
    paused_ = !paused_;
  else if (key == 'f' || key == 'F')
    write_file_ = 1;
  
  // Use glutGetModifiers for modifiers
  // GLUT_ACTIVE_SHIFT, GLUT_ACTIVE_CTRL, GLUT_ACTIVE_ALT
}

// Handle special keys, e.g. F1, F2, ...
void on_special(int key, int x, int y)
{
  switch (key) {
  case GLUT_KEY_F1:
    break;
  case GLUT_KEY_HOME:
    break;
  }
}

void on_reshape(int w, int h)
{
  // Window size changed
  window_width_ = w;
  window_height_ = h;
}

void on_mouse(int button, int state, int x, int y)
{
  // button:
  // GLUT_LEFT_BUTTON
  // GLUT_MIDDLE_BUTTON
  // GLUT_RIGHT_BUTTON
  //
  // state:
  // GLUT_UP
  // GLUT_DOWN
}

void on_motion(int x, int y)
{
}

void on_passive_motion(int x, int y)
{
  if (paused_) return;

  double pitch = -(0.5-(double)y/window_height_)*M_PI; // in window coordinates positive y-axis is down
  double yaw =    (0.5-(double)x/window_width_)*M_PI*2;

  camera_->set_pitch(pitch);
  camera_->set_yaw(yaw);
}

void on_entry(int state)
{
  // state:
  // GLUT_LEFT
  // GLUT_ENTERED
}

// Read in a 3D map, assuming a set of PCD files, each point is a polygon vertex
void load_PCDstack_model(const std::vector<std::string> & files)
{
/*
  for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file)
  {
    std::cout << "Load model: " << *file << std::endl;

    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZRGBA>);
    if (pcl::io::loadPCDFile<pcl::PointXYZRGBA> (*file, *cloud) == -1) // load the file
    {
      PCL_ERROR ("Couldn't read file %s \n", file->c_str()) ;
      exit (-1);
    }

    PointCloudModel::Ptr model = PointCloudModel::Ptr(new PointCloudModel(GL_POLYGON, cloud));
    scene_->add(model);
  }
*/
}

// Read in a 3D map from any 
void load_PolygonMesh_model(string &polygon_file)
{
  pcl::PolygonMesh mesh;	// (new pcl::PolygonMesh);
  //pcl::io::loadPolygonFile("/home/mfallon/data/models/dalet/Darlek_modified_works.obj",mesh);
  pcl::io::loadPolygonFile(polygon_file.c_str(), mesh);
  pcl::PolygonMesh::Ptr cloud (new pcl::PolygonMesh(mesh));
  
  // Not sure if PolygonMesh assumes triangles if to
  // TODO: Ask a developer
  PolygonMeshModel::Ptr model = PolygonMeshModel::Ptr(new PolygonMeshModel(GL_POLYGON,cloud));
  scene_->add(model);  
  
  std::cout << "Just read " << polygon_file << std::endl;
  std::cout << mesh.polygons.size() << " polygons and " 
	    << mesh.cloud.data.size() << " triangles\n";
  
}


void initialize(int argc, char** argv)
{
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << "OpenGL Version: " << version << std::endl;

  // works well for MIT CSAIL model 3rd floor:
//  camera_->set(4.04454, 44.9377, 1.1, 0.0, 0.0, -2.00352);
  // works for small files:
  camera_->set(-5.0, 0.0, 1.0, 0.0, 0.0, 0.0);

  int method;	
  std::string map_file = "" ;
  parse_argument (argc, argv, "-method", method);
  parse_argument (argc, argv, "-map", map_file);
  
  if (method==0){
    cout << method << " pcd file stack\n";
    
    cout << map_file << endl;

    cout << "TODO: parse the file path from something like:\n";
    cout << "    /data/rgbd/freenect/2011_06_10_trolley/submaps_new/submap_0*\n";
//    printHelp (argc, argv);
//    exit(-1);
    
    // this used to work:
    std::vector<std::string> files;
    int map_arg_index = find_argument(argc, argv, "-map");
    if (map_arg_index != -1) {
      for (int i=map_arg_index+1; i<argc; ++i) files.push_back(argv[i]);
    }
    load_PCDstack_model(files);

  } else if (method==1) {
    //map_file = "/home/mfallon/data/models/obj_samples/teapot.obj";
    cout << "About to read: " << map_file.c_str() << endl;
    load_PolygonMesh_model(map_file);
  }else{
    cout << "Method not correctly specfied\n";
    printHelp (argc, argv);
    exit(-1); 
  }
    
  
  //if (method ==0){ ///raw pcd 

  
  // TODO: choose between modes at arguments:
  //load_rwx_models(files); // disabled
  //load_PCDstack_model(files);
  
  
  //string polygon_file = "/home/mfallon/data/pcd_data/example_data/example_output/mesh.vtk";
  
  //wait();

  int i;
  for (i=0; i<2048; i++) {
    float v = i/2048.0;
    v = powf(v, 3)* 6;
    t_gamma[i] = v*6*256;
  }

  paused_ = false;
}



int main(int argc, char** argv)
{
  print_info ("Manually generate a simulated RGB-D point cloud using pcl::simulation. For more information, use: %s -h\n", argv[0]);

  if (argc < 5)
  {
    printHelp (argc, argv);
    return (-1);
  }  

  camera_ = Camera::Ptr(new Camera());
  scene_ = Scene::Ptr(new Scene());

  range_likelihood_ = RangeLikelihood::Ptr(new RangeLikelihood(1, 1, 480, 640, scene_, 640));
//  range_likelihood_ = RangeLikelihood::Ptr(new RangeLikelihood(10, 10, 96, 96, scene_));
  //range_likelihood_ = RangeLikelihood::Ptr(new RangeLikelihood(1, 1, 480, 640, scene_));

  // Actually corresponds to default parameters:
  range_likelihood_->set_CameraIntrinsicsParameters(640,480,576.09757860,
	    576.09757860, 321.06398107,  242.97676897);

  window_width_ = range_likelihood_->width() * 2;
  window_height_ = range_likelihood_->height();

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);// was GLUT_RGBA
  glutInitWindowPosition(10,10);
  glutInitWindowSize(window_width_, window_height_);
  glutCreateWindow("OpenGL range likelihood");

  initialize(argc, argv);

  glutReshapeFunc(on_reshape);
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc(on_keyboard);
  glutMouseFunc(on_mouse);
  glutMotionFunc(on_motion);
  glutPassiveMotionFunc(on_passive_motion);
  glutEntryFunc(on_entry);
  glutMainLoop(); 
}
