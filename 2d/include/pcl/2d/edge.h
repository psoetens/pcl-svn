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
namespace pcl
{
  namespace pcl_2d
  {
    class edge
    {
      private:
        convolution_2d *conv_2d;
        /**
         * \param rowOffset
         * \param colOffset
         * \param row
         * \param col
         * \param maxima
         *
         * This function performs edge tracing for Canny Edge detector.
         * This is used in the hysteresis thresholding step.
         */
        void
        cannyTraceEdge (int rowOffset, int colOffset, int row, int col, ImageType &maxima);

        /** 
        * \param thet angle image passed by reference
        * 
        * Discretize the direction of gradient.
        */
        void 
        discretizeAngles (ImageType &thet);

        /** 
        * \param[in] G gradient image passed by reference
        * \param[in] thet angle image passed by reference
        * \param[out] maxima local maxima image passed by reference
        * \param[in] tLow lower threshold for edges
        * Suppress non maxima
        */
        void
        suppressNonMaxima (ImageType &G, ImageType &thet, ImageType &maxima, float tLow);

      public:
        edge  ()
        {
          conv_2d = new convolution_2d ();
        }

        /**
         *
         * \param output Output image passed by reference
         * \param input Input image passed by reference
         *
         * This is a convenience function which calls
         * canny(ImageType &output, ImageType &input, float t_low, float t_high)
         * with parameters t_low = 20 and t_high = 50.
         * These values were chosen because they seem to work well on a number of images which were used
         * in our experiments.
         */
        void canny  (ImageType &output, ImageType &input);

        /**
         * \param output Output image passed by reference
         * \param input Input image passed by reference
         * \param t_low lower threshold for edges
         * \param t_higher higher threshold for edges
         *
         * All edges of magnitude above t_high are always classified as edges. All edges below t_low are discarded.
         * Edge values between t_low and t_high are classified as edges only if they are connected to edges having magnitude > t_high
         * and are located in a direction perpendicular to that strong edge.
         */
        void canny  (ImageType &output, ImageType &input, float t_low, float t_high);

        /**
         * \param output Output image passed by reference
         * \param input_x Input image passed by reference for the first derivative in the horizontal direction
         * \param input_y Input image passed by reference for the first derivative in the vertical direction
         * \param t_low lower threshold for edges
         * \param t_higher higher threshold for edges
         *
         * Perform Canny edge detection with two separated input images for horizontal and vertical derivatives.
         * All edges of magnitude above t_high are always classified as edges. All edges below t_low are discarded.
         * Edge values between t_low and t_high are classified as edges only if they are connected to edges having magnitude > t_high
         * and are located in a direction perpendicular to that strong edge.
         */
        void
        canny (ImageType &output, ImageType &input_x, ImageType &input_y, float t_low, float t_high);

        /**
         * \param Gx Returns the gradients in x direction.
         * \param Gy Returns the gradients in y direction.
         * \param input Input image passed by reference
         *
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void sobelXY  (ImageType &Gx, ImageType &Gy, ImageType &input);

        /**
         * \param G Returns the gradients magnitude.
         * \param thet Returns the gradients direction.
         * \param input Input image passed by reference
         * 
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void sobelMagnitudeDirection  (ImageType &G, ImageType &thet, ImageType &input);

        /**
         * \param G Returns the gradients magnitude.
         * \param thet Returns the gradients direction.
         * \param input_x Input image passed by reference for the first derivative in the horizontal direction
         * \param input_y Input image passed by reference for the first derivative in the vertical direction
         * 
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void
        sobelMagnitudeDirection (ImageType &G, ImageType &thet, ImageType &input_x, ImageType &input_y);

        /**
         * \param Gx Returns the gradients in x direction.
         * \param Gy Returns the gradients in y direction.
         * \param input Input image passed by reference
         *
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void prewittXY  (ImageType &Gx, ImageType &Gy, ImageType &input);

        /**
         * \param G Returns the gradients magnitude.
         * \param thet Returns the gradients direction.
         * \param input Input image passed by reference
         *
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void prewittMagnitudeDirection  (ImageType &G, ImageType &thet, ImageType &input);

        /**
         * \param Gx Returns the gradients in x direction.
         * \param Gy Returns the gradients in y direction.
         * \param input Input image passed by reference
         *
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */

        void robertsXY  (ImageType &Gx, ImageType &Gy, ImageType &input);

        /**
         * \param G Returns the gradients magnitude.
         * \param thet Returns the gradients direction.
         * \param input Input image passed by reference
         *
         * Uses the Sobel kernel for edge detection.
         * This function does NOT include a smoothing step.
         * The image should be smoothed before using this function to reduce noise.
         *
         */
        void robertsMagnitudeDirection  (ImageType &G, ImageType &thet, ImageType &input);

        /**
         * \param kernel_size The kernel is of size kernel_size x kernel_size.
         * \param sigma This is the variance of the Gaussian smoothing.
         * \param input Input image passed by reference
         *
         * creates Laplcian of Gausian Kernel. This kernel is useful for detecting edges.     *
         */
        void LoGKernel  (ImageType &kernel, const int kernel_size, const float sigma);

        /**
         * \param kernel_size The kernel is of size kernel_size x kernel_size.
         * \param sigma This is the variance of the Gaussian smoothing.
         * \param input Input image passed by reference
         * \param output Output image passed by reference
         *
         * Uses the LoGKernel to apply LoG on the input image.
         * Zero crossings of the Laplacian operator applied on an image indicate edges.
         * Gaussian kernel is used to smoothen the image prior to the Laplacian. This is because Laplacian uses the
         * second order derivative of the image and hence, is very sensitive to noise.
         * The implementation is not two-step but rather applies the LoG kernel directly.
         */
        void LoG  (ImageType &output, const int kernel_size, const float sigma, ImageType &input);

        /**
         * \param output Output image passed by reference
         * \param input Input image passed by reference
         *
         * This is a convenience function which calls LoG(output, 9, 1.4, input);
         * These values were chosen because they seem to work well on a number of images which were used
         * in our experiments.
         */
        void LoG  (ImageType &output, ImageType &input);

        /**
         * \param output Output image passed by reference
         * \param input Input image passed by reference
         *
         * Computes the image derivative in x direction using central differences
         */
        void ComputeDerivativeXCentral  (ImageType &output, ImageType &input);

        /**
         * \param output Output image passed by reference
         * \param input Input image passed by reference
         *
         * Computes the image derivative in y direction using central differences
         */
        void ComputeDerivativeYCentral  (ImageType &output, ImageType &input);
    };
  }
}
#include <pcl/2d/impl/edge.hpp>
#endif // PCL_FILTERS_CONVOLUTION_3D_H
