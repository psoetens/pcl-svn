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
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of Willow Garage, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#ifndef PCL_FILTERS_SAMPLING_SURFACE_NORMAL_H_
#define PCL_FILTERS_SAMPLING_SURFACE_NORMAL_H_

#include <pcl/filters/filter_indices.h>
#include <time.h>
#include <limits.h>

namespace pcl
{
  /** \brief @b SamplingSurfaceNormal divides the input space into grids until each grid contains a maximum of N points, 
   *  and samples points randomly within each grid. Normal is computed using the N points of each grid. All points
   *  sampled within a grid are assigned the same normal.
   *
   *  \author Aravindhan K Krishnan. This code is ported from libpointmatcher (https://github.com/ethz-asl/libpointmatcher)
   *  \ingroup filters
   */
  template<typename PointT>
  class SamplingSurfaceNormal: public FilterIndices<PointT>
  {
    using FilterIndices<PointT>::filter_name_;
    using FilterIndices<PointT>::getClassName;
    using FilterIndices<PointT>::indices_;
    using FilterIndices<PointT>::input_;

    typedef typename FilterIndices<PointT>::PointCloud PointCloud;
    typedef typename PointCloud::Ptr PointCloudPtr;
    typedef typename PointCloud::ConstPtr PointCloudConstPtr;

    typedef typename Eigen::Matrix<float, Eigen::Dynamic, 1> Vector;

    public:
      /** \brief Empty constructor. */
      SamplingSurfaceNormal () : 
        sample_ (10), seed_ (static_cast<unsigned int> (time (NULL)))
      {
        filter_name_ = "SamplingSurfaceNormal";
        srand (seed_);
      }

      /** \brief Set maximum number of samples in each grid
        * \param[in] maximum number of samples in each grid
        */
      inline void
      setSample (unsigned int sample)
      {
        sample_ = sample;
      }

      /** \brief Get the value of the internal \a sample parameter. */
      inline unsigned int
      getSample () const
      {
        return (sample_);
      }

      /** \brief Set seed of random function.
        * \param[in] seed the input seed
        */
      inline void
      setSeed (unsigned int seed)
      {
        seed_ = seed;
        srand (seed_);
      }

      /** \brief Get the value of the internal \a seed parameter. */
      inline unsigned int
      getSeed () const
      {
        return (seed_);
      }

      /** \brief Set ratio of points to be sampled in each grid
        * \param[in] sample the ratio of points to be sampled in each grid
        */
      inline void
      setRatio (float ratio)
      {
        ratio_ = ratio;
      }

      /** \brief Get the value of the internal \a ratio parameter. */
      inline float
      getRatio () const
      {
        return ratio_;
      }

    protected:

      /** \brief Maximum number of samples in each grid. */
      unsigned int sample_;
      /** \brief Random number seed. */
      unsigned int seed_;
      /** \brief Ratio of points to be sampled in each grid */
      float ratio_;

      /** \brief Sample of point indices into a separate PointCloud
        * \param[out] output the resultant point cloud
        */
      void
      applyFilter (PointCloud &output);

      /** \brief Sample of point indices
        * \param indices the resultant point cloud indices
        */
      void
      applyFilter (vector <int>& indices)
      {
      }

    private:

      /** \brief @b CompareDim is a comparator object for sorting across a specific dimenstion (i,.e X, Y or Z)
       */
      struct CompareDim
      {
        /** \brief The dimension to sort */
        const int dim;
        /** \brief The input point cloud to sort */
        const pcl::PointCloud <PointT>& cloud;

        /** \brief Constructor. */
        CompareDim (const int dim, const pcl::PointCloud <PointT>& cloud) : dim (dim), cloud (cloud)
        {
        }

        /** \brief The operator function for sorting. */
        bool operator () (const int& p0, const int& p1)
        {
          if (dim == 0)
          {
            return cloud.points[p0].x < cloud.points[p1].x;
          }
          else if (dim == 1)
          {
            return cloud.points[p0].y < cloud.points[p1].y;
          }
          else if (dim == 2)
          {
            return cloud.points[p0].z < cloud.points[p1].z;
          }
        }
      };

      /** \brief Finds the max and min values in each dimension
        * \param[out] max and min vectors
        */
      void findXYZMaxMin (const PointCloud& cloud, Vector& max_vec, Vector& min_vec);

      /** \brief Recursively partition the point cloud, stopping when each grid contains less than sample_ points
       *  Points are randomly sampled when a grid is found
        * \param[out] output the resultant point cloud
        */
      void partition (PointCloud&  cloud, const int first, const int last, const Vector minValues,
                      const Vector maxValues, std::vector <int>& indices, PointCloud& outcloud);

      /** \brief Randomly sample the points in each grid.
        * \param[out] output the resultant point cloud
        */
      void samplePartition (PointCloud& data, const int first, const int last, 
                            vector <int>& indices, PointCloud& outcloud);

      /** \brief Returns the threshold for splitting in a given dimension.
        */
      float findCutVal (PointCloud& cloud, const int cutDim, const int cutIndex);

      /** \brief Computes the normal for points in a grid. This is a port from features to avoid features dependency for
       *  filters
       *  \param cloud The input cloud
       *  \param[out] normal the computed normal
       *  \param[out] curvature the computed curvature
       */
      void computeNormal (PointCloud& cloud, Eigen::Vector4f &normal, float& curvature);

      /** \brief Computes the covariance matrix for points in the cloud. This is a port from features to avoid features dependency for
       *  filters
       *  \param cloud The input cloud
       *  \param[out] covariance_matrix the covariance matrix 
       *  \param[out] centroid the centroid
       */
      unsigned int computeMeanAndCovarianceMatrix (const pcl::PointCloud<PointT> &cloud,
                                                    Eigen::Matrix3f &covariance_matrix,
                                                    Eigen::Vector4f &centroid);

      /** \brief Solve the eigenvalues and eigenvectors of a given 3x3 covariance matrix, and estimate the least-squares
       * plane normal and surface curvature.
       * \param covariance_matrix the 3x3 covariance matrix
       * \param[out] (nx ny nz) plane_parameters the resultant plane parameters as: a, b, c, d (ax + by + cz + d = 0)
       * \param[out] curvature the estimated surface curvature as a measure of
       */
      void solvePlaneParameters (const Eigen::Matrix3f &covariance_matrix,
                                  float &nx, float &ny, float &nz, float &curvature);
  };
}
#endif  //#ifndef PCL_FILTERS_SAMPLING_SURFACE_NORMAL_H_
