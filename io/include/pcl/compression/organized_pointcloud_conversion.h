/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2011, Willow Garage, Inc.
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
 * $Id$
 * Authors: Julius Kammerl (julius@kammerl.de)
 */

#ifndef PCL_ORGANIZED_CONVERSION_H_
#define PCL_ORGANIZED_CONVERSION_H_

#include <pcl/pcl_macros.h>
#include <pcl/point_cloud.h>

#include <vector>
#include <limits>
#include <assert.h>

namespace pcl
{
  namespace io
  {

    template<typename PointT> struct CompressionPointTraits
    {
        static const bool hasColor = false;
        static const unsigned int channels = 1;
        static const size_t bytesPerPoint = 3 * sizeof(float);
    };

    template<>
    struct CompressionPointTraits<PointXYZRGB>
    {
        static const bool hasColor = true;
        static const unsigned int channels = 4;
        static const size_t bytesPerPoint = 3 * sizeof(float) + 3 * sizeof(uint8_t);
    };

    template<>
    struct CompressionPointTraits<PointXYZRGBA>
    {
        static const bool hasColor = true;
        static const unsigned int channels = 4;
        static const size_t bytesPerPoint = 3 * sizeof(float) + 3 * sizeof(uint8_t);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    template <typename PointT, bool enableColor = CompressionPointTraits<PointT>::hasColor >
    struct OrganizedConversion;

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Uncolored point cloud specialization
    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT>
    struct OrganizedConversion<PointT, false>
    {
      /** \brief Convert point cloud to disparity image
        * \param[in] cloud_arg input point cloud
        * \param[in] maxDepth_arg maximum depth in point cloud
        * \param[in] depthQuantization_arg inverse depth quantization parameter
        * \param[out] disparityData_arg output disparity image
        * \ingroup io
        */
      static void convert(const pcl::PointCloud<PointT>& cloud_arg,
                          float maxDepth_arg,
                          float depthQuantization_arg,
                          typename std::vector<uint16_t>& disparityData_arg,
                          typename std::vector<uint8_t>&)
      {
        size_t cloud_size, i;

        cloud_size = cloud_arg.points.size ();

        // Clear image data
        disparityData_arg.clear ();

        if (maxDepth_arg > 0)
        {
          disparityData_arg.reserve (cloud_size);

          // Inverse depth quantization parameters
          float depthQuantA = depthQuantization_arg * (depthQuantization_arg + 1.0f);
          float depthQuantB = 1.0f - depthQuantA / maxDepth_arg;

          for (i = 0; i < cloud_size; ++i)
          {
            // Get point from cloud
            const PointT& point = cloud_arg.points[i];

            if (pcl::isFinite (point))
            {
              // Inverse depth quantization
              uint16_t depth = static_cast<uint16_t> (depthQuantA / point.z + depthQuantB);
              disparityData_arg.push_back (depth);
            }
            else
            {
              // Non-valid points are encoded with zeros
              disparityData_arg.push_back (0);
            }
          }

        }
        else
        {
          // Generate zero image
          disparityData_arg.resize (cloud_size, 0);
        }

      }

