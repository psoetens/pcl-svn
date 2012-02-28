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
 *
 *
 */

#ifndef PCL_SEGMENTATION_ORGANIZED_MULTI_PLANE_SEGMENTATION_H_
#define PCL_SEGMENTATION_ORGANIZED_MULTI_PLANE_SEGMENTATION_H_

#include "pcl/pcl_base.h"
#include "pcl/PointIndices.h"
#include "pcl/ModelCoefficients.h"
#include "pcl/segmentation/plane_coefficient_comparator.h"

namespace pcl
{
  /** \brief OrganizedMultiPlaneSegmentation finds all planes present in the input cloud, and outputs a vector of plane equations, as well as a vector of point clouds corresponding to the inliers of each detected plane.
   * Only planes with more than min_inliers points are detected.
   *
   * \author Alex Trevor, Suat Gedikli
   */
  template<typename PointT, typename PointNT, typename PointLT>
  class OrganizedMultiPlaneSegmentation : public PCLBase<PointT>
  {
    using PCLBase<PointT>::input_;
    using PCLBase<PointT>::indices_;
    using PCLBase<PointT>::initCompute;
    using PCLBase<PointT>::deinitCompute;

    public:
      typedef pcl::PointCloud<PointT> PointCloud;
      typedef typename PointCloud::Ptr PointCloudPtr;
      typedef typename PointCloud::ConstPtr PointCloudConstPtr;

      typedef typename pcl::PointCloud<PointNT> PointCloudN;
      typedef typename PointCloudN::Ptr PointCloudNPtr;
      typedef typename PointCloudN::ConstPtr PointCloudNConstPtr;

      typedef typename pcl::PointCloud<PointLT> PointCloudL;
      typedef typename PointCloudL::Ptr PointCloudLPtr;
      typedef typename PointCloudL::ConstPtr PointCloudLConstPtr;

      OrganizedMultiPlaneSegmentation () :
        min_inliers_ (1000), angular_threshold_ (3.0 * 0.017453), distance_threshold_ (0.02)
      {
      }

      virtual
      ~OrganizedMultiPlaneSegmentation ()
      {
      }

      /** \brief Provide a pointer to the input normals.
        * \param[in] normals the input normal cloud
        */
      inline void
      setInputNormals (const PointCloudNConstPtr &normals) 
      {
        normals_ = normals;
      }

      /** \brief Get the input normals. */
      inline PointCloudNConstPtr
      getInputNormals () const
      {
        return (normals_);
      }

      /** \brief Set the minimum number of inliers required for a plane
        * \param[in] min_inliers the minimum number of inliers required per plane
        */
      inline void
      setMinInliers (int min_inliers)
      {
        min_inliers_ = min_inliers;
      }

      /** \brief Get the minimum number of inliers required per plane. */
      inline int
      getMinInliers () const
      {
        return (min_inliers_);
      }

      /** \brief Set the tolerance in radians for difference in normal direction between neighboring points, to be considered part of the same plane.
        * \param[in] angular_threshold the tolerance in radians
        */
      inline void
      setAngularThreshold (double angular_threshold)
      {
        angular_threshold_ = angular_threshold;
      }

      /** \brief Get the angular threshold in radians for difference in normal direction between neighboring points, to be considered part of the same plane. */
      inline double
      getAngularThreshold () const
      {
        return (angular_threshold_);
      }

      /** \brief Set the tolerance in meters for difference in perpendicular distance (d component of plane equation) to the plane between neighboring points, to be considered part of the same plane.
        * \param[in] distance_threshold the tolerance in meters
        */
      inline void
      setDistanceThreshold (double distance_threshold)
      {
        distance_threshold_ = distance_threshold;
      }

      /** \brief Get the distance threshold in meters (d component of plane equation) between neighboring points, to be considered part of the same plane. */
      inline double
      getDistanceThreshold () const
      {
        return (distance_threshold_);
      }

      /** \brief Segmentation of all planes in a point cloud given by setInputCloud(), setIndices()
        * \param[out] model_coefficients a vector of model_coefficients for each plane found in the input cloud
        * \param[out] inlier_indices a vector of inliers for each detected plane
        * \param[out] labels a point cloud for the connected component labels of each pixel
        * \param[out] label_indices a vector of PointIndices for each labeled component
        */
      virtual void
      segment (std::vector<ModelCoefficients>& model_coefficients, std::vector<PointIndices>& inlier_indices, pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices);

      /** \brief Segmentation of all planes in a point cloud given by setInputCloud(), setIndices()
        * \param[out] model_coefficients a vector of model_coefficients for each plane found in the input cloud
        * \param[out] inlier_indices a vector of inliers for each detected plane
        */
      virtual void
      segment (std::vector<ModelCoefficients>& model_coefficients, std::vector<PointIndices>& inlier_indices);

    protected:

      /** \brief A pointer to the input normals */
      PointCloudNConstPtr normals_;

      /** \brief The minimum number of inliers required for each plane. */
      int min_inliers_;

      /** \brief The tolerance in radians for difference in normal direction between neighboring points, to be considered part of the same plane. */
      double angular_threshold_;

      /** \brief The tolerance in meters for difference in perpendicular distance (d component of plane equation) to the plane between neighboring points, to be considered part of the same plane. */
      double distance_threshold_;

      /** \brief Class getName method. */
      virtual std::string
      getClassName () const
      {
        return ("OrganizedMultiPlaneSegmentation");
      }
  };

}

#endif //#ifndef PCL_SEGMENTATION_ORGANIZED_MULTI_PLANE_SEGMENTATION_H_
