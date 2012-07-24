/*
 * Software License Agreement (BSD License)
 *
 * Point Cloud Library (PCL) - www.pointclouds.org
 * Copyright (c) 2009-2011, Willow Garage, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * * Neither the name of Willow Garage, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ACTIVE_SEGMENTATION_H_
#define ACTIVE_SEGMENTATION_H_

#include <pcl/search/pcl_search.h>
#include <pcl/pcl_base.h>

#include <pcl/features/boundary.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/organized_edge_detection.h>

#include <Eigen/Core>

namespace pcl
{
  /////////////////////////////////////////////////////////////////////
  /**
    * \brief stand alone method for doing active segmentation
    * \param[in] cloud_in input cloud
    * \param[in] boundary the boundary map
    * \param[in] normals normals for the input cloud
    * \param[in] tree the spatial locator used for radius search
    * \param[in] index of the fixation point
    * \param[in] search_radius radius of search
    * \param[in] angle threshold for convexity check in degrees
    * \param[out] indices_out the resulting segment as indices of the input cloud
    */
  template <typename PointT> void
  activeSegmentation (const pcl::PointCloud<PointT>                         &cloud_in,
                      const pcl::PointCloud<pcl::Boundary>                  &boundary,
                      const pcl::PointCloud<pcl::Normal>                    &normals,
                      const boost::shared_ptr<pcl::search::Search<PointT> > &tree,
                      int                                                   fp_index,
                      float                                                 search_radius,
                      double                                                eps_angle,
                      pcl::PointIndices                                     &indices_out);

  /**
    * \brief
    * \author Ferenc Balint-Benczedi
    * \ingroup segmentation
    * \description Active segmentation or segmentation around a fixation point as the authors call it,
    *  extracts a region enclosed by a boundary based on a point inside this.
    *
    * \note If you use this code in any academic work, please cite:
    *
    *      - Ajay Mishra, Yiannis Aloimonos, Cornelia Fermuller
    *      Active Segmentation for Robotics
    *      In 2009 IEEERSJ International Conference on Intelligent Robots and Systems (2009)
    *
    */
  template<typename PointT, typename NormalT>
  class ActiveSegmentation : public PCLBase<PointT>
  {
    typedef pcl::search::Search<PointT> KdTree;
    typedef typename KdTree::Ptr KdTreePtr;
    typedef pcl::PointCloud<pcl::Boundary> Boundary;
    typedef typename Boundary::Ptr BoundaryPtr;

    typedef pcl::PointCloud<NormalT> Normal;
    typedef typename Normal::Ptr NormalPtr;

    using PCLBase<PointT>::input_;
    using PCLBase<PointT>::indices_;

    typedef pcl::PointCloud<PointT> PointCloud;

    public:
      /* \brief empty constructor */
      ActiveSegmentation () :
          tree_ (), normals_ (), boundary_ (), fixation_point_ (), fp_index_ (), search_radius_ (0.02),eps_angle_(89)
      {
      }

      /* \brief empty destructor */
      virtual ~ActiveSegmentation ()
      {
      }

      /** \brief Set the fixation point.
        * \param[in] p the fixation point
        */
      void
      setFixationPoint (const PointT &p);

      /** \brief Set the fixation point.
        * \param[in] x the X coordinate of the fixation point
        * \param[in] y the Y coordinate of the fixation point
        * \param[in] z the Z coordinate of the fixation point
        */
      inline void
      setFixationPoint (float x, float y, float z)
      {
        PointT p;
        p.x = x;
        p.y = y;
        p.z = z;
        setFixationPoint (p);
      }

      /* \brief Returns the fixation point as a Point struct. */
      PointT
      getFixationPoint ()
      {
        return (fixation_point_);
      }

      /** \brief Set the fixation point as an index in the input cloud.
        * \param[in] index the index of the point in the input cloud to use
        */
      inline void
      setFixationPoint (int index)
      {
        fixation_point_ = input_->points[index];
        fp_index_ = index;
      }

      /* \brief Returns the fixation point index. */
      int
      getFixationPointIndex ()
      {
        return (fp_index_);
      }

      /** \brief Provide a pointer to the search object.
        * \param[in] tree a pointer to the spatial search object.
        */
      inline void
      setSearchMethod (const KdTreePtr &tree)
      {
        tree_ = tree;
      }

      /** \brief returns a pointer to the search method used. */
      inline KdTreePtr
      getSearchMethod () const
      {
        return (tree_);
      }

      /** \brief Set the boundary map of the input cloud
        * \param[in] boundary a pointer to the boundary cloud
        */
      inline void
      setBoundaryMap (const BoundaryPtr &boundary)
      {
        boundary_ = boundary;
      }

      /* \brief returns a pointer to the boundary map currently set. */
      inline BoundaryPtr
      getBoundaryMap () const
      {
        return (boundary_);
      }

      /** \brief Set search radius for the region growing
        * \param[in] r the radius used
        */
      inline void
      setSearchRadius (double r)
      {
        search_radius_ = r;
      }

      /** \brief Set the input normals to be used for the segmentation
        * \param[in] norm the normals to be used
        */
      inline void
      setInputNormals (const NormalPtr &norm)
      {
        normals_ = norm;
      }

      /** \brief Set the angle threshold for convexity check in the region growing
       *  \param[in] eps_angle angle in degrees
       */
      inline void
      setAngleThreshold (double eps_angle)
      {
        eps_angle_ = eps_angle *M_PI / 180;
      }

      /* \brief returns angle threshold in degrees  */
      inline double
      getAngleThreshold () const
      {
        return (eps_angle_* 180/M_PI);
      }

      /** \brief returns a pointer to the normals */
      inline NormalPtr
      getInputNormals ()
      {
        return normals_;
      }

      /** \brief Method for segmenting the object that contains the fixation point
        * \param[out] indices_out
        */
      void
      segment (PointIndices &indices_out);

    private:

      /** \brief A pointer to the spatial search object. */
      KdTreePtr tree_;

      /** \brief A pointer to the normals of the object. */
      NormalPtr normals_;

      /**\brief A pointer to the boundary map associated with the cloud*/
      BoundaryPtr boundary_;

      /**\brief fixation point as a pcl:struct*/
      PointT fixation_point_;

      /** \brief fixation point as an index*/
      int fp_index_;

      /**\brief radius of search for region growing*/
      double search_radius_;

      /** \brief angle threshold for convexity check in the region growing*/
      double eps_angle_;

      /** \brief Checks if the angle between the vector connecting the fixation point and current point
        *  and the current points normal is greater then the set threshold
        * \return true if point can be added to segment
        * \param[in] index of point to be verified
        */
      bool
      isPointValid (int v_point);

      /** \brief verifies the existence of a boundary point among the indices returned by knn. Used to determine if point found can be seed candidates
        * \return true if there is a boundary point
        * \param[in] vector of indices
        */
      bool
      hasBoundaryPoint (std::vector<int>);
  };

}
#endif /* ACTIVE_SEGMENTATION_H_ */