      /** \brief Convert disparity image to point cloud
        * \param[in] disparityData_arg input depth image
        * \param[in] width_arg width of disparity image
        * \param[in] height_arg height of disparity image
        * \param[in] maxDepth_arg maximum depth
        * \param[in] depthQuantization_arg inverse depth quantization parameter
        * \param[in] focalLength_arg focal length
        * \param[out] cloud_arg output point cloud
        * \ingroup io
        */
      static void convert(typename std::vector<uint16_t>& disparityData_arg,
                          typename std::vector<uint8_t>&,
                          size_t width_arg,
                          size_t height_arg,
                          float maxDepth_arg,
                          float depthQuantization_arg,
                          float focalLength_arg,
                          pcl::PointCloud<PointT>& cloud_arg)
      {
        size_t i;
        size_t cloud_size = width_arg * height_arg;
        int x, y, centerX, centerY;

        assert(disparityData_arg.size()==cloud_size);

        // Reset point cloud
        cloud_arg.points.clear ();
        cloud_arg.points.reserve (cloud_size);

        // Define point cloud parameters
        cloud_arg.width = static_cast<uint32_t> (width_arg);
        cloud_arg.height = static_cast<uint32_t> (height_arg);
        cloud_arg.is_dense = false;

        // Calculate center of disparity image
        centerX = static_cast<int> (width_arg / 2);
        centerY = static_cast<int> (height_arg / 2);

        const float fl_const = 1.0f / focalLength_arg;
        static const float bad_point = std::numeric_limits<float>::quiet_NaN ();

        if (maxDepth_arg > 0)
        {
          // Inverse depth quantization parameters
          float depthQuantA = depthQuantization_arg * (depthQuantization_arg + 1.0f);
          float depthQuantB = 1.0f - depthQuantA / maxDepth_arg;

          i = 0;
          for (y = -centerY; y < +centerY; ++y)
            for (x = -centerX; x < +centerX; ++x)
            {
              PointT newPoint;
              const uint16_t& pixel_depth = disparityData_arg[i];
              ++i;

              if (pixel_depth)
              {
                // Inverse depth decoding
                float depth = depthQuantA / (static_cast<float> (pixel_depth) - depthQuantB);

                // Generate new points
                newPoint.x = static_cast<float> (x) * depth * fl_const;
                newPoint.y = static_cast<float> (y) * depth * fl_const;
                newPoint.z = depth;

              }
              else
              {
                // Generate bad point
                newPoint.x = newPoint.y = newPoint.z = bad_point;
              }

              cloud_arg.points.push_back (newPoint);
            }

        }
        else
        {
          // Reconstruct point cloud of bad points
          PointT newPoint;
          newPoint.x = newPoint.y = newPoint.z = bad_point;

          for (i = 0; i < cloud_size; ++i)
            cloud_arg.points.push_back (newPoint);
        }

      }

    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Colored point cloud specialization
    //////////////////////////////////////////////////////////////////////////////////////////////
    template <typename PointT>
    struct OrganizedConversion<PointT, true>
    {
      /** \brief Convert point cloud to disparity image and rgb image
        * \param[in] cloud_arg input point cloud
        * \param[in] maxDepth_arg maximum depth in point cloud
        * \param[in] depthQuantization_arg inverse depth quantization parameter
        * \param[out] disparityData_arg output disparity image
        * \param[out] rgbData_arg output rgb image
        * \ingroup io
        */
      static void convert(const pcl::PointCloud<PointT>& cloud_arg,
                          float maxDepth_arg,
                          float depthQuantization_arg,
                          typename std::vector<uint16_t>& disparityData_arg,
                          typename std::vector<uint8_t>& rgbData_arg)
      {
        size_t cloud_size, i;

        cloud_size = cloud_arg.points.size ();

        // Reset output vectors
        disparityData_arg.clear ();
        rgbData_arg.clear ();

        if (maxDepth_arg > 0)
        {
          // Allocate memory
          disparityData_arg.reserve (cloud_size);
          rgbData_arg.reserve (cloud_size * 3);

          // Inverse depth quantization parameters
          float depthQuantA = depthQuantization_arg
              * (depthQuantization_arg + 1.0f);
          float depthQuantB = 1.0f - depthQuantA / maxDepth_arg;

          for (i = 0; i < cloud_size; ++i)
          {
            const PointT& point = cloud_arg.points[i];

            if (pcl::isFinite (point))
            {
              // Encode point color
              const uint32_t rgb = *reinterpret_cast<const int*> (&point.rgb);

              rgbData_arg.push_back ( (rgb >> 16) & 0x0000ff);
              rgbData_arg.push_back ( (rgb >> 8) & 0x0000ff);
              rgbData_arg.push_back ( (rgb >> 0) & 0x0000ff);

              // Inverse depth quantization
              uint16_t depth = static_cast<uint16_t> (depthQuantA / point.z + depthQuantB);

              // Encode disparity
              disparityData_arg.push_back (depth);
            }
            else
            {
              // Encode black point
              rgbData_arg.push_back (0);
              rgbData_arg.push_back (0);
              rgbData_arg.push_back (0);

              // Encode bad point
              disparityData_arg.push_back (0);
            }
          }

        }
        else
        {
          // Generate zero disparity and rgb image
          rgbData_arg.resize (cloud_size * 3, 0);
          disparityData_arg.resize (cloud_size, 0);
        }

      }

