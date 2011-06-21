/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Alexandru-Eugen Ichim
 *                      Willow Garage, Inc
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


#ifndef PCL_PPF_REGISTRATION_H_
#define PCL_PPF_REGISTRATION_H_

#include "pcl/registration/registration.h"
#include <pcl/features/ppf.h>
#include <boost/unordered_map.hpp>

namespace pcl
{
  class PCL_EXPORTS PPFHashMapSearch
  {
    public:
      /** \brief Data structure to hold the information for the key in the feature hash map of the
        * PPFHashMapSearch class
        * \note It uses multiple pair levels in order to enable the usage of the boost::hash function
        * which has the std::pair implementation (i.e., does not require a custom hash function)
        */
      struct HashKeyStruct : public std::pair <int, std::pair <int, std::pair <int, int> > >
      {
        HashKeyStruct(int a, int b, int c, int d)
        {
          this->first = a;
          this->second.first = b;
          this->second.second.first = c;
          this->second.second.second = d;
        }
      };
      typedef boost::unordered_multimap<HashKeyStruct, std::pair<size_t, size_t> > FeatureHashMapType;
      typedef boost::shared_ptr<FeatureHashMapType> FeatureHashMapTypePtr;
      typedef boost::shared_ptr<PPFHashMapSearch> Ptr;

      PPFHashMapSearch (float a_angle_discretization_step = 12.0 / 180 * M_PI,
                        float a_distance_discretization_step = 0.01)
      :  angle_discretization_step (a_angle_discretization_step),
         distance_discretization_step (a_distance_discretization_step)
      {
        feature_hash_map = FeatureHashMapTypePtr (new FeatureHashMapType);
        internals_initialized = false;
        max_dist = -1.0;
      }


      void
      setInputFeatureCloud (PointCloud<PPFSignature>::ConstPtr feature_cloud);

      void
      nearestNeighborSearch (float &f1, float &f2, float &f3, float &f4,
                             std::vector<std::pair<size_t, size_t> > &indices);

      Ptr
      makeShared() { return Ptr (new PPFHashMapSearch (*this)); }

      std::vector <std::vector <float> > alpha_m;

      float
      getAngleDiscretizationStep () { return angle_discretization_step; }

      float
      getDistanceDiscretizationStep () { return distance_discretization_step; }

      float
      getModelDiameter () { return max_dist; }

    private:
      FeatureHashMapTypePtr feature_hash_map;
      bool internals_initialized;

      /// parameters
      float angle_discretization_step, distance_discretization_step;

      float max_dist;
  };

  /** \brief 
    * \author Alexandru-Eugen Ichim
    */
  template <typename PointSource, typename PointTarget>
  class PPFRegistration : public Registration<PointSource, PointTarget>
  {
    public:
      /** \brief Structure for storing a pose (represented as an Eigen::Affine3f) and an integer for counting votes
        * \note initially used std::pair<Eigen::Affine3f, unsigned int>, but it proved problematic
        * because of the Eigen structures alignment problems - std::pair does not have a custom allocator
        */
      struct PoseWithVotes
      {
        PoseWithVotes(Eigen::Affine3f &a_pose, unsigned int &a_votes)
        : pose (a_pose),
          votes (a_votes)
        {}

        Eigen::Affine3f pose;
        unsigned int votes;
      };
      typedef std::vector<PoseWithVotes, Eigen::aligned_allocator<PoseWithVotes> > PoseWithVotesList;

      /// input_ is the model cloud
      using Registration<PointSource, PointTarget>::input_;
      /// target_ is the scene cloud
      using Registration<PointSource, PointTarget>::target_;
      using Registration<PointSource, PointTarget>::converged_;
      using Registration<PointSource, PointTarget>::final_transformation_;
      using Registration<PointSource, PointTarget>::transformation_;

      typedef pcl::PointCloud<PointSource> PointCloudSource;
      typedef typename PointCloudSource::Ptr PointCloudSourcePtr;
      typedef typename PointCloudSource::ConstPtr PointCloudSourceConstPtr;

