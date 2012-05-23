/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2012, Willow Garage, Inc.
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
 * $Id: edge.h nsomani $
 *
 */

#ifndef PCL_2D_EDGE_H
#define PCL_2D_EDGE_H

#include <vector>
#include "convolution_2d.h"
using namespace std;
namespace pcl{
	namespace pcl_2d{
		class edge{
		private :
			convolution_2d *conv_2d;
		public :			
			edge(){
				conv_2d = new convolution_2d();
			}
			void traceEdge(int rowOffset,int colOffset,int row,int col, float theta, float tLow, float tHigh, vector<vector<float> > &G, vector<vector<float> > &thet);
			void canny(vector<vector<float> > &output, vector<vector<float> > &input);
			void sobelXY(vector<vector<float> > &Gx, vector<vector<float> > &Gy, vector<vector<float> > &input);
			void sobel(vector<vector<float> > &G, vector<vector<float> > &thet, vector<vector<float> > &input);	
			void prewittXY(vector<vector<float> > &Gx, vector<vector<float> > &Gy, vector<vector<float> > &input);
			void prewitt(vector<vector<float> > &G, vector<vector<float> > &thet, vector<vector<float> > &input);				
			void robertsXY(vector<vector<float> > &Gx, vector<vector<float> > &Gy, vector<vector<float> > &input);
			void roberts(vector<vector<float> > &G, vector<vector<float> > &thet, vector<vector<float> > &input);				
			void LoGKernel(vector<vector<float> > &kernel, int dim, float sigma);
			void LoG(vector<vector<float> > &output, int dim, float sigma, vector<vector<float> > &input);
			void LoG(vector<vector<float> > &output, vector<vector<float> > &input);
		};
	}
}
#include <pcl/2d/impl/edge.hpp>
#endif // PCL_FILTERS_CONVOLUTION_3D_H
