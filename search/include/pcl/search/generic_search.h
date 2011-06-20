/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
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

#ifndef PCL_SEARCH_GENERIC_SEARCH_H_
#define PCL_SEARCH_GENERIC_SEARCH_H_

#include <limits.h>
#include "pcl/pcl_macros.h"
#include "pcl/point_cloud.h"
#include "pcl/point_representation.h"
//#include "pcl/search/search_flann.h"

namespace pcl
{

const static int KDTREE_FLANN           = 0;
const static int ORGANIZED_INDEX        = 1;
const static int OCTREE                 = 2;

  template <typename PointT>
  class Search
  {

  public:
    typedef pcl::PointCloud<PointT> PointCloud;
    typedef boost::shared_ptr<PointCloud> PointCloudPtr;
    typedef boost::shared_ptr<const PointCloud> PointCloudConstPtr;
    typedef boost::shared_ptr<pcl::Search<PointT> > SearchPtr;
    typedef boost::shared_ptr<const pcl::Search<PointT> > SearchConstPtr;

    typedef boost::shared_ptr <std::vector<int> > IndicesPtr;
    typedef boost::shared_ptr <const std::vector<int> > IndicesConstPtr;




    Search(int spatial_locator)
    {
        initSearchDS(spatial_locator);

    }

    Search(){}


    ~Search(){}

    void initSearchDS(int spatial_locator);
    
    virtual void 
    setInputCloud (const PointCloudConstPtr& cloud, const IndicesConstPtr& indices);
    
    virtual void 
    setInputCloud (const PointCloudConstPtr& cloud);

    virtual int
    nearestKSearch (const PointT& point, int k, std::vector<int>& k_indices, std::vector<float>& k_sqr_distances);

    virtual int
    nearestKSearch (const PointCloud& cloud, int index, int k, std::vector<int>& k_indices, std::vector<float>& k_sqr_distances);

    virtual int
    nearestKSearch (int index, int k, std::vector<int>& k_indices, std::vector<float>& k_sqr_distances);

    virtual int 
    radiusSearch (const PointT& point, const double radius, std::vector<int>& k_indices,    std::vector<float>& k_distances, int max_nn ) const;

//    virtual int 
  //  radiusSearch (const PointT& point, double radius,
    //              std::vector<int>& k_indices, std::vector<float> &k_sqr_distances_arg);

//    virtual int 
//    radiusSearch (const PointT& point, double radius, std::vector<int>& k_indices,    std::vector<float>& k_distances);

    virtual int
    radiusSearch (const PointCloud& cloud, int index, double radius,
                  std::vector<int>& k_indices, std::vector<float>& k_distances,
                  int max_nn = -1);

    virtual int
    radiusSearch (int index, double radius, std::vector<int>& k_indices,
                  std::vector<float>& k_distances, int max_nn = -1) const;




  private:
    SearchPtr _searchptr;
    int spatial_loc;
  };

}

#endif  //#ifndef _PCL_SEARCH_GENERIC_SEARCH_H_
