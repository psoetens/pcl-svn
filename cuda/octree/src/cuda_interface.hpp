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

#ifndef PCL_GPU_CUDA_OCTREE_
#define PCL_GPU_CUDA_OCTREE_

#include "pcl/gpu/containers/device_array.hpp"

#include "octree_global.hpp"
#include "builder/tasks_global.hpp"

namespace pcl
{
    namespace gpu
    {          
        class OctreeImpl
        {
        public:
            typedef float4 PointType;
            typedef DeviceArray<PointType> PointArray;

            typedef PointArray PointCloud;
            typedef PointArray BatchQueries;
                       
            typedef DeviceArray<float> BatchRadiuses;
            typedef DeviceArray<int> BatchResult;            
            typedef DeviceArray<int> BatchResultSizes;
            typedef DeviceArray<float> BatchResultSqrDists;

            static void get_gpu_arch_compiled_for(int& bin, int& ptr);

            OctreeImpl(int max_threads_x_arg) : max_threads_x(max_threads_x_arg) {};
            ~OctreeImpl() {};

            void setCloud(const PointCloud& input_points);           
            void build();
            void radiusSearchHost(const PointType& center, float radius, std::vector<int>& out, int max_nn) const;
            void approxNearestSearchHost(const PointType& query, int& out_index, float& sqr_dist) const;
            
            void radiusSearchBatch(const BatchQueries& queries, float radius, int max_results, BatchResult& output, BatchResultSizes& out_sizes);

            void radiusSearchBatch(const BatchQueries& queries, const BatchRadiuses& radiuses, int max_results, BatchResult& output, BatchResultSizes& out_sizes);


            void approxNearestSearchBatch(const BatchQueries& queries, BatchResult& out) const;

            //void nearestKSearchBatch(const BatchQueries& queries, int k, BatchResult& results, BatchResultSqrDists& sqr_dists) const;

            //just reference 
            PointCloud points;

            // data
            DeviceArray2D<float> points_sorted;
            DeviceArray<int> codes;
            DeviceArray<int> indices;
                        
            pcl::device::TasksGlobal tasksGlobal;
            pcl::device::OctreeGlobalWithBox octreeGlobal;    

            //storage
            DeviceArray2D<int> storage;            

            struct OctreeDataHost
            {
                std::vector<int> nodes;
                std::vector<int> codes;	

                std::vector<int> begs;
                std::vector<int> ends;	
                

                std::vector<int> indices;	
                
                std::vector<float> points_sorted;
                int points_sorted_step;

                int downloaded;

            } host_octree;

                        
            void internalDownload(); 
            int max_threads_x;
        private:
            template<typename Batch>
            void radiusSearchBatchEx(Batch& batch, const BatchQueries& queries, int max_results, BatchResult& output, BatchResultSizes& out_sizes);
        };

        void bruteForceRadiusSearch(const OctreeImpl::PointCloud& cloud, const OctreeImpl::PointType& query, float radius, DeviceArray<int>& result, DeviceArray<int>& buffer);

    }
}

#endif /* PCL_GPU_CUDA_OCTREE_ */
