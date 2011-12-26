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

using namespace std;
using namespace pcl;
using namespace pcl::gpu;
using namespace Eigen;

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

template<typename PointT>
void
donwloadOrganized (const DeviceArray2D<PointT>& device, pcl::PointCloud<PointT>& host)
{
  int c;
  device.download (host.points, c);
  host.width = device.cols ();
  host.height = device.rows ();
  host.is_dense = false;
}

#include "../src/internal.h"

struct KinFuApp
{
  KinFuApp(CaptureOpenNI& source, bool show_current_frame = false) : exit_ (false), scan_ (false), scan_volume_(false), showNormals_ (false), connected26_ (false), use_cpu_for_cloud_extraction_ (false), hasImage_ (false),
    registration_(false), frame_time_ms(0), capture_ (source), cloud_viewer_ ("Volume Cloud Viewer")
  {
    /////////////////////////////////////////
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

    /////////////////////////////////////////
    //Init KinfuApp

    bool registration_ = capture_.setRegistration(true);
    
    viewer3d_.setWindowTitle ("View3D from ray tracing");
    viewer2d_.setWindowTitle ("Kinect Depth stream");

    cloud_ptr_ = PointCloud<PointXYZ>::Ptr (new PointCloud<pcl::PointXYZ>);
    cloud_normals_ptr_ = PointCloud<Normal>::Ptr (new PointCloud<pcl::Normal>);
    cloud_combined_ptr_ = PointCloud<PointNormal>::Ptr (new PointCloud<pcl::PointNormal>);
    cloud_ptr_->points.push_back (pcl::PointXYZ (0, 0, 0));
    cloud_ptr_->width = cloud_ptr_->height = 1;

    tsdf_cloud_ptr_ = pcl::PointCloud<pcl::PointXYZI>::Ptr (new pcl::PointCloud<pcl::PointXYZI>);

    cloud_viewer_.setBackgroundColor (0, 0, 0);
    cloud_viewer_.addPointCloud<pcl::PointXYZ> (cloud_ptr_);
    cloud_viewer_.setPointCloudRenderingProperties (visualization::PCL_VISUALIZER_POINT_SIZE, 1);
    cloud_viewer_.addCoordinateSystem (1.0);
    cloud_viewer_.initCameraParameters ();
    cloud_viewer_.camera_.clip[0] = 0.01;
    cloud_viewer_.camera_.clip[1] = 10.01;
    cloud_viewer_.addText ("HotKeys: T, M, S, B, P, C, N", 2, 15, 20, 34, 135, 246);

    float diag = sqrt ((float)kinfu_.cols () * kinfu_.cols () + kinfu_.rows () * kinfu_.rows ());
    cloud_viewer_.camera_.fovy = 2 * atan (diag / (2 * f));

    viewer3d_.registerKeyboardCallback (keyboard_callback, (void*)this);
    cloud_viewer_.registerKeyboardCallback (keyboard_callback, (void*)this);
    viewer2d_.registerKeyboardCallback(keyboard_callback, (void*)this);

    if (show_current_frame)
    {
      frame_cloud_ptr_ = PointCloud<PointXYZ>::Ptr (new PointCloud<pcl::PointXYZ>);
      frame_cloud_ptr_->points.push_back (pcl::PointXYZ (0, 0, 0));
      frame_cloud_ptr_->width = frame_cloud_ptr_->height = 1;

      frame_normals_ptr_ = PointCloud<pcl::Normal>::Ptr (new PointCloud<pcl::Normal>);
      //frame_normals_ptr_->points.push_back(pcl::Normal(1, 1, 1));
      frame_normals_ptr_->width = frame_normals_ptr_->height = 1;

      frame_cloud_viewer_ = boost::shared_ptr<visualization::PCLVisualizer>(new visualization::PCLVisualizer ("Frame Cloud Viewer"));
      frame_cloud_viewer_->setBackgroundColor (0, 0, 0.3);
      frame_cloud_viewer_->addPointCloud<pcl::PointXYZ> (frame_cloud_ptr_);
      frame_cloud_viewer_->setPointCloudRenderingProperties (visualization::PCL_VISUALIZER_POINT_SIZE, 1);
      frame_cloud_viewer_->addCoordinateSystem (1.0);
      frame_cloud_viewer_->initCameraParameters ();
      frame_cloud_viewer_->camera_.clip[0] = 0.01;
      frame_cloud_viewer_->camera_.clip[1] = 10.01;
      frame_cloud_viewer_->registerKeyboardCallback (keyboard_callback, (void*)this);
      setViewerPose (*frame_cloud_viewer_, kinfu_.getCameraPose ());
    }
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
      stopWatch_.reset();

      //run kinfu algorithm
      hasImage_ = kinfu_ (depth_device_);
      if (hasImage_)
        kinfu_.getImage (view_device_);
      
      //stop timer
      frame_time_ms += stopWatch_.getTime();                
      const int each = 33;
      if (i % each == 0)
          cout << "Average frame time = " << frame_time_ms/each  << "ms ( " << 1000.f * each/frame_time_ms << "fps )" << endl, frame_time_ms = 0;
      
      if (scan_)
      {
        scan_ = false;

        // download point cloud
        {
          ScopeTimeT time ("point cloud extraction");
          cout << "\nGetting cloud..." << endl;

          if (use_cpu_for_cloud_extraction_)
            kinfu_.getCloudFromVolumeHost (*cloud_ptr_, connected26_);
          else
          {
            DeviceArray<KinfuTracker::PointType> extracted = kinfu_.getCloudFromVolume (cloud_buffer_device_);
            extracted.download (cloud_ptr_->points);
            cloud_ptr_->width = (int)cloud_ptr_->points.size ();
            cloud_ptr_->height = 1;

            if (showNormals_)
            {
              kinfu_.getNormalsFromVolume (extracted, cloud_normals_device_);
              cloud_normals_device_.download (cloud_normals_ptr_->points);
              cloud_normals_ptr_->width = (int)cloud_normals_ptr_->points.size ();
              cloud_normals_ptr_->height = 1;
            }
          }
        }
        cout << "  Cloud size: " << cloud_ptr_->points.size () / 1000 << "K" << endl << endl;
        cloud_viewer_.removeAllPointClouds ();

        if (scan_volume_)
        {
          // download tsdf volume
          {
            ScopeTimeT time ("tsdf volume download");
            cout << "Downloading TSDF volume from device ... " << flush;
            kinfu_.getTsdfVolumeAndWeighs (tsdf_volume_.volumeWriteable(), tsdf_volume_.weightsWriteable());
            tsdf_volume_.setHeader (Eigen::Vector3i (pcl::device::VOLUME_X, pcl::device::VOLUME_Y, pcl::device::VOLUME_Z), kinfu_.getVolumeSize());
            cout << "done [" << tsdf_volume_.size() << " voxels]" << endl << endl;
          }          
          {
            ScopeTimeT time ("converting");
            cout << "Converting volume to TSDF cloud ... " << flush;
            tsdf_volume_.convertToTsdfCloud (tsdf_cloud_ptr_);
            cout << "done [" << tsdf_cloud_ptr_->size() << " points]" << endl << endl;
          }          
        }
        else
          cout << "[!] tsdf volume download is disabled" << endl << endl;

        // handle and show normals
        if (showNormals_ && !use_cpu_for_cloud_extraction_)
        {
          cloud_viewer_.addPointCloud<pcl::PointXYZ> (cloud_ptr_, "Cloud");
          cloud_viewer_.addPointCloudNormals<pcl::PointXYZ, pcl::Normal>(cloud_ptr_, cloud_normals_ptr_, 50);

          //AB: It seems this doesn't work
          //std::cout << "Copying X,Y,Z points and normals in one cloud" << std::endl;
          //cout << "clear comb. cloud" << endl;
          //cloud_combined_ptr_->clear ();
          //cout << "copy cloud:" << endl;
          //pcl::copyPointCloud (*cloud_ptr_, *cloud_combined_ptr_);
          //cout << "copy normals:" << endl;
          //pcl::copyPointCloud (*cloud_normals_ptr_, *cloud_combined_ptr_);
          //cout << "show comb. in viewer" << endl;
          //cloud_viewer_.addPointCloud<pcl::PointNormal> (cloud_combined_ptr_, "Cloud");
        }
        else
          cloud_viewer_.addPointCloud<pcl::PointXYZ> (cloud_ptr_);
      }

      if (hasImage_)
      {
        int cols;
        view_device_.download (view_host_, cols);
        viewer3d_.showRGBImage ((unsigned char*)&view_host_[0], view_device_.cols (), view_device_.rows ());

#ifdef HAVE_OPENCV
        if (false)
        {
          views_.push_back(cv::Mat());
          cv::cvtColor(cv::Mat(480, 640, CV_8UC3, (void*)&view_host_[0]), views_.back(), CV_RGB2GRAY);
        }
#endif

        if (frame_cloud_viewer_)
        {
          kinfu_.getLastFrameCloud (frameCloud_);
          //kinfu_.getLastFrameNormals(frameNormals_);
          donwloadOrganized (frameCloud_, *frame_cloud_ptr_);
          //donwloadOrganized(frameNormals_, *frame_normals_ptr_);
          frame_cloud_viewer_->updatePointCloud (frame_cloud_ptr_);
          frame_cloud_viewer_->spinOnce (1, true);
        }
      }
      setViewerPose (cloud_viewer_, kinfu_.getCameraPose ());
      cloud_viewer_.spinOnce ();

	  viewer2d_.showShortImage (depth.data, depth.cols, depth.rows, 0, 5000, true);

#if ((VTK_MAJOR_VERSION >= 5) && (VTK_MINOR_VERSION > 6))
      if (hasImage_)
		  viewer3d_.spinOnce (3);        // don't know why crash under ubuntu is here.	  
      viewer2d_.spinOnce(3);
#endif      
    }
  }
  
  bool exit_;
  bool scan_;
  bool scan_volume_;
  bool showNormals_;
  bool connected26_;
  bool use_cpu_for_cloud_extraction_;

  bool hasImage_;

  bool registration_;

  int frame_time_ms;

  CaptureOpenNI& capture_;
  KinfuTracker kinfu_;

  pcl::StopWatch stopWatch_;

  visualization::ImageViewer viewer3d_;
  visualization::ImageViewer viewer2d_;
  visualization::PCLVisualizer cloud_viewer_;
  boost::shared_ptr<visualization::PCLVisualizer> frame_cloud_viewer_;

  PointCloud<PointXYZ>::Ptr cloud_ptr_;
  DeviceArray<KinfuTracker::PointType> cloud_buffer_device_;

  PointCloud<Normal>::Ptr cloud_normals_ptr_;
  DeviceArray<KinfuTracker::NormalType> cloud_normals_device_;

  PointCloud<PointNormal>::Ptr cloud_combined_ptr_;

  KinfuTracker::DepthMap depth_device_;
  KinfuTracker::View view_device_;
  vector<KinfuTracker::RGB> view_host_;

  DeviceArray2D<KinfuTracker::PointType> frameCloud_;
  DeviceArray2D<KinfuTracker::NormalType> frameNormals_;

  PointCloud<PointXYZ>::Ptr frame_cloud_ptr_;
  PointCloud<pcl::Normal>::Ptr frame_normals_ptr_;

  pcl::TSDFVolume<float, short> tsdf_volume_;
  pcl::PointCloud<pcl::PointXYZI>::Ptr tsdf_cloud_ptr_;

