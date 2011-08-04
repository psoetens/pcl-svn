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
 *
 */
#ifndef PCL_REGISTRATION_IMPL_CORRESPONDENCE_REJECTION_FEATURES_HPP_
#define PCL_REGISTRATION_IMPL_CORRESPONDENCE_REJECTION_FEATURES_HPP_

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::registration::CorrespondenceRejectorFeatures::applyRejection (
    pcl::registration::Correspondences &correspondences)
{
  // Go over the map of features
  //
  // For each set of features, call findFeatureCorrespondences (with K or Radius set a-priori)
  //
  // Check the indices obtained with the set of correspondences given as input
  // WARN: this only works if the clouds have the same size?!?!

/*  unsigned int number_valid_correspondences = 0;
  correspondences.resize (input_correspondences_->size ());
  for (size_t i = 0; i < input_correspondences_->size (); ++i)
  {
    if ((*input_correspondences_)[i].distance < max_distance_)
    {
      correspondences[number_valid_correspondences] = (*input_correspondences_)[i];
      ++number_valid_correspondences;
    }
  }
  correspondences.resize (number_valid_correspondences);*/
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::registration::CorrespondenceRejectorFeatures::getRemainingCorrespondences (
    const pcl::registration::Correspondences& original_correspondences, 
    pcl::registration::Correspondences& remaining_correspondences)
{
/*  size_t number_valid_correspondences = 0;
  remaining_correspondences.resize (original_correspondences.size ());
  for (size_t i = 0; i < original_correspondences.size (); ++i)
  {
    if (original_correspondences[i].distance < max_distance_)
    {
      remaining_correspondences[number_valid_correspondences] = original_correspondences[i];
      ++number_valid_correspondences;
    }
  }
  remaining_correspondences.resize (number_valid_correspondences);
  */
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline void 
pcl::registration::CorrespondenceRejectorFeatures::setSourceFeature (
    const typename pcl::PointCloud<FeatureType>::ConstPtr &source_feature, const std::string &key)
{
  if (features_map_.count (key) == 0)
    features_map_[key].reset (new FeatureContainer<FeatureType>);
  boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->setSourceFeature (source_feature);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline typename pcl::PointCloud<FeatureType>::ConstPtr 
pcl::registration::CorrespondenceRejectorFeatures::getSourceFeature (const std::string &key)
{
  if (features_map_.count (key) == 0)
    return (boost::shared_ptr<pcl::PointCloud<const FeatureType> > ());
  else
    return (boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->getSourceFeature ());
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline void 
pcl::registration::CorrespondenceRejectorFeatures::setTargetFeature (
    const typename pcl::PointCloud<FeatureType>::ConstPtr &target_feature, const std::string &key)
{
  if (features_map_.count (key) == 0)
    features_map_[key].reset (new FeatureContainer<FeatureType>);
  boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->setTargetFeature (target_feature);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline typename pcl::PointCloud<FeatureType>::ConstPtr 
pcl::registration::CorrespondenceRejectorFeatures::getTargetFeature (const std::string &key)
{
  typedef pcl::PointCloud<FeatureType> FeatureCloud;
  typedef typename FeatureCloud::ConstPtr FeatureCloudConstPtr;

  if (features_map_.count (key) == 0)
    return (boost::shared_ptr<const pcl::PointCloud<FeatureType> > ());
  else
    return (boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->getTargetFeature ());
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline void 
pcl::registration::CorrespondenceRejectorFeatures::setRadiusSearch (
    float r, const std::string &key, 
    const typename pcl::KdTree<FeatureType>::Ptr &tree)
{
  if (features_map_.count (key) == 0)
    features_map_[key].reset (new FeatureContainer<FeatureType>);
  boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->setRadiusSearch (tree, r);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline void 
pcl::registration::CorrespondenceRejectorFeatures::setKSearch (
    int k, const std::string &key, 
    const typename pcl::KdTree<FeatureType>::Ptr &tree)
{
  if (features_map_.count (key) == 0)
    features_map_[key].reset (new FeatureContainer<FeatureType>);
  boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->setKSearch (tree, k);
}

//////////////////////////////////////////////////////////////////////////////////////////////
inline bool
pcl::registration::CorrespondenceRejectorFeatures::hasValidFeatures ()
{
  if (features_map_.empty ())
    return (false);
  typename FeaturesMap::const_iterator feature_itr;
  for (feature_itr = features_map_.begin (); feature_itr != features_map_.end (); ++feature_itr)
    if (!feature_itr->second->isValid ())
      return (false);
  return (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename FeatureType> inline void 
pcl::registration::CorrespondenceRejectorFeatures::setFeatureRepresentation (
  const std::string &key, 
  const typename pcl::KdTree<FeatureType>::PointRepresentationConstPtr &fr)
{
  if (features_map_.count (key) == 0)
    features_map_[key].reset (new FeatureContainer<FeatureType>);
  boost::static_pointer_cast<FeatureContainer<FeatureType> > (features_map_[key])->setFeatureRepresentation (fr);
}


#endif /* PCL_REGISTRATION_IMPL_CORRESPONDENCE_REJECTION_FEATURES_HPP_ */