      /** \brief Convert disparity image to point cloud
        * \param[in] disparityData_arg output disparity image
        * \param[in] rgbData_arg output rgb image
        * \param[in] width_arg width of disparity image
        * \param[in] height_arg height of disparity image
        * \param[in] maxDepth_arg maximum depth
        * \param[in] depthQuantization_arg inverse depth quantization parameter
        * \param[in] focalLength_arg focal length
        * \param[out] cloud_arg output point cloud
        * \ingroup io
        */
      static void convert(typename std::vector<uint16_t>& disparityData_arg,
                          typename std::vector<uint8_t>& rgbData_arg,
                          size_t width_arg,
                          size_t height_arg,
                          float maxDepth_arg,
                          float depthQuantization_arg,
                          float focalLength_arg,
                          pcl::PointCloud<PointT>& cloud_arg)
      {
        size_t i;
        size_t cloud_size = width_arg*height_arg;
        bool hasColor = (rgbData_arg.size () > 0);

        // Check size of input data
        assert (disparityData_arg.size()==cloud_size);
        if (hasColor)
        {
          assert (rgbData_arg.size()==cloud_size*3);
        }

        int x, y, centerX, centerY;

        // Reset point cloud
        cloud_arg.points.clear();
        cloud_arg.points.reserve(cloud_size);

        // Define point cloud parameters
        cloud_arg.width = static_cast<uint32_t>(width_arg);
        cloud_arg.height = static_cast<uint32_t>(height_arg);
        cloud_arg.is_dense = false;

        // Calculate center of disparity image
        centerX = static_cast<int>(width_arg/2);
        centerY = static_cast<int>(height_arg/2);

        const float fl_const = 1.0f/focalLength_arg;
        static const float bad_point = std::numeric_limits<float>::quiet_NaN ();

        if (maxDepth_arg>0)
        {
          // Inverse depth quantization parameters
          float depthQuantA = depthQuantization_arg * (depthQuantization_arg + 1.0f);
          float depthQuantB = 1.0f - depthQuantA / maxDepth_arg;

          i = 0;
          for (y=-centerY; y<+centerY; ++y )
            for (x=-centerX; x<+centerX; ++x )
            {
              PointT newPoint;

              const uint16_t& pixel_disparity = disparityData_arg[i];

              if (pixel_disparity)
              {
                float depth = depthQuantA  /  ( static_cast<float> (pixel_disparity) - depthQuantB );

                // Define point location
                newPoint.z = depth;
                newPoint.x = static_cast<float> (x) * depth * fl_const;
                newPoint.y = static_cast<float> (y) * depth * fl_const;

                if (hasColor)
                {
                  const uint8_t& pixel_r = rgbData_arg[i*3+0];
                  const uint8_t& pixel_g = rgbData_arg[i*3+1];
                  const uint8_t& pixel_b = rgbData_arg[i*3+2];

                  // Define point color
                  uint32_t rgb = (static_cast<uint32_t>(pixel_r) << 16
                                | static_cast<uint32_t>(pixel_g) << 8
                                | static_cast<uint32_t>(pixel_b));
                  newPoint.rgb = *reinterpret_cast<float*>(&rgb);
                } else
                {
                  // Set white point color
                  uint32_t rgb = (static_cast<uint32_t>(255) << 16
                                | static_cast<uint32_t>(255) << 8
                                | static_cast<uint32_t>(255));
                  newPoint.rgb = *reinterpret_cast<float*>(&rgb);
                }
              } else
              {
                // Define bad point
                newPoint.x = newPoint.y = newPoint.z = bad_point;
                newPoint.rgb = 0.0f;
              }

              // Add point to cloud
              cloud_arg.points.push_back(newPoint);
              // Increment point iterator
              ++i;
          }

        } else
        {
          PointT newPoint;

          // Define bad point
          newPoint.x = newPoint.y = newPoint.z = bad_point;
          newPoint.rgb = 0.0;

          // Generate point cloud of bad points
          for (i=0; i<cloud_size; ++i )
            cloud_arg.points.push_back(newPoint);
        }
      }
    };

  }
}


#endif
