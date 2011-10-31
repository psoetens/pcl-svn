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

#include "device.hpp"

using namespace pcl::device;

namespace pcl
{
    namespace device
    {        

        struct ImageGenerator
        {
            enum 
            {
                CTA_SIZE_X = 32, CTA_SIZE_Y = 8
            };

            PtrStep<float> vmap;
            PtrStep<float> nmap;

            LightSource light;

            mutable PtrStepSz<uchar3> dst;

            __device__ __forceinline__ void operator()()const
            {
                int x = threadIdx.x + blockIdx.x * CTA_SIZE_X;
                int y = threadIdx.y + blockIdx.y * CTA_SIZE_Y;

                if (x >= dst.cols || y >= dst.rows)
                    return;

                float3 v, n;
                v.x = vmap.ptr(y)[x];
                n.x = nmap.ptr(y)[x];

                uchar3 color = make_uchar3(0, 0, 0);

                if (!isnan(v.x) && !isnan(n.x))
                {
                    v.y = vmap.ptr(y+  dst.rows)[x];
                    v.z = vmap.ptr(y+2*dst.rows)[x];

                    n.y = nmap.ptr(y+  dst.rows)[x];
                    n.z = nmap.ptr(y+2*dst.rows)[x];

                    float weight = 1.f; 

                    for(int i = 0; i < light.number; ++i)
                    {						
                        float3 vec = normalized(light.pos[i] - v);

                        weight *= fabs(dot(vec, n));					
                    }

                    int br = (int)(255*weight) + 50;
                    br = max(0, min(255, br));
                    color = make_uchar3(br, br, br);
                }				
                dst.ptr(y)[x] = color;
            }
        };

        __global__ void generateImageKernel(const ImageGenerator ig) { ig(); }		
    }
}


void pcl::device::generateImage(const MapArr& vmap, const MapArr& nmap, const LightSource& light, PtrStepSz<uchar3> dst)
{
    ImageGenerator ig;
    ig.vmap = vmap;
    ig.nmap = nmap;
    ig.light = light;	
    ig.dst = dst;

    dim3 block(ImageGenerator::CTA_SIZE_X, ImageGenerator::CTA_SIZE_Y);
    dim3 grid(divUp(dst.cols, block.x), divUp(dst.rows, block.y));

    generateImageKernel<<<grid, block>>>(ig);
    cudaSafeCall( cudaGetLastError() );
    cudaSafeCall(cudaDeviceSynchronize());
}