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

#pragma once 

#include "pcl/gpu/utils/device/limits.hpp"
#include "pcl/gpu/utils/device/vector_math.hpp"

#include "internal.hpp"

namespace pcl
{
    namespace device
    {       
        const int DIVISOR = 32767; // SHRT_MAX;
        
        __device__ __forceinline__ short2 pack_tsdf(float tsdf, int weight)
		{
			//assumming that fabs(tsdf) <= 1 and weight is interger
            
            int fixedp = max(-DIVISOR, min(DIVISOR, __float2int_rz(tsdf * DIVISOR)));
            return make_short2(fixedp, weight);			
		}

		__device__ __forceinline__ void unpack_tsdf(short2 value, float& tsdf, int& weight)		
        {	            
            weight = value.y;
            tsdf = static_cast<float>(value.x)/DIVISOR;            
		}

         __device__ __forceinline__ float3 operator*(const Mat33& m, const float3& vec)
        {
            return make_float3(dot(m.data[0], vec), dot(m.data[1], vec), dot(m.data[2], vec));			
        }
    }
}