#ifdef HAVE_OPENCV
  vector<cv::Mat> views_;
#endif

  static void
  keyboard_callback (const visualization::KeyboardEvent &e, void *cookie)
  {
    KinFuApp* app = reinterpret_cast<KinFuApp*> (cookie);

    if (e.keyDown ())
      switch (e.getKeyCode ())
      {
        case 27:
          app->exit_ = true;
          break;
        case (int)'t': case (int)'T':
          app->scan_ = true;
          cout << "Scan set to true" << endl;
          break;
        case (int)'m': case (int)'M':
          app->connected26_ = !app->connected26_;
          cout << endl << "Cloud extraction mode: " << (app->connected26_ ? "connected26 (CPU only)" : "connected6") << endl << endl;
          break;
        case (int)'n': case (int)'N':
          app->showNormals_ = !app->showNormals_;
          cout << endl << "Show normals: " << (app->showNormals_ ? "true (GPU only)" : "false") << endl << endl;
          break;
        case (int)'x': case (int)'X':
          app->scan_volume_ = !app->scan_volume_;
          cout << endl << "Volume scan: " << (app->scan_volume_ ? "enabled" : "disabled") << endl << endl;
          break;
        case (int)'c': case (int)'C':
          app->cloud_viewer_.removeAllPointClouds ();
          cout << "Cloud viewer is reset" << endl;
          break;
        case (int)'v': case (int)'V':
          cout << "Saving TSDF volume to tsdf_volume.dat ... " << flush;
          app->tsdf_volume_.save("tsdf_volume.dat", true);
          cout << "done [" << app->tsdf_volume_.size() << " voxels]" << endl;
          cout << "Saving TSDF volume cloud to tsdf_cloud.pcd ... " << flush;
          pcl::io::savePCDFile<pcl::PointXYZI> ("tsdf_cloud.pcd", *app->tsdf_cloud_ptr_, true);
          cout << "done [" << app->tsdf_cloud_ptr_->size() << " points]" << endl;
          break;
        case (int)'s': case (int)'S':
          cout << "Saving point cloud to cloud.pcd (ASCII) ... " << flush;
          if (app->showNormals_)
            pcl::io::savePCDFile ("cloud.pcd", *app->cloud_combined_ptr_, false);
          else
            pcl::io::savePCDFile ("cloud.pcd", *app->cloud_ptr_, false);
          cout << "done" << endl;
          break;
        case (int)'b': case (int)'B':
          cout << "Saving point cloud to cloud.pcd (binary) ... ";
          if (app->showNormals_)
            pcl::io::savePCDFile ("cloud.pcd", *app->cloud_combined_ptr_, true);
          else
            pcl::io::savePCDFile ("cloud.pcd", *app->cloud_ptr_, true);
          cout << "done" << endl;
          break;
        case (int)'p': case (int)'P':
          cout << "Saving point cloud to cloud.ply ... ";
          pcl::io::savePLYFileASCII ("cloud.ply", *app->cloud_ptr_);
          cout << "done" << endl;
          break;
        default:
          break;
      };
  }
};

