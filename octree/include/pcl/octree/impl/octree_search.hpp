/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 * Author: Julius Kammerl (julius@kammerl.de)
 */

#ifndef OCTREE_SEARCH_HPP_
#define OCTREE_SEARCH_HPP_

#include <vector>
#include <assert.h>

#include "pcl/common/common.h"

namespace pcl
{
  namespace octree
  {

    using namespace std;

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::voxelSearch (const PointT& point_arg,
                                                                   std::vector<int>& pointIdx_data_arg)
      {
        OctreeKey key;
        bool bSuccess = false;

        // generate key
        genOctreeKeyforPoint (point_arg, key);

        LeafT* leaf = getLeaf (key);

        if (leaf)
        {
          leaf->getData (pointIdx_data_arg);
          bSuccess = true;
        }

        return bSuccess;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::voxelSearch (const int index_arg,
                                                                   std::vector<int>& pointIdx_data_arg)
      {

        const PointT searchPoint = this->getPointByIndex (index_arg);

        return this->voxelSearch (searchPoint, pointIdx_data_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::nearestKSearch (const PointCloudConstPtr &cloud_arg,
                                                                      int index_arg, int k_arg,
                                                                      std::vector<int> &k_indices_arg,
                                                                      std::vector<float> &k_sqr_distances_arg)
      {
        this->setInputCloud (cloud_arg);
        this->addPointsFromInputCloud ();

        return nearestKSearch (index_arg, k_arg, k_indices_arg, k_sqr_distances_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::nearestKSearch (const PointT &p_q_arg, int k_arg,
                                                                      std::vector<int> &k_indices_arg,
                                                                      std::vector<float> &k_sqr_distances_arg)
      {

        unsigned int i;
        unsigned int resultCount;

        prioPointQueueEntry pointEntry;
        std::vector<prioPointQueueEntry> pointCandidates;

        assert (this->leafCount_>0);

        OctreeKey key;
        key.x = key.y = key.z = 0;

        // initalize smallest point distance in search with high value
        double smallestDist = numeric_limits<double>::max ();

        k_indices_arg.clear ();
        k_sqr_distances_arg.clear ();

        getKNearestNeighborRecursive (p_q_arg, k_arg, this->rootNode_, key, 1, smallestDist, pointCandidates);

        resultCount = pointCandidates.size ();

        for (i = 0; i < resultCount; i++)
        {
          pointEntry = pointCandidates.back ();

          k_indices_arg.push_back (pointEntry.pointIdx_);
          k_sqr_distances_arg.push_back (pointEntry.pointDistance_);

          pointCandidates.pop_back ();
        }

        return k_indices_arg.size ();

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::nearestKSearch (int index_arg, int k_arg,
                                                                      std::vector<int> &k_indices_arg,
                                                                      std::vector<float> &k_sqr_distances_arg)
      {

        const PointT searchPoint = this->getPointByIndex (index_arg);

        return nearestKSearch (searchPoint, k_arg, k_indices_arg, k_sqr_distances_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearch (const PointCloudConstPtr &cloud_arg,
                                                                           int query_index_arg, int &result_index_arg,
                                                                           float &sqr_distance_arg)
      {
        this->setInputCloud (cloud_arg);
        this->addPointsFromInputCloud ();

        return approxNearestSearch (query_index_arg, result_index_arg, sqr_distance_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearch (const PointT &p_q_arg,
                                                                           int &result_index_arg,
                                                                           float &sqr_distance_arg)
      {
        assert (this->leafCount_>0);

        OctreeKey key;
        key.x = key.y = key.z = 0;

        approxNearestSearchRecursive (p_q_arg, this->rootNode_, key, 1, result_index_arg, sqr_distance_arg);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearch (int query_index_arg, int &result_index_arg,
                                                                           float &sqr_distance_arg)
      {
        const PointT searchPoint = this->getPointByIndex (query_index_arg);

        return approxNearestSearch (searchPoint, result_index_arg, sqr_distance_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::radiusSearch (const PointCloudConstPtr &cloud_arg, int index_arg,
                                                                    double radius_arg, std::vector<int> &k_indices_arg,
                                                                    std::vector<float> &k_sqr_distances_arg,
                                                                    int max_nn_arg)
      {
        this->setInputCloud (cloud_arg);
        this->addPointsFromInputCloud ();

        return radiusSearch (index_arg, radius_arg, k_indices_arg, k_sqr_distances_arg, max_nn_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::radiusSearch (const PointT &p_q_arg, const double radius_arg,
                                                                    std::vector<int> &k_indices_arg,
                                                                    std::vector<float> &k_sqr_distances_arg,
                                                                    int max_nn_arg) const
      {

        OctreeKey key;
        key.x = key.y = key.z = 0;

        k_indices_arg.clear ();
        k_sqr_distances_arg.clear ();

        getNeighborsWithinRadiusRecursive (p_q_arg, radius_arg * radius_arg, this->rootNode_, key, 1, k_indices_arg,
                                           k_sqr_distances_arg, max_nn_arg);

        return k_indices_arg.size ();

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::radiusSearch (int index_arg,
                                                                    const double radius_arg,
                                                                    std::vector<int> &k_indices_arg,
                                                                    std::vector<float> &k_sqr_distances_arg,
                                                                    int max_nn_arg) const
      {

        const PointT searchPoint = this->getPointByIndex (index_arg);

        return radiusSearch (searchPoint, radius_arg, k_indices_arg, k_sqr_distances_arg, max_nn_arg);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      double
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::getKNearestNeighborRecursive (
                                                                                    const PointT & point_arg,
                                                                                    unsigned int K_arg,
                                                                                    const OctreeBranch* node_arg,
                                                                                    const OctreeKey& key_arg,
                                                                                    unsigned int treeDepth_arg,
                                                                                    const double squaredSearchRadius_arg,
                                                                                    std::vector<prioPointQueueEntry>& pointCandidates_arg) const
      {

        std::vector<prioBranchQueueEntry> searchEntryHeap;
        searchEntryHeap.resize (8);

        unsigned char childIdx;

        OctreeKey newKey;

        double smallestSquaredDist = squaredSearchRadius_arg;

        // get spatial voxel information
        double voxelSquaredDiameter = this->getVoxelSquaredDiameter (treeDepth_arg);

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          if (branchHasChild (*node_arg, childIdx))
          {

            PointT voxelCenter;

            searchEntryHeap[childIdx].key.x = (key_arg.x << 1) + (!!(childIdx & (1 << 2)));
            searchEntryHeap[childIdx].key.y = (key_arg.y << 1) + (!!(childIdx & (1 << 1)));
            searchEntryHeap[childIdx].key.z = (key_arg.z << 1) + (!!(childIdx & (1 << 0)));

            // generate voxel center point for voxel at key
            genVoxelCenterFromOctreeKey (searchEntryHeap[childIdx].key, treeDepth_arg, voxelCenter);

            // generate new priority queue element
            searchEntryHeap[childIdx].node = getBranchChild (*node_arg, childIdx);
            searchEntryHeap[childIdx].pointDistance = pointSquaredDist (voxelCenter, point_arg);

          }
          else
          {
            searchEntryHeap[childIdx].pointDistance = numeric_limits<double>::infinity ();
          }
        }

        std::sort (searchEntryHeap.begin (), searchEntryHeap.end ());

        // iterate over all children in priority queue
        // check if the distance to seach candidate is smaller than the best point distance (smallestSquaredDist)
        while ((!searchEntryHeap.empty ()) && (searchEntryHeap.back ().pointDistance < smallestSquaredDist
            + voxelSquaredDiameter / 4.0 + sqrt (smallestSquaredDist * voxelSquaredDiameter) - this->epsilon_))
        {

          const OctreeNode* childNode;

          // read from priority queue element
          childNode = searchEntryHeap.back ().node;
          newKey = searchEntryHeap.back ().key;

          if (treeDepth_arg < this->octreeDepth_)
          {
            // we have not reached maximum tree depth
            smallestSquaredDist = getKNearestNeighborRecursive (point_arg, K_arg, (OctreeBranch*)childNode, newKey,
                                                                treeDepth_arg + 1, smallestSquaredDist,
                                                                pointCandidates_arg);

          }
          else
          {
            // we reached leaf node level

            double squaredDist;
            size_t i;
            vector<int> decodedPointVector;

            OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

            // decode leaf node into decodedPointVector
            childLeaf->getData (decodedPointVector);

            // Linearly iterate over all decoded (unsorted) points
            for (i = 0; i < decodedPointVector.size (); i++)
            {

              const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

              // calculate point distance to search point
              squaredDist = pointSquaredDist (candidatePoint, point_arg);

              // check if a closer match is found
              if (squaredDist < smallestSquaredDist)
              {
                prioPointQueueEntry pointEntry;

                pointEntry.pointDistance_ = squaredDist;
                pointEntry.pointIdx_ = decodedPointVector[i];
                pointCandidates_arg.push_back (pointEntry);
              }
            }

            std::sort (pointCandidates_arg.begin (), pointCandidates_arg.end ());

            if (pointCandidates_arg.size () > K_arg)
              pointCandidates_arg.resize (K_arg);

            if (pointCandidates_arg.size () == K_arg)
            {
              smallestSquaredDist = pointCandidates_arg.back ().pointDistance_;
            }

          }

          // pop element from priority queue
          searchEntryHeap.pop_back ();
        }

        return smallestSquaredDist;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::getNeighborsWithinRadiusRecursive (
                                                                                         const PointT & point_arg,
                                                                                         const double radiusSquared_arg,
                                                                                         const OctreeBranch* node_arg,
                                                                                         const OctreeKey& key_arg,
                                                                                         unsigned int treeDepth_arg,
                                                                                         std::vector<int>& k_indices_arg,
                                                                                         std::vector<float>& k_sqr_distances_arg,
                                                                                         int max_nn_arg) const
      {
        // child iterator
        unsigned char childIdx;

        // get spatial voxel information
        double voxelSquaredDiameter = this->getVoxelSquaredDiameter (treeDepth_arg);

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          if (branchHasChild (*node_arg, childIdx))
          {
            const OctreeNode* childNode;
            childNode = getBranchChild (*node_arg, childIdx);

            OctreeKey newKey;
            PointT voxelCenter;
            double squaredDist;

            // generate new key for current branch voxel
            newKey.x = (key_arg.x << 1) + (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) + (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) + (!!(childIdx & (1 << 0)));

            // generate voxel center point for voxel at key
            genVoxelCenterFromOctreeKey (newKey, treeDepth_arg, voxelCenter);

            // calculate distance to search point
            squaredDist = pointSquaredDist ((const PointT &)voxelCenter, point_arg);

            // if distance is smaller than search radius
            if (squaredDist + this->epsilon_ <= voxelSquaredDiameter / 4.0 + radiusSquared_arg
                + sqrt (voxelSquaredDiameter * radiusSquared_arg))
            {

              if (treeDepth_arg < this->octreeDepth_)
              {
                // we have not reached maximum tree depth
                getNeighborsWithinRadiusRecursive (point_arg, radiusSquared_arg, (OctreeBranch*)childNode, newKey,
                                                   treeDepth_arg + 1, k_indices_arg, k_sqr_distances_arg, max_nn_arg);
                if (k_indices_arg.size () == (unsigned int)max_nn_arg)
                  return;
              }
              else
              {
                // we reached leaf node level

                size_t i;
                OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;
                vector<int> decodedPointVector;

                // decode leaf node into decodedPointVector
                childLeaf->getData (decodedPointVector);

                // Linearly iterate over all decoded (unsorted) points
                for (i = 0; i < decodedPointVector.size (); i++)
                {

                  const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

                  // calculate point distance to search point
                  squaredDist = pointSquaredDist (candidatePoint, point_arg);

                  // check if a match is found
                  if (squaredDist <= radiusSquared_arg)
                  {

                    // add point to result vector
                    k_indices_arg.push_back (decodedPointVector[i]);
                    k_sqr_distances_arg.push_back (squaredDist);

                    if (k_indices_arg.size () == (unsigned int)max_nn_arg)
                      return;
                  }
                }

              }

            }

          }

        }
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearchRecursive (const PointT & point_arg,
                                                                                    const OctreeBranch* node_arg,
                                                                                    const OctreeKey& key_arg,
                                                                                    unsigned int treeDepth_arg,
                                                                                    int& result_index_arg,
                                                                                    float& sqr_distance_arg)
      {

        unsigned char childIdx;
        unsigned char minChildIdx;
        double minVoxelCenterDistance;

        OctreeKey minChildKey;
        OctreeKey newKey;

        const OctreeNode* childNode;

        // set minimum voxel distance to maximum value
        minVoxelCenterDistance = numeric_limits<double>::max ();

        minChildIdx = 0xFF;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          if (branchHasChild (*node_arg, childIdx))
          {

            PointT voxelCenter;
            double voxelPointDist;

            newKey.x = (key_arg.x << 1) + (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) + (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) + (!!(childIdx & (1 << 0)));

            // generate voxel center point for voxel at key
            genVoxelCenterFromOctreeKey (newKey, treeDepth_arg, voxelCenter);

            voxelPointDist = pointSquaredDist (voxelCenter, point_arg);

            // search for child voxel with shortest distance to search point
            if (voxelPointDist < minVoxelCenterDistance)
            {
              minVoxelCenterDistance = voxelPointDist;
              minChildIdx = childIdx;
              minChildKey = newKey;
            }

          }
        }

        // make sure we found at least one branch child
        assert (minChildIdx<8);

        childNode = getBranchChild (*node_arg, minChildIdx);

        if (treeDepth_arg < this->octreeDepth_)
        {
          // we have not reached maximum tree depth
          approxNearestSearchRecursive (point_arg, (OctreeBranch*)childNode, minChildKey, treeDepth_arg + 1,
                                        result_index_arg, sqr_distance_arg);

        }
        else
        {
          // we reached leaf node level

          double squaredDist;
          double smallestSquaredDist;
          size_t i;
          vector<int> decodedPointVector;

          OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

          smallestSquaredDist = numeric_limits<double>::max ();

          // decode leaf node into decodedPointVector
          childLeaf->getData (decodedPointVector);

          // Linearly iterate over all decoded (unsorted) points
          for (i = 0; i < decodedPointVector.size (); i++)
          {

            const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

            // calculate point distance to search point
            squaredDist = pointSquaredDist (candidatePoint, point_arg);

            // check if a closer match is found
            if (squaredDist < smallestSquaredDist)
            {
              result_index_arg = decodedPointVector[i];
              sqr_distance_arg = smallestSquaredDist = squaredDist;
            }
          }
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      double
      OctreePointCloudSearch<PointT, LeafT, OctreeT>::pointSquaredDist (const PointT & pointA_arg,
                                                                        const PointT & pointB_arg) const
      {
        double distX, distY, distZ;

        // distance between pointA_arg and pointB_arg for each axis
        distX = pointA_arg.x - pointB_arg.x;
        distY = pointA_arg.y - pointB_arg.y;
        distZ = pointA_arg.z - pointB_arg.z;

        // return squared absolute distance between pointA_arg and pointB_arg
        return (distX * distX + distY * distY + distZ * distZ);

      }

  }
}

#endif
