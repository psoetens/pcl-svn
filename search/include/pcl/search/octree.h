/*
 * Software License Agreement (BSD License)
 *
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


#ifndef PCL_SEARCH_OCTREE_H
#define PCL_SEARCH_OCTREE_H

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include "pcl/octree/octree_base.h"
#include "pcl/octree/octree_pointcloud.h"
#include "pcl/octree/octree_nodes.h"
#include "pcl/search/search.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>

namespace pcl
{
  namespace search
  {
    /** \brief @b search::Octree is a wrapper class which inherits octree::Octree class for performing search functions using Octree structure. 
     *  \note Octree implementation for pointclouds. Only indices are stored by the octree leaf nodes (zero-copy).
     *  \note The octree pointcloud class needs to be initialized with its voxel resolution. Its bounding box is automatically adjusted
     *  \note according to the pointcloud dimension or it can be predefined.
     *  \note Note: The tree depth equates to the resolution and the bounding box dimensions of the octree.
     *  \note
     *  \note typename: PointT: type of point used in pointcloud
     *  \note typename: LeafT:  leaf node class (usuallz templated with integer indices values)
     *  \note typename: OctreeT: octree implementation ()
     *  \ingroup octree
     *  \author Julius Kammerl (julius@kammerl.de)
     */
    template<typename PointT, typename LeafTWrap = pcl::octree::OctreeLeafDataTVector<int> , typename OctreeT = pcl::octree::OctreeBase<int, LeafTWrap> >
    class Octree : public pcl::search::Search<PointT>, public OctreeT
    {
      public:
        // public typedefs
        typedef boost::shared_ptr<std::vector<int> > IndicesPtr;
        typedef boost::shared_ptr<const std::vector<int> > IndicesConstPtr;

        typedef pcl::PointCloud<PointT> PointCloud;
        typedef boost::shared_ptr<PointCloud> PointCloudPtr;
        typedef boost::shared_ptr<const PointCloud> PointCloudConstPtr;

        // Boost shared pointers
        typedef boost::shared_ptr<pcl::octree::OctreePointCloud<PointT, LeafTWrap, OctreeT> > Ptr;
        typedef boost::shared_ptr<const pcl::octree::OctreePointCloud<PointT, LeafTWrap, OctreeT> > ConstPtr;
        Ptr tree_;

        /** \brief Octree constructor.
         *  \param resolution_arg: octree resolution at lowest octree level
         * */
        Octree (const double resolution_arg)
        {
          tree_.reset (new pcl::octree::OctreePointCloud<PointT, LeafTWrap, OctreeT> (resolution_arg));
        }

        /** \brief Empty Destructor. */
        virtual ~Octree () {}

	/** \brief Provide a pointer to the input dataset.
        * \param cloud_arg the const boost shared pointer to a PointCloud message
        */
        inline void
        setInputCloud (const PointCloudConstPtr &cloud_arg)
        {
          tree_->deleteTree ();
          tree_->setInputCloud (cloud_arg);
          tree_->addPointsFromInputCloud ();
        }
	/** \brief Provide a pointer to the input dataset.
        * \param cloud_arg the const boost shared pointer to a PointCloud message
        * \param indices_arg the point indices subset that is to be used from \a cloud - if NULL the whole point cloud is used
        */
        inline void
        setInputCloud (const PointCloudConstPtr &cloud_arg, const IndicesConstPtr& indices_arg)
        {
          tree_->deleteTree ();
          tree_->setInputCloud (cloud_arg,indices_arg);
          tree_->addPointsFromInputCloud ();
        }

        /** \brief search for k-nearest neighbors at the query point.
         * \param cloud_arg the point cloud data
         * \param index_arg the index in \a cloud representing the query point
         * \param k_arg the number of neighbors to search for
         * \param k_indices_arg the resultant indices of the neighboring points (must be resized to \a k a priori!)
         * \param k_sqr_distances_arg the resultant squared distances to the neighboring points (must be resized to \a k
         * a priori!)
         * \return number of neighbors found
         */
        int
        nearestKSearch (const PointCloudConstPtr &cloud_arg, 
                        int index_arg, 
                        int k_arg, 
                        std::vector<int> &k_indices_arg,
                        std::vector<float> &k_sqr_distances_arg);

        /** \brief search for k-nearest neighbors for the given query point.
        * \param cloud the point cloud data
        * \param index the index in \a cloud representing the query point
        * \param k the number of neighbors to search for
        * \param k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k
        * a priori!)
        * \return number of neighbors found
        */
	inline int
        nearestKSearch (const PointCloud& cloud, 
                        int index, 
                        int k, 
                        std::vector<int>& k_indices, 
                        std::vector<float>& k_sqr_distances)
        {
          PCL_ERROR("[pcl::search::Octree::nearestKSearch] This function is not supported by Octree\n");
          return (0);
        }
 	/** \brief search for all the nearest neighbors of the query point in a given radius.
         * \param cloud the point cloud data
         * \param index the index in \a cloud representing the query point
         * \param radius the radius of the sphere bounding all of p_q's neighbors
         * \param k_indices the resultant indices of the neighboring points
         * \param k_distances the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        inline int
        radiusSearch (const PointCloud &cloud,
                      int index, 
                      double radius,
                      std::vector<int> &k_indices, 
                      std::vector<float> &k_distances,
                      int max_nn) 
        {
          PCL_ERROR("[pcl::search::Octree::radiusSearch] This function is not supported by Octree\n");
          return (0);
        }
	/** \brief Approximate search for all the nearest neighbors of the query point in a given radius.
        * \param cloud the const boost shared pointer to a PointCloud message
        * \param index the index in \a cloud representing the query point
        * \param radius the radius of the sphere bounding all of point's neighbors
        * \param k_indices the resultant indices of the neighboring points
        * \param k_distances the resultant squared distances to the neighboring points
        * \param max_nn if given, bounds the maximum returned neighbors to this value
        * \return number of neighbors found in radius
        */
        inline int
        approxRadiusSearch (const PointCloudConstPtr &cloud, 
                            int index, 
                            double radius,
                            std::vector<int> &k_indices, 
                            std::vector<float> &k_distances,
                            int max_nn) const
        {
          PCL_ERROR("[pcl::search::Octree::approxRadiusSearch] This function is not supported by Octree\n");
          return (0);
        }
  	/** \brief Approximate search for k-nearest neighbors for the given query point.
        * \param cloud the const boost shared pointer to a PointCloud message
        * \param index the index in \a cloud representing the query point
        * \param k the number of neighbors to search for
        * \param k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k
        * a priori!)
  	* \return number of neighbors found
        */
        inline int
        approxNearestKSearch (const PointCloudConstPtr &cloud, 
                              int index, 
                              int k, 
                              std::vector<int> &k_indices, 
                              std::vector<float> &k_sqr_distances)
        {
          PCL_ERROR("[pcl::search::Octree::approxNearestKSearch] This function is not supported by Octree\n");
          return (0);
        }

        /** \brief Evaluate the search Methods for the given cloud.
        * \param cloud the const boost shared pointer to a PointCloud message
        * \param search_type the search type NEAREST_K_SEARCH and NEAREST_RADIUS_SEARCH
        */
        inline void
        evaluateSearchMethods (const PointCloudConstPtr& cloud, const int search_type)
        {
          PCL_ERROR("[pcl::search::Octree::evaluateSearchMethods] This function is not supported by Octree\n");
        }

	/** \brief search for k-nearest neighbors for the given query points.
        * \param point the given query points
        * \param k the numbers of the query point's neighbors to search for
        * \param k_indices the resultant indices of the neighboring points 
        * \param k_sqr_distances the resultant squared distances to the neighboring points 
        * \return number of neighbors found
        */
        inline int
        nearestKSearch (std::vector<PointT, Eigen::aligned_allocator<PointT> > &point, 
                        std::vector<int> &k, 
                        std::vector<std::vector<int> > &k_indices,
                        std::vector<std::vector<float> > &k_sqr_distances)
        {
          PCL_ERROR("[pcl::search::Octree::nearestKSearch] This function is not supported by Octree\n");
          return (0);
        }

	/** \brief search for all the nearest neighbors of the query points in the given radiuses.
        * \param point the given query points
        * \param radii the radiuses of the sphere bounding all of point's neighbors
        * \param k_indices the resultant indices of the neighboring points
        * \param k_distances the resultant squared distances to the neighboring points
        * \param max_nn if given, bounds the maximum returned neighbors to this value
        * \return number of neighbors found in radiuses
        */
        inline int
        radiusSearch (std::vector<PointT, Eigen::aligned_allocator<PointT> > &point, 
                      std::vector<double> &radii, 
                      std::vector<std::vector<int> > &k_indices,
                      std::vector<std::vector<float> > &k_distances, 
                      int max_nn) const
        {
          PCL_ERROR("[pcl::search::Octree::radiusSearch] This function is not supported by Octree\n");
          return (0);
        }

        /** \brief search for k-nearest neighbors at given query point.
         * @param p_q_arg the given query point
         * @param k_arg the number of neighbors to search for
         * @param k_indices_arg the resultant indices of the neighboring points (must be resized to k a priori!)
         * @param k_sqr_distances_arg  the resultant squared distances to the neighboring points (must be resized to k a priori!)
         * @return number of neighbors found
         */
        int
        nearestKSearch (const PointT &p_q_arg, int k_arg, std::vector<int> &k_indices_arg,
                        std::vector<float> &k_sqr_distances_arg);

        /** \brief Search for k-nearest neighbors at query point
         * \param index_arg index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector.
         * \param k_arg the number of neighbors to search for
         * \param k_indices_arg the resultant indices of the neighboring points (must be resized to \a k a priori!)
         * \param k_sqr_distances_arg the resultant squared distances to the neighboring points (must be resized to \a k
         * a priori!)
         * \return number of neighbors found
         */
        int
        nearestKSearch (int index_arg, int k_arg, std::vector<int> &k_indices_arg,
                        std::vector<float> &k_sqr_distances_arg);

        /** \brief search for approximate nearest neighbor at the query point.
         * \param cloud_arg the point cloud data
         * \param query_index_arg the index in \a cloud representing the query point
         * \param result_index_arg the resultant index of the neighbor point
         * \param sqr_distance_arg the resultant squared distance to the neighboring point
         * \return number of neighbors found
         */
        void
        approxNearestSearch (const PointCloudConstPtr &cloud_arg, int query_index_arg, int &result_index_arg,
                             float &sqr_distance_arg);

        /** \brief search for approximate nearest neighbor at the query point.
         * @param p_q_arg the given query point
         * \param result_index_arg the resultant index of the neighbor point
         * \param sqr_distance_arg the resultant squared distance to the neighboring point
         */
        void
        approxNearestSearch (const PointT &p_q_arg, int &result_index_arg, float &sqr_distance_arg);

        /** \brief search for approximate nearest neighbor at the query point.
         * \param query_index_arg index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector.
         * \param result_index_arg the resultant index of the neighbor point
         * \param sqr_distance_arg the resultant squared distance to the neighboring point
         * \return number of neighbors found
         */
        void
        approxNearestSearch (int query_index_arg, int &result_index_arg, float &sqr_distance_arg);

        /** \brief search for all neighbors of query point that are within a given radius.
         * \param cloud_arg the point cloud data
         * \param index_arg the index in \a cloud representing the query point
         * \param radius_arg the radius of the sphere bounding all of p_q's neighbors
         * \param k_indices_arg the resultant indices of the neighboring points
         * \param k_sqr_distances_arg the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (const PointCloudConstPtr &cloud_arg, int index_arg, double radius_arg,
                      std::vector<int> &k_indices_arg, std::vector<float> &k_sqr_distances_arg,
                      int max_nn = INT_MAX) ;

        /** \brief search for all neighbors of query point that are within a given radius.
         * \param p_q_arg the given query point
         * \param radius_arg the radius of the sphere bounding all of p_q's neighbors
         * \param k_indices_arg the resultant indices of the neighboring points
         * \param k_sqr_distances_arg the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (const PointT &p_q_arg, const double radius_arg, std::vector<int> &k_indices_arg,
                      std::vector<float> &k_sqr_distances_arg, int max_nn = INT_MAX) const;

        /** \brief search for all neighbors of query point that are within a given radius.
         * \param index_arg index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector
         * \param radius_arg radius of the sphere bounding all of p_q's neighbors
         * \param k_indices_arg the resultant indices of the neighboring points
         * \param k_sqr_distances_arg the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (int index_arg, const double radius_arg, std::vector<int> &k_indices_arg,
                      std::vector<float> &k_sqr_distances_arg, int max_nn = INT_MAX) const;
    };
  }
}

#endif    // PCL_SEARCH_OCTREE_H
 