      typedef pcl::PointCloud<PointTarget> PointCloudTarget;
      typedef typename PointCloudTarget::Ptr PointCloudTargetPtr;
      typedef typename PointCloudTarget::ConstPtr PointCloudTargetConstPtr;


      /** \brief Constructor that initializes all the parameters of the algorithm
        * \param a_scene_reference_point_sampling_rate sampling rate for the scene reference point
        * \param a_clustering_position_diff_threshold distance threshold below which two poses are
        * considered close enough to be in the same cluster (for the clustering phase of the algorithm)
        * \param a_clustering_rotation_diff_threshold rotation difference threshold below which two
        * poses are considered to be in the same cluster (for the clustering phase of the algorithm)
        */
      PPFRegistration (unsigned int a_scene_reference_point_sampling_rate = 5,
                       float a_clustering_position_diff_threshold = 0.01,
                       float a_clustering_rotation_diff_threshold = 20.0 / 180 * M_PI)
      :  Registration<PointSource, PointTarget> (),
         scene_reference_point_sampling_rate (a_scene_reference_point_sampling_rate),
         clustering_position_diff_threshold (a_clustering_position_diff_threshold),
         clustering_rotation_diff_threshold (a_clustering_rotation_diff_threshold)
      {
        search_method = PPFHashMapSearch::Ptr ();
      }

      /** \brief Function that sets the search method for the algorithm
       * \note Right now, the only available method is the one initially proposed by
       * the authors - by using a hash map with discretized feature vectors
       * \param a_search_method smart pointer to the search method to be set
       */
      void
      setSearchMethod (PPFHashMapSearch::Ptr a_search_method) { search_method = a_search_method; }

      /** \brief Provide a pointer to the input target (e.g., the point cloud that we want to align the input source to)
       * \param cloud the input point cloud target
       */
      void
      setInputTarget (const PointCloudTargetConstPtr &cloud);


    private:

      /** \brief Method that calculates the transformation between the input_ and target_
       * point clouds, based on the PPF features */
      void
      computeTransformation (PointCloudSource &output);


      /** \brief the search method that is going to be used to find matching feature pairs */
      PPFHashMapSearch::Ptr search_method;
      /** \brief parameter for the sampling rate of the scene reference points */
      unsigned int scene_reference_point_sampling_rate;
      /** \brief position and rotation difference thresholds below which two
        * poses are considered to be in the same cluster (for the clustering phase of the algorithm) */
      float clustering_position_diff_threshold, clustering_rotation_diff_threshold;

      /** \brief use a kd-tree with range searches of range max_dist to skip an O(N) pass through the point cloud */
      typename pcl::KdTreeFLANN<PointTarget>::Ptr scene_search_tree;


      /** \brief static method used for the std::sort function to order two PoseWithVotes
       * instances by their number of votes*/
      static bool
      poseWithVotesCompareFunction (const PoseWithVotes &a,
                                    const PoseWithVotes &b);

      /** \brief static method used for the std::sort function to order two pairs <index, votes>
       * by the number of votes (unsigned integer value) */
      static bool
      clusterVotesCompareFunction (const std::pair<size_t, unsigned int> &a,
                                   const std::pair<size_t, unsigned int> &b);

      /** \brief Method that clusters a set of given poses by using the clustering thresholds
       * and their corresponding number of votes (see publication for more details) */
      void
      clusterPoses (PoseWithVotesList &poses,
                    PoseWithVotesList &result);

      /** \brief Method that checks whether two poses are close together - based on the clustering threshold parameters
       * of the class */
      bool
      posesWithinErrorBounds (Eigen::Affine3f &pose1,
                              Eigen::Affine3f &pose2);

  };
}

#include "pcl/registration/impl/ppf_registration.hpp"

#endif // PCL_PPF_REGISTRATION_H_
