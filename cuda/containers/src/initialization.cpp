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

#include "pcl/gpu/containers/initialization.hpp"
#include "cuda_runtime_api.h"

//temporary dependence, should eliminated in future
//Replace with THROW_PCL_EXCEPTION from PCL2.0 API
#include "pcl/gpu/utils/safe_call.hpp"



#define HAVE_CUDA
//#include "pcl_config.h"


#if !defined(HAVE_CUDA)

void throw_nogpu() { throw "PCL 2.0 exception"; }
int  pcl::gpu::getCudaEnabledDeviceCount() { return 0; }
void pcl::gpu::setDevice(int device) { throw_nogpu(); }

#else


int pcl::gpu::getCudaEnabledDeviceCount()
{
    int count;
    cudaError_t error = cudaGetDeviceCount( &count );

    if (error == cudaErrorInsufficientDriver)
        return -1;

    if (error == cudaErrorNoDevice)
        return 0;

    cudaSafeCall(error);
    return count;  
}

void pcl::gpu::setDevice(int device)
{
    cudaSafeCall( cudaSetDevice( device ) );
}


#endif