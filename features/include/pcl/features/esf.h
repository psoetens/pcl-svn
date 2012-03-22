/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
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
 * ww
 */

#ifndef PCL_ESF_H_
#define PCL_ESF_H_

#include "pcl/features/feature.h"
#define GRIDSIZE 64
#define GRIDSIZE_H GRIDSIZE/2
#include "boost/multi_array.hpp"
#include <vector>
namespace pcl
{
  /** \brief @b ESFEstimation estimates the ensemble of shape functions descriptors for a given point cloud
    * dataset containing points. Shape functions are D2, D3, A3.  For more information about the ESF descriptor, see:
    *
    *
    * \author Walter Wohlkinger
    * \ingroup features
    */

  template <typename PointInT,  typename PointOutT = pcl::ESFSignature640>
  class ESFEstimation: public Feature<PointInT, PointOutT>
  {
    public:
    using Feature<PointInT, PointOutT>::feature_name_;
    using Feature<PointInT, PointOutT>::getClassName;
    using Feature<PointInT, PointOutT>::indices_;
    using Feature<PointInT, PointOutT>::k_;
    using Feature<PointInT, PointOutT>::search_radius_;
    using Feature<PointInT, PointOutT>::input_;
    using Feature<PointInT, PointOutT>::surface_;

      typedef typename pcl::PointCloud<PointInT> PointCloudIn;
      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;

      /** \brief Empty constructor. */
      ESFEstimation ()
      {
        feature_name_ = "ESFEstimation";
        lut.resize(boost::extents[GRIDSIZE][GRIDSIZE][GRIDSIZE]);
        search_radius_ = 0;
        k_ = 5;
      }



      /** \brief Estimate the Ensebmel of Shape Function (ESF) descriptors at a set of points given by
        * <setInputCloud (),
        * \param output the resultant point cloud model histogram that contains the ESF feature estimates
        */
      void 
      computeFeature (PointCloudOut &output);

    protected:

      int      lci(const int x1, const int y1, const int z1, const int x2, const int y2, const int z2, float &ratio, int &incnt, int &pointcount);
      void     computeESF (PointCloudIn &pc, std::vector<float> &hist);
      void     voxelize9 (PointCloudIn &cluster);
      void     cleanup9 (PointCloudIn &cluster);
      void     scale_points_unit_sphere (const pcl::PointCloud<PointInT> &pc, float scalefactor, Eigen::Vector4f& centroid);



    private:

      boost::multi_array<int, 3> lut;
      PointCloudIn local_cloud;
      /** \brief Make the computeFeature (&Eigen::MatrixXf); inaccessible from outside the class
        * \param[out] output the output point cloud
        */
      void
      computeFeatureEigen (pcl::PointCloud<Eigen::MatrixXf> &) {}

  };
}

#endif // #
