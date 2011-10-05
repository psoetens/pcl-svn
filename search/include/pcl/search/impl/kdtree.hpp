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
 */

#ifndef PCL_SEARCH_KDTREE_IMPL_H_
#define PCL_SEARCH_KDTREE_IMPL_H_

#include "pcl/search/kdtree.h"

////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT>
  void
  pcl::search::KdTree<PointT>::setInputCloud (const PointCloudConstPtr& cloud, const IndicesConstPtr& indices)
  {
    tree_->setInputCloud (cloud, indices);
  }

////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT>
  void
  pcl::search::KdTree<PointT>::setInputCloud (const PointCloudConstPtr& cloud)
  {
    const IndicesConstPtr& indices = IndicesConstPtr ();
    setInputCloud (cloud, indices);
  }

////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT>
  int
  pcl::search::KdTree<PointT>::nearestKSearch (const PointT &point, int k, std::vector<int> &k_indices,
                                               std::vector<float> &k_distances)
  {
    return (tree_->nearestKSearch (point, k, k_indices, k_distances));
  }

////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT>
  int
  pcl::search::KdTree<PointT>::radiusSearch (const PointT &point, double radius, std::vector<int> &k_indices,
                                             std::vector<float> &k_squared_distances, int max_nn) const
  {
    return (tree_->radiusSearch (point, radius, k_indices, k_squared_distances, max_nn));
  }

#define PCL_INSTANTIATE_KdTree(T) template class PCL_EXPORTS pcl::search::KdTree<T>;

#endif  //#ifndef PCL_SEARCH_KDTREE_IMPL_H_