int
main2 ();

int
main ()
{
  //return main2();

  pcl::gpu::setDevice (0);
  pcl::gpu::printShortCudaDeviceInfo (0);

  CaptureOpenNI cap (0);  //First OpenNI device.
  //CaptureOpenNI cap("d:/onis/20111013-224932.oni");
  //CaptureOpenNI cap("d:/onis/white1.oni");
  //CaptureOpenNI cap("/media/Main/onis/20111013-224932.oni");
  //CaptureOpenNI cap("20111013-225218.oni");
  //CaptureOpenNI cap("d:/onis/20111013-224551.oni");
  //CaptureOpenNI cap("d:/onis/20111013-224719.oni");

  //KinFuApp app(cap, true); // show current frame cloud
  KinFuApp app (cap);
  //app.use_cpu_for_cloud_extraction_ = true;

  // executing
  try
  {
    app.execute ();
  }
  catch (const std::bad_alloc& /*e*/)
  {
    cout << "Bad alloc" << endl;
  }
  catch (const std::exception& /*e*/)
  {
    cout << "Exception" << endl;
  }

  // saving sequence of 3D views
  //pcl::io::savePCDFile("cloud.pcd", app.cloud_ptr_, true);
  //cout << "Saved to cloud.pcd" << endl;

#ifdef HAVE_OPENCV
  for (size_t t = 0; t < app.views_.size (); ++t)
  {
    if (t == 0)
    {
      cout << "Saving depth map of first view." << endl;
      cv::imwrite ("./depthmap_1stview.png", app.views_[0]);
      cout << "Saving sequence of (" << app.views_.size () << ") views." << endl;
    }
    char buf[4096];
    sprintf (buf, "./%06d.png", (int)t);
    cv::imwrite (buf, app.views_[t]);
    printf ("writing: %s\n", buf);
  }  
#endif
  return 0;
}
