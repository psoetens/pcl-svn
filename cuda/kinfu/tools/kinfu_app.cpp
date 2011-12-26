/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Author: Anatoly Baskeheev, Itseez Ltd, (myname.mysurname@mycompany.com)
 */

#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>

#include <pcl/console/parse.h>

#include "pcl/gpu/kinfu/kinfu.h"
#include "pcl/gpu/containers/initialization.hpp"

#include <pcl/common/time.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/image_viewer.h>
#include "pcl/visualization/pcl_visualizer.h"
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>

#include "openni_capture.h"

#include "tsdf_volume.h"
#include "tsdf_volume.hpp"

#ifdef HAVE_OPENCV
  #include "opencv2/opencv.hpp"
  #include "pcl/gpu/utils/timers_opencv.hpp"
//#include "video_recorder.h"
typedef pcl::gpu::ScopeTimerCV ScopeTimeT;
#else
typedef pcl::ScopeTime ScopeTimeT;
#endif

#include "../src/internal.h"

using namespace std;
using namespace pcl;
using namespace pcl::gpu;
using namespace Eigen;
namespace pc = pcl::console;

namespace pcl
{
  namespace gpu
  {
    void paint3DView (const KinfuTracker::View& rgb24, KinfuTracker::View& view, float colors_weight = 0.5f);
    void mergePointNormal (const DeviceArray<PointXYZ>& cloud, const DeviceArray<Normal>& normals, DeviceArray<PointNormal>& output);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
setViewerPose (visualization::PCLVisualizer& viewer, const Eigen::Affine3f& viewer_pose)
{
  Eigen::Vector3f pos_vector = viewer_pose * Eigen::Vector3f (0, 0, 0);
  Eigen::Vector3f look_at_vector = viewer_pose.rotation () * Eigen::Vector3f (0, 0, 1) + pos_vector;
  Eigen::Vector3f up_vector = viewer_pose.rotation () * Eigen::Vector3f (0, -1, 0);
  viewer.camera_.pos[0] = pos_vector[0];
  viewer.camera_.pos[1] = pos_vector[1];
  viewer.camera_.pos[2] = pos_vector[2];
  viewer.camera_.focal[0] = look_at_vector[0];
  viewer.camera_.focal[1] = look_at_vector[1];
  viewer.camera_.focal[2] = look_at_vector[2];
  viewer.camera_.view[0] = up_vector[0];
  viewer.camera_.view[1] = up_vector[1];
  viewer.camera_.view[2] = up_vector[2];
  viewer.updateCamera ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CurrentFrameCloudView
{
  CurrentFrameCloudView() : cloud_device_ (480, 640), cloud_viewer_ ("Frame Cloud Viewer")
  {
    cloud_ptr_ = PointCloud<PointXYZ>::Ptr (new PointCloud<PointXYZ>);

    cloud_viewer_.setBackgroundColor (0, 0, 0.15);
    cloud_viewer_.setPointCloudRenderingProperties (visualization::PCL_VISUALIZER_POINT_SIZE, 1);
    cloud_viewer_.addCoordinateSystem (1.0);
    cloud_viewer_.initCameraParameters ();
    cloud_viewer_.camera_.clip[0] = 0.01;
    cloud_viewer_.camera_.clip[1] = 10.01;
  }

  void
  show (const KinfuTracker& kinfu)
  {
    kinfu.getLastFrameCloud (cloud_device_);

    int c;
    cloud_device_.download (cloud_ptr_->points, c);
    cloud_ptr_->width = cloud_device_.cols ();
    cloud_ptr_->height = cloud_device_.rows ();
    cloud_ptr_->is_dense = false;

    cloud_viewer_.removeAllPointClouds ();
    cloud_viewer_.addPointCloud<PointXYZ>(cloud_ptr_);
    cloud_viewer_.spinOnce ();
  }

  void
  setViewerPose (const Eigen::Affine3f& viewer_pose) {
    ::setViewerPose (cloud_viewer_, viewer_pose);
  }

  PointCloud<PointXYZ>::Ptr cloud_ptr_;
  DeviceArray2D<PointXYZ> cloud_device_;
  visualization::PCLVisualizer cloud_viewer_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ImageView
{
  ImageView() : paintImage_ (false), accumulateViews_ (false)
  {
    viewerScene_.setWindowTitle ("View3D from ray tracing");
    viewerDepth_.setWindowTitle ("Kinect Depth stream");
    //viewerColor_.setWindowTitle ("Kinect RGB stream");
  }

  void
  showScene (KinfuTracker& kinfu, const PtrStepSz<const KinfuTracker::RGB>& rgb24, bool registration = false)
  {
    kinfu.getImage (view_device_);
    if (paintImage_ && registration)
    {
      colors_device_.upload (rgb24.data, rgb24.step, rgb24.rows, rgb24.cols);
      paint3DView (colors_device_, view_device_);
    }

    int cols;
    view_device_.download (view_host_, cols);
    viewerScene_.showRGBImage ((unsigned char*)&view_host_[0], view_device_.cols (), view_device_.rows ());
    //viewerScene_.spinOnce();

    //viewerColor_.showRGBImage ((unsigned char*)&rgb24.data, rgb24.cols, rgb24.rows);
    //viewerColor_.spinOnce();

#ifdef HAVE_OPENCV
    if (accumulateViews_)
    {
      views_.push_back (cv::Mat ());
      cv::cvtColor (cv::Mat (480, 640, CV_8UC3, (void*)&view_host_[0]), views_.back (), CV_RGB2GRAY);
      //cv::copy(cv::Mat(480, 640, CV_8UC3, (void*)&view_host_[0]), views_.back());
    }
#endif

  }

  void
  showDepth (const PtrStepSz<const unsigned short>& depth)
  {
    viewerDepth_.showShortImage (depth.data, depth.cols, depth.rows, 0, 5000, true);
    //viewerDepth_.spinOnce();
  }

  void
  toggleImagePaint ()
  {
    paintImage_ = !paintImage_;
    cout << "Paint image: " << (paintImage_ ? "On   (requires registration mode)" : "Off") << endl;
  }

  bool paintImage_;
  bool accumulateViews_;

  visualization::ImageViewer viewerScene_;
  visualization::ImageViewer viewerDepth_;
  //visualization::ImageViewer viewerColor_;

  KinfuTracker::View view_device_;
  KinfuTracker::View colors_device_;
  vector<KinfuTracker::RGB> view_host_;

#ifdef HAVE_OPENCV
  vector<cv::Mat> views_;
#endif
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SceneCloudView
{
  enum { GPU_Connected6 = 0, CPU_Connected6 = 1, CPU_Connected26 = 2 };

  SceneCloudView() : extraction_mode_ (GPU_Connected6), computeNormals_ (false), validCombined_ (false), cloud_viewer_ ("Scene Cloud Viewer")
  {
    cloud_ptr_ = PointCloud<PointXYZ>::Ptr (new PointCloud<PointXYZ>);
    normals_ptr_ = PointCloud<Normal>::Ptr (new PointCloud<Normal>);
    combined_ptr_ = PointCloud<PointNormal>::Ptr (new PointCloud<PointNormal>);

    cloud_viewer_.setBackgroundColor (0, 0, 0);
    cloud_viewer_.addCoordinateSystem (1.0);
    cloud_viewer_.initCameraParameters ();
    cloud_viewer_.camera_.clip[0] = 0.01;
    cloud_viewer_.camera_.clip[1] = 10.01;

    cloud_viewer_.addText ("H: print help", 2, 15, 20, 34, 135, 246);
  }

  void
  show (KinfuTracker& kinfu)
  {
    ScopeTimeT time ("PointCloud Extraction");
    cout << "\nGetting cloud... ";

    validCombined_ = false;

    if (extraction_mode_ != GPU_Connected6)     // So use CPU
    {
      kinfu.getCloudFromVolumeHost (*cloud_ptr_, extraction_mode_ == CPU_Connected26);
    }
    else
    {
      DeviceArray<PointXYZ> extracted = kinfu.getCloudFromVolume (cloud_buffer_device_);
      if (computeNormals_)
        kinfu.getNormalsFromVolume (extracted, normals_device_);

      if (computeNormals_)
      {
        pcl::gpu::mergePointNormal (extracted, normals_device_, combined_device_);
        combined_device_.download (combined_ptr_->points);
        combined_ptr_->width = (int)combined_ptr_->points.size ();
        combined_ptr_->height = 1;

        validCombined_ = true;
      }
      else
      {
        extracted.download (cloud_ptr_->points);
        cloud_ptr_->width = (int)cloud_ptr_->points.size ();
        cloud_ptr_->height = 1;
      }
    }
    size_t points_size = validCombined_ ? combined_ptr_->points.size () : cloud_ptr_->points.size ();
    cout << "Done.  Cloud size: " << points_size / 1000 << "K" << endl;

    cloud_viewer_.removeAllPointClouds ();
    if (validCombined_)
    {
      cloud_viewer_.addPointCloud<PointNormal> (combined_ptr_, "Cloud");
      cloud_viewer_.addPointCloudNormals<pcl::PointNormal>(combined_ptr_, 50);
    }
    else
      cloud_viewer_.addPointCloud<PointXYZ> (cloud_ptr_);

    //cloud_viewer_.setPointCloudRenderingProperties (visualization::PCL_VISUALIZER_POINT_SIZE, 1);
  }

  void
  toggleExctractionMode ()
  {
    extraction_mode_ = (extraction_mode_ + 1) % 3;

    switch (extraction_mode_)
    {
    case 0: cout << "Cloud exctraction mode: GPU, Connected-6" << endl; break;
    case 1: cout << "Cloud exctraction mode: CPU, Connected-6    (requires a lot of memory)" << endl; break;
    case 2: cout << "Cloud exctraction mode: CPU, Connected-26   (requires a lot of memory)" << endl; break;
    }
    ;
  }

  void
  toggleNormals ()
  {
    computeNormals_ = !computeNormals_;
    cout << "Computer normals: " << (computeNormals_ ? "On" : "Off") << endl;
  }

  void
  clearClouds ()
  {
    cloud_viewer_.removeAllPointClouds ();
    cloud_ptr_->points.clear ();
    normals_ptr_->points.clear ();
    cout << "Clouds were cleared" << endl;
  }

  void
  setViewerPose (const Eigen::Affine3f& viewer_pose) {
    ::setViewerPose (cloud_viewer_, viewer_pose);
  }

  int extraction_mode_;
  bool computeNormals_;
  bool validCombined_;

  visualization::PCLVisualizer cloud_viewer_;

  PointCloud<PointXYZ>::Ptr cloud_ptr_;
  PointCloud<Normal>::Ptr normals_ptr_;

  DeviceArray<PointXYZ> cloud_buffer_device_;
  DeviceArray<Normal> normals_device_;

  PointCloud<PointNormal>::Ptr combined_ptr_;
  DeviceArray<PointNormal> combined_device_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct KinFuApp
{
  enum { PCD_BIN = 1, PCD_ASCII = 2, PLY = 3 };

  KinFuApp(CaptureOpenNI& source) : exit_ (false), scan_ (false), scan_volume_ (false), hasImage_ (false),
    registration_ (false), integrateColors_ (false), frame_time_ms (0), capture_ (source)
  {    
    //Init Kinfu Tracker
    Eigen::Vector3f volume_size = Vector3f::Constant (3.f);

    float f = capture_.depth_focal_length_VGA;
    kinfu_.setDepthIntrinsics (f, f);
    kinfu_.setVolumeSize (volume_size);

    Eigen::Matrix3f R = Eigen::Matrix3f::Identity ();   // * AngleAxisf(-30.f/180*3.1415926, Vector3f::UnitX());
    Eigen::Vector3f t = volume_size * 0.5f - Vector3f (0, 0, volume_size (2) / 2 * 1.2f);

    Eigen::Affine3f pose = Eigen::Translation3f (t) * Eigen::AngleAxisf (R);

    kinfu_.setInitalCameraPose (pose);
    kinfu_.setTrancationDistance (0.030f);    // in meters;
    kinfu_.setIcpCorespFilteringParams (0.1f /*meters*/, sin (20.f * 3.14159254f / 180.f));
    //kinfu_.setDepthTruncationForICP(5.f /*meters*/);

    //Init KinfuApp        
    tsdf_cloud_ptr_ = pcl::PointCloud<pcl::PointXYZI>::Ptr (new pcl::PointCloud<pcl::PointXYZI>);

    scene_cloud_view_.cloud_viewer_.registerKeyboardCallback (keyboard_callback, (void*)this);
    image_view_.viewerScene_.registerKeyboardCallback (keyboard_callback, (void*)this);
    image_view_.viewerDepth_.registerKeyboardCallback (keyboard_callback, (void*)this);

    float diag = sqrt ((float)kinfu_.cols () * kinfu_.cols () + kinfu_.rows () * kinfu_.rows ());
    scene_cloud_view_.cloud_viewer_.camera_.fovy = 2 * atan (diag / (2 * f));
  }

  void
  initCurrentFrameView ()
  {
    current_frame_cloud_view_ = boost::shared_ptr<CurrentFrameCloudView>(new CurrentFrameCloudView ());
    current_frame_cloud_view_->cloud_viewer_.registerKeyboardCallback (keyboard_callback, (void*)this);
    current_frame_cloud_view_->setViewerPose (kinfu_.getCameraPose ());
  }

  void
  tryRegistrationInit ()
  {
    registration_ = capture_.setRegistration (true);
    cout << "Registration mode: " << (registration_ ?  "On" : "Off (not supported by source)") << endl;
  }

  void
  toggleColorsIntegration () {
    integrateColors_ = true;
  }

  void
  execute ()
  {
    PtrStepSz<const unsigned short> depth;
    PtrStepSz<const KinfuTracker::RGB> rgb24;

    for (int i = 0; !exit_; ++i)
    {
      //cout << i << endl;
      //if (i == 3) break;

      if (!capture_.grab (depth, rgb24))
      {
        cout << "Can't grab" << endl;
        break;
      }

      depth_device_.upload (depth.data, depth.step, depth.rows, depth.cols);

      //start timer
      stopWatch_.reset ();

      //run kinfu algorithm
      hasImage_ = kinfu_ (depth_device_);

      //stop timer
      frame_time_ms += stopWatch_.getTime ();
      const int each = 33;
      if (i % each == 0)
        cout << "Average frame time = " << frame_time_ms / each << "ms ( " << 1000.f * each / frame_time_ms << "fps )" << endl, frame_time_ms = 0;

      if (scan_)
      {
        scan_ = false;
        scene_cloud_view_.show (kinfu_);

        if (scan_volume_)
        {
          // download tsdf volume
          {
            ScopeTimeT time ("tsdf volume download");
            cout << "Downloading TSDF volume from device ... " << flush;
            kinfu_.getTsdfVolumeAndWeighs (tsdf_volume_.volumeWriteable (), tsdf_volume_.weightsWriteable ());
            tsdf_volume_.setHeader (Eigen::Vector3i (pcl::device::VOLUME_X, pcl::device::VOLUME_Y, pcl::device::VOLUME_Z), kinfu_.getVolumeSize ());
            cout << "done [" << tsdf_volume_.size () << " voxels]" << endl << endl;
          }
          {
            ScopeTimeT time ("converting");
            cout << "Converting volume to TSDF cloud ... " << flush;
            tsdf_volume_.convertToTsdfCloud (tsdf_cloud_ptr_);
            cout << "done [" << tsdf_cloud_ptr_->size () << " points]" << endl << endl;
          }
        }
        else
          cout << "[!] tsdf volume download is disabled" << endl << endl;
      }

      if (hasImage_)
        image_view_.showScene (kinfu_, rgb24, registration_);

      if (current_frame_cloud_view_)
        current_frame_cloud_view_->show (kinfu_);

      scene_cloud_view_.setViewerPose (kinfu_.getCameraPose ());
      scene_cloud_view_.cloud_viewer_.spinOnce ();

      image_view_.showDepth (depth);
    }
  }

  void
  writeCloud (int format) const
  {
    if (scene_cloud_view_.validCombined_)
      writeCloud (format, *scene_cloud_view_.combined_ptr_);
    else
      writeCloud (format, *scene_cloud_view_.cloud_ptr_);
  }

  template<typename CloudT>
  void
  writeCloud (int format, const CloudT& cloud) const
  {
    if (format == PCD_BIN)
    {
      cout << "Saving point cloud to 'cloud_bin.pcd' (binary)... ";
      pcl::io::savePCDFile ("cloud_bin.pcd", cloud, true);
    }
    else
    if (format == PCD_ASCII)
    {
      cout << "Saving point cloud to 'cloud.pcd' (ASCII)... ";
      pcl::io::savePCDFile ("cloud.pcd", cloud, false);
    }
    else   /* if (format == PLY) */
    {
      cout << "Saving point cloud to 'cloud.ply' (ASCII)... ";
      pcl::io::savePLYFileASCII ("cloud.ply", cloud);

    }
    cout << "Done" << endl;
  }

  void
  printHelp ()
  {
    cout << endl;
    cout << "KinFu app hotkeys" << endl;
    cout << "=================" << endl;
    cout << "    H    : print this help" << endl;
    cout << "   Esc   : exit" << endl;
    cout << "    T    : take cloud" << endl;
    cout << "    M    : toggle cloud exctraction mode" << endl;
    cout << "    N    : toggle normals exctraction" << endl;
    //cout << "    *    : toggle scene view painting ( requires registration mode )" << endl;
    cout << "    C    : clear clouds" << endl;
    cout << "   1,2,3 : save cloud to PCD(binary), PCD(ASCII), PLY(ASCII)" << endl;
    cout << "   X, V  : TSDF volume utility" << endl;
    cout << endl;
  }

  bool exit_;
  bool scan_;
  bool scan_volume_;

  bool hasImage_;
  bool registration_;
  bool integrateColors_;

  int frame_time_ms;

  CaptureOpenNI& capture_;
  KinfuTracker kinfu_;

  pcl::StopWatch stopWatch_;

  SceneCloudView scene_cloud_view_;
  ImageView image_view_;
  boost::shared_ptr<CurrentFrameCloudView> current_frame_cloud_view_;

  KinfuTracker::DepthMap depth_device_;

  pcl::TSDFVolume<float, short> tsdf_volume_;
  pcl::PointCloud<pcl::PointXYZI>::Ptr tsdf_cloud_ptr_;

  static void
  keyboard_callback (const visualization::KeyboardEvent &e, void *cookie)
  {
    KinFuApp* app = reinterpret_cast<KinFuApp*> (cookie);

    int key = e.getKeyCode ();

    if (e.keyDown ())
      switch (key)
      {
      case 27: app->exit_ = true; break;
      case (int)'t': case (int)'T': app->scan_ = true; break;
      case (int)'h': case (int)'H': app->printHelp (); break;
      case (int)'m': case (int)'M': app->scene_cloud_view_.toggleExctractionMode (); break;
      case (int)'n': case (int)'N': app->scene_cloud_view_.toggleNormals (); break;
      case (int)'c': case (int)'C': app->scene_cloud_view_.clearClouds (); break;
      case (int)'1': case (int)'2': case (int)'3': app->writeCloud (key - (int)'0'); break;
      case '*': app->image_view_.toggleImagePaint (); break;

      case (int)'x': case (int)'X':
        app->scan_volume_ = !app->scan_volume_;
        cout << endl << "Volume scan: " << (app->scan_volume_ ? "enabled" : "disabled") << endl << endl;
        break;
      case (int)'v': case (int)'V':
        cout << "Saving TSDF volume to tsdf_volume.dat ... " << flush;
        app->tsdf_volume_.save ("tsdf_volume.dat", true);
        cout << "done [" << app->tsdf_volume_.size () << " voxels]" << endl;
        cout << "Saving TSDF volume cloud to tsdf_cloud.pcd ... " << flush;
        pcl::io::savePCDFile<pcl::PointXYZI> ("tsdf_cloud.pcd", *app->tsdf_cloud_ptr_, true);
        cout << "done [" << app->tsdf_cloud_ptr_->size () << " points]" << endl;
        break;

      default:
        break;
      }
    ;
  }
};



int
printCliHelp ()
{
  cout << "\nKinfu app concole parameters help:" << endl;
  cout << "--help, -h                      : print this message" << endl;
  cout << "-dev <deivce>, -oni <oni_file>  : selects depth souce. Default will be selected if not specified" << endl;
  cout << "--registration, -r              : try to enable registration ( requires source to support this )" << endl;
  cout << "--current-cloud, -cc            : show current frame cloud" << endl;
  cout << "--save-views, -sv               : accumulate scene view and save in the end ( Requires OpenCV. Will cause 'bad_alloc' after some time )" << endl;

  return 0;
}

int
main (int argc, char* argv[])
{
  if (pc::find_switch (argc, argv, "--help") || pc::find_switch (argc, argv, "-h"))
    return printCliHelp ();

  int device = 0;
  pc::parse_argument (argc, argv, "-gpu", device);
  pcl::gpu::setDevice (device);
  pcl::gpu::printShortCudaDeviceInfo (device);

  CaptureOpenNI capture;

  int openni_device = 0;
  std::string oni_file;
  if (pc::parse_argument (argc, argv, "-dev", openni_device) > 0)
  {
    capture.init (openni_device);
  }
  else
  if (pc::parse_argument (argc, argv, "-oni", oni_file) > 0)
  {
    capture.init (oni_file);
  }
  else
  {
    capture.init (openni_device);
    //capture.init("d:/onis/20111013-224932.oni");
    //capture.init("d:/onis/white1.oni");
    //capture.init("/media/Main/onis/20111013-224932.oni");
    //capture.init("20111013-225218.oni");
    //capture.init("d:/onis/20111013-224551.oni");
    //capture.init("d:/onis/20111013-224719.oni");
  }

  KinFuApp app (capture);

  if (pc::find_switch (argc, argv, "--registration") || pc::find_switch (argc, argv, "-r"))
    app.tryRegistrationInit ();

  if (pc::find_switch (argc, argv, "--current-cloud") || pc::find_switch (argc, argv, "-cc"))
    app.initCurrentFrameView ();

  if (pc::find_switch (argc, argv, "--save-views") || pc::find_switch (argc, argv, "-sv"))
    app.image_view_.accumulateViews_ = true;   //will cause bad alloc after some time

  if (pc::find_switch (argc, argv, "--integrate-colors") || pc::find_switch (argc, argv, "-ic"))
    app.toggleColorsIntegration ();   // requires a lot of video memory (~1.5Gb)

  // executing
  try { app.execute (); }
  catch (const std::bad_alloc& /*e*/) { cout << "Bad alloc" << endl; }
  catch (const std::exception& /*e*/) { cout << "Exception" << endl; }

#ifdef HAVE_OPENCV
  for (size_t t = 0; t < app.image_view_.views_.size (); ++t)
  {
    if (t == 0)
    {
      cout << "Saving depth map of first view." << endl;
      cv::imwrite ("./depthmap_1stview.png", app.image_view_.views_[0]);
      cout << "Saving sequence of (" << app.image_view_.views_.size () << ") views." << endl;
    }
    char buf[4096];
    sprintf (buf, "./%06d.png", (int)t);
    cv::imwrite (buf, app.image_view_.views_[t]);
    printf ("writing: %s\n", buf);
  }
#endif
  return 0;
}
