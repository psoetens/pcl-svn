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
//#include <opencv2/opencv.hpp>
#include "pcl/gpu/kinfu/openni_capture.hpp"
#include "pcl/gpu/kinfu/kinfu.hpp"
//#include "pcl/gpu/kinfu/video_recorder.hpp"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/image_viewer.h>

#include "opencv2/opencv.hpp"

using namespace std;
using namespace pcl;
using namespace pcl::gpu;

int main()
{
    //return just_capture(), 0;    
    try
    {
        //BufferedRecorder recorder;
        CaptureOpenNI cap(0);
        //CaptureOpenNI cap("d:/onis/20111013-224932.oni");
        //CaptureOpenNI cap("d:/onis/20111013-224551.oni");
        //CaptureOpenNI cap("d:/onis/20111013-224719.oni");

        KinfuTracker kinfu(480, 640);

        kinfu.fx = kinfu.fy = cap.depth_focal_length_VGA;

        pcl::visualization::ImageViewer viewer;
        pcl::visualization::ImageViewer viewer3d;        

        KinfuTracker::DepthMap depth_device;
        KinfuTracker::View view_device;       
        vector<uchar3> view3d_host;
        for(int i = 0;;++i)
        {                        
            cout << i << endl;
            
            PtrStepSz<const unsigned short> depth;
            PtrStepSz<const uchar3> rgb24;
            if (!cap.grab(depth, rgb24))
            {
                cout << "Can't grab" << endl;
                break;
            }

            cv::Mat d(480, 640, CV_16S, (void*)depth.data), d32, d16;
            d.convertTo(d32, CV_32F);
            
            cv::bilateralFilter(d32, d16, 5, 10, 10);
            d16.convertTo(d16, CV_16U);

            depth.data = d16.ptr<unsigned short>();

            viewer.showRGBImage ((unsigned char*)rgb24.data, rgb24.cols, rgb24.rows);
            viewer.spinOnce(3);

            /*double maxval;
            cv::minMaxLoc(depth, 0, &maxval);
            depth.convertTo(du8, CV_8U, 255.0/maxval);
            imshow("depth", du8);*/

            depth_device.upload(depth.data, depth.step, depth.rows, depth.cols);

            kinfu(depth_device, view_device);
            if (!view_device.empty())
            {
                int cols;
                view_device.download(view3d_host, cols);
                viewer3d.showRGBImage((unsigned char*)&view3d_host[0], view_device.cols(), view_device.rows());
            }
            
            viewer3d.spinOnce(3);
           
            //recorder.push_back(bgr, du8, res);            
        }

        //recorder.save("video.avi");
    }
    catch(const std::exception& /*e*/) { cout << "exception" << endl; }	

    return 0;
}