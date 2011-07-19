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

#pragma warning (disable : 4996 4530)

#include <gtest/gtest.h>

#include<iostream>

#pragma warning (disable: 4521)
#include <pcl/point_cloud.h>
#include <pcl/octree/octree.h>
#pragma warning (default: 4521)

#include "pcl/gpu/octree/octree.hpp"
#include "pcl/gpu/common/device_array.hpp"
#include "pcl/gpu/common/timers_opencv.hpp"
#include "pcl/gpu/common/safe_call.hpp"

#include "data_gen.hpp"

#include <opencv2/contrib/contrib.hpp>

using namespace pcl::gpu;
using namespace std;

//TEST(PCL_OctreeGPU, DISABLED_perfomance)
TEST(PCL_OctreeGPU, perfomance)
{
    int device;
    cudaSafeCall( cudaGetDevice( &device ) );    
    cudaDeviceProp prop;
    cudaSafeCall( cudaGetDeviceProperties( &prop, device) );
    cout << "Device: " << prop.name << endl;

    DataGenerator data;
    data.data_size = 871000;
    data.tests_num = 10000;
    data.cube_size = 1024.f;
    data.max_radius    = data.cube_size/15.f;
    data.shared_radius = data.cube_size/15.f;
    data.printParams();

    //generate data
    data();

    //prepare device cloud
    pcl::gpu::Octree::PointCloud cloud_device;
    cloud_device.upload(data.points);

    //prepare queries_device
    pcl::gpu::Octree::BatchQueries queries_device;
    queries_device.upload(data.queries);  

    //prepare host cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_host(new pcl::PointCloud<pcl::PointXYZ>);	
    cloud_host->width = data.points.size();
    cloud_host->height = 1;
    cloud_host->points.resize (cloud_host->width * cloud_host->height);
    for (size_t i = 0; i < cloud_host->points.size(); ++i)
        cloud_host->points[i] = pcl::PointXYZ(data.points[i].x, data.points[i].y, data.points[i].z);

    float host_octree_resolution = 25.f;
    cout << "[!] Host octree resolution: " << host_octree_resolution << endl;    

    cout << "======  Build perfomance =====" << endl;
    // build device octree
    pcl::gpu::Octree octree_device;                
    octree_device.setCloud(cloud_device);	    
    {
        ScopeTimerCV up("gpu-build");	                
        octree_device.build();
    }    
    {
        ScopeTimerCV up("gpu-download");	
        octree_device.internalDownload();
    }

    //build host octree
    pcl::octree::OctreePointCloud<pcl::PointXYZ> octree_host(host_octree_resolution);
    octree_host.setInputCloud (cloud_host);
    {
        ScopeTimerCV t("host-build");	        
        octree_host.addPointsFromInputCloud();
    }

    // build opencv octree
    cv::Octree octree_opencv;
    const static int opencv_octree_points_per_leaf = 32;
    vector<cv::Point3f>& opencv_points = (vector<cv::Point3f>&)data.points;        
    {        
        ScopeTimerCV t("opencv-build");	        
        octree_opencv.buildTree(opencv_points, 10, opencv_octree_points_per_leaf); 
    }
    
    //// Radius search perfomance ///

    const int max_answers = 500;

    //host buffers
    vector<int> indeces;
    vector<float> pointRadiusSquaredDistance;
    vector<cv::Point3f> opencv_results;

    //reserve
    indeces.reserve(data.data_size);
    pointRadiusSquaredDistance.reserve(data.data_size);
    opencv_results.reserve(data.data_size);

    //device buffers
    pcl::gpu::DeviceArray_<int> bruteforce_results_device, buffer(cloud_device.size());    
    pcl::gpu::Octree::BatchResult      result_device(queries_device.size() * max_answers);
    pcl::gpu::Octree::BatchResultSizes  sizes_device(queries_device.size());

    cout << "======  Separate radius for each query =====" << endl;
    {
        ScopeTimerCV up("gpu-search-{host}-all");	
        for(size_t i = 0; i < data.tests_num; ++i)
            octree_device.radiusSearchHost(data.queries[i], data.radiuses[i], indeces, max_answers);                        
    }

    {                
        ScopeTimerCV up("host-search-all");	
        for(size_t i = 0; i < data.tests_num; ++i)
            octree_host.radiusSearch(pcl::PointXYZ(data.queries[i].x, data.queries[i].y, data.queries[i].z), 
                data.radiuses[i], indeces, pointRadiusSquaredDistance, max_answers);                        
    }
     
    {
        ScopeTimerCV up("gpu_bruteforce-search-all");	         
        for(size_t i = 0; i < data.tests_num; ++i)
            pcl::gpu::bruteForceRadiusSearchGPU(cloud_device, data.queries[i], data.radiuses[i], bruteforce_results_device, buffer);
    }

    cout << "======  Shared radius (" << data.shared_radius << ") =====" << endl;
    
    {
        ScopeTimerCV up("gpu-search-batch-all");	        
        octree_device.radiusSearchBatchGPU(queries_device, data.shared_radius, max_answers, result_device, sizes_device);                        
    }

    {
        ScopeTimerCV up("gpu-search-{host}-all");
        for(size_t i = 0; i < data.tests_num; ++i)
            octree_device.radiusSearchHost(data.queries[i], data.shared_radius, indeces, max_answers);                        
    }

    {                
        ScopeTimerCV up("host-search-all");	
        for(size_t i = 0; i < data.tests_num; ++i)
            octree_host.radiusSearch(pcl::PointXYZ(data.queries[i].x, data.queries[i].y, data.queries[i].z), 
                data.radiuses[i], indeces, pointRadiusSquaredDistance, max_answers);                        
    }
     
    {
        ScopeTimerCV up("gpu-bruteforce-search-all");	         
        for(size_t i = 0; i < data.tests_num; ++i)
            pcl::gpu::bruteForceRadiusSearchGPU(cloud_device, data.queries[i], data.shared_radius, bruteforce_results_device, buffer);
    }
}