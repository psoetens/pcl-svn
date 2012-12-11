/*
 * Software License Agreement  (BSD License)
 *
 *  Point Cloud Library  (PCL) - www.pointclouds.org
 *  Copyright (c) 2012-, Open Perception, Inc.
 *
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
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES  (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef PCL_APPS_OPTRONIC_VIEWER_MAIN_WINDOW_H_
#define PCL_APPS_OPTRONIC_VIEWER_MAIN_WINDOW_H_

#include <pcl/apps/optronic_viewer/qt.h>
#include <boost/shared_ptr.hpp>
#include <pcl/io/openni_grabber.h>
#include <pcl/apps/optronic_viewer/openni_grabber.h>

#include <pcl/visualization/pcl_visualizer.h>


namespace pcl
{
  namespace apps
  {
    namespace optronic_viewer
    {
      
      struct OpenNIDevice
      {
        int device_id;

        std::string name;
        std::string vendor;
        std::string serial_number;
      };

      class MainWindow : public QMainWindow
      {
        Q_OBJECT

        public:
          static MainWindow& getInstance() {
            static MainWindow theSingleton;
            return theSingleton;
          }

        public slots:
          void selectedSensorChanged (int index);
          void cloud_callback (const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr cloud);
          void refresh ();

        private:
          // create the file menu in the menu bar
          void createFileMenu ();

          // find connected devices
          void findConnectedDevices ();

        private slots:

        private:
          MainWindow();
          MainWindow(const MainWindow &) : QMainWindow () {}            // copy ctor hidden
          MainWindow& operator=(const MainWindow &) { return (*this); } // assign op. hidden
          ~MainWindow();

          // visualization of processing chain
          QListView * processing_list_view_;

          // visualization of point clouds
          QVTKWidget * qvtk_widget_;
          pcl::visualization::PCLVisualizer * pcl_visualizer_;
          pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr cloud_;

          // input stuff
          std::vector<OpenNIDevice> connected_openni_devices_;
          pcl::Grabber * grabber_;
          pcl::apps::optronic_viewer::OpenNIGrabber * grabber_thread_;
          boost::mutex cloud_mutex_;
      };
    }
  }
}

#endif // PCL_APPS_OPTRONIC_VIEWER_MAIN_WINDOW_H_
