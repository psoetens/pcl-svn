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

#ifndef PCL_GPU_FEATURES_INTERNAL_HPP_
#define PCL_GPU_FEATURES_INTERNAL_HPP_

#include "pcl/gpu/containers/device_array.hpp"
#include "pcl/gpu/features/device_format.hpp"
#include "pcl/gpu/octree/device_format.hpp"

#include <cuda_runtime.h>

namespace pcl
{
    namespace device
    {   
        using pcl::gpu::DeviceArray;
        using pcl::gpu::DeviceArray2D;
        using pcl::gpu::NeighborIndices;

        typedef float4 PointType;
        typedef float4 NormalType;

        typedef DeviceArray< PointType> PointCloud;
        typedef DeviceArray<NormalType> Normals;
        typedef DeviceArray<int> Indices;

        struct PFHSignature125
        {
            float histogram[125];
        };

        struct FPFHSignature33
        {
            float histogram[33];
        };

        struct PfhImpl
        {
            PointCloud cloud;
            Normals normals;

            NeighborIndices neighbours;

            DeviceArray2D<float> data_rpk;
            int max_elems_rpk;

            void compute(DeviceArray2D<PFHSignature125>& features);

        private:
            void repack();
        };

        void computeSPFH(const PointCloud& surface, const Normals& normals, const Indices& indices, const NeighborIndices& neighbours, DeviceArray2D<FPFHSignature33>& spfh33);
        void computeFPFH(const PointCloud& cloud, const NeighborIndices& neighbours, const DeviceArray2D<FPFHSignature33>& spfh, DeviceArray2D<FPFHSignature33>& features);

        void computeFPFH(const PointCloud& cloud, const Indices& indices, const PointCloud& surface, 
            const NeighborIndices& neighbours, DeviceArray<int>& lookup, const DeviceArray2D<FPFHSignature33>& spfh, DeviceArray2D<FPFHSignature33>& features);
                

        void computeNormals(const PointCloud& cloud, const NeighborIndices& nn_indices, Normals& normals);
        void flipNormalTowardsViewpoint(const PointCloud& cloud, const float3& vp, Normals& normals);        
        void flipNormalTowardsViewpoint(const PointCloud& cloud, const Indices& indices, const float3& vp, Normals& normals);


        int computeUniqueIndices(size_t surface_size, const NeighborIndices& neighbours, DeviceArray<int>& unique_indices, DeviceArray<int>& lookup);
    }
}

#endif /* PCL_GPU_FEATURES_INTERNAL_HPP_ */
