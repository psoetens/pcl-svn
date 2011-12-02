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
 * $Id$
 */

#ifndef PCL_SEARCH_FLANN_SEARCH_H_
#define PCL_SEARCH_FLANN_SEARCH_H_

#include <pcl/search/search.h>
#include <pcl/point_representation.h>
#include <flann/flann.hpp>

namespace pcl
{
  namespace search
  {

    /** \brief @b search::FlannSearch is a generic FLANN wrapper class for the new search interface.
     * It is able to wrap any FLANN index type, e.g. the kd tree as well as indices for high-dimensional
     * searches and intended as a more powerful and cleaner successor to KdTreeFlann.
     * THIS CODE IS NOT DONE YET! IT CONTAINS KNOWN UNHANDLED FAILURE CASES, AND IS CURRENTLY TO BE
     * CONSIDERED WORK-IN-PROGRESS!
     *
     * \author Andreas Muetzel
     * \ingroup search
     */
    template<typename PointT>
    class FlannSearch: public Search<PointT>
    {
      typedef typename Search<PointT>::PointCloud PointCloud;
      typedef typename Search<PointT>::PointCloudConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<std::vector<int> > IndicesPtr;
      typedef boost::shared_ptr<const std::vector<int> > IndicesConstPtr;
      typedef flann::NNIndex< flann::L2<float> > Index;

      typedef pcl::PointRepresentation<PointT> PointRepresentation;
      //typedef boost::shared_ptr<PointRepresentation> PointRepresentationPtr;
      typedef boost::shared_ptr<const PointRepresentation> PointRepresentationConstPtr;

    public:
      typedef boost::shared_ptr<FlannSearch<PointT> > Ptr;
      typedef boost::shared_ptr<const FlannSearch<PointT> > ConstPtr;

      class FlannIndexCreator
      {
      public:
        virtual Index *createIndex( const flann::Matrix<float>& data )=0;
      };

      class KdTreeIndexCreator: public FlannIndexCreator
      {
      public:
        virtual Index *createIndex( const flann::Matrix<float>& data )
        {
          return new flann::KDTreeSingleIndex<flann::L2<float> >(data);
        }
      };

      FlannSearch ( FlannIndexCreator* creator ) : index_(0), creator_(creator), eps_(0), input_copied_for_flann_(false)
      {
        point_representation_.reset (new DefaultPointRepresentation<PointT>);
        dim_=point_representation_->getNumberOfDimensions();
      }

      /** \brief Destructor for KdTree. */
      virtual
      ~FlannSearch ()
      {
        delete creator_;
        delete index_;
      }

      //void
      //setInputCloud (const PointCloudConstPtr &cloud, const IndicesConstPtr &indices = IndicesConstPtr ());

      /** \brief Set the search epsilon precision (error bound) for nearest neighbors searches.
       * \param eps precision (error bound) for nearest neighbors searches
       */
      inline void
      setEpsilon (double eps)
      {
        eps_=eps;
      }

      /** \brief Get the search epsilon precision (error bound) for nearest neighbors searches. */
      inline double
      getEpsilon ()
      {
        return eps_;
      }

      /** \brief Provide a pointer to the input dataset.
       * \param cloud the const boost shared pointer to a PointCloud message
       * \param indices the point indices subset that is to be used from \a cloud
       */
      inline void
      setInputCloud (const PointCloudConstPtr& cloud, const IndicesConstPtr& indices)
      {
        input_=cloud;
        indices_=indices;
        delete index_;
        convertInputToFlannMatrix();
        index_=creator_->createIndex( input_flann_ );
        index_->buildIndex();
      }

      /** \brief Provide a pointer to the input dataset.
       * \param cloud the const boost shared pointer to a PointCloud message
       */
      inline void
      setInputCloud (const PointCloudConstPtr& cloud)
      {
        const IndicesConstPtr& indices = IndicesConstPtr ();
        setInputCloud (cloud, indices);
      }

      /** \brief Get a pointer to the input dataset as passed by the user. */
      PointCloudConstPtr
      getInputCloud ()
      {
        return input_;
      }

      /** \brief Get a pointer to the set of input indices used as passed by the user. */
      virtual IndicesConstPtr const
      getIndices ()
      {
        return indices_;
      }

      /** \brief Search for the k-nearest neighbors for the given query point.
       * \param[in] point the given query point
       * \param[in] k the number of neighbors to search for
       * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
       * \param[out] k_distances the resultant squared distances to the neighboring points (must be resized to \a k
       * a priori!)
       * \return number of neighbors found
       */
      int
      nearestKSearch (const PointT &point, int k, std::vector<int> &indices, std::vector<float> &dists)
      {
        bool can_cast = point_representation_->isTrivial();

        float* data=0;
        if( !can_cast )
        {
          data = new float[point_representation_->getNumberOfDimensions()];
          point_representation_->copyToFloatArray(point,data);
        }

        float* cdata = can_cast ? const_cast<float*>( reinterpret_cast<const float*>(&point) ): data;
        const flann::Matrix<float> m( cdata ,1, point_representation_->getNumberOfDimensions());

        flann::SearchParams p;
        p["eps"]=eps_;
        indices.resize(k,-1);
        dists.resize(k);
        flann::Matrix<int> i(&indices[0],1,k);
        flann::Matrix<float> d(&dists[0],1,k);
        int result = index_->knnSearch(m,i,d,k, p);

        delete [] data;

        if (!identity_mapping_)
        {
          for (size_t i = 0; i < (size_t)k; ++i)
          {
            int& neighbor_index = indices[i];
            neighbor_index = index_mapping_[neighbor_index];
          }
        }
        return result;
      }


      /** \brief Search for the k-nearest neighbors for the given query point.
        * \param[in] cloud the point cloud data
        * \param[in] index the index in \a cloud representing the query point
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points, k_indices[i] corresponds to the neighbors of the query point i
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points, k_sqr_distances[i] corresponds to the neighbors of the query point i
        */
      virtual void
      nearestKSearch (const PointCloud& cloud, const std::vector<int>& indices, int k, std::vector< std::vector<int> >& k_indices,
              std::vector< std::vector<float> >& k_sqr_distances)
      {
        if( indices.empty() ) // full point cloud + trivial copy operation = no need to do any conversion/copying to the flann matrix!
        {
          k_indices.resize( cloud.size() );
          k_sqr_distances.resize( cloud.size() );

          bool can_cast = point_representation_->isTrivial();

          float* data=0;
          if( !can_cast )
          {
            data = new float[dim_*cloud.size()];
            for( size_t i=0; i<cloud.size(); ++i )
              point_representation_->copyToFloatArray(cloud[i],data+i*dim_);
          }

          float* cdata = can_cast ? const_cast<float*>( reinterpret_cast<const float*>(&cloud[0]) ): data;
          const flann::Matrix<float> m( cdata ,cloud.size(), dim_, sizeof(PointT)/sizeof(float) );

          flann::SearchParams p;
          p["eps"]=eps_;
          index_->knnSearch(m,k_indices,k_sqr_distances,k, p);

          delete [] data;
        }
        else // if indices are present, the cloud has to be copied anyway. Only copy the relevant parts of the points here.
        {
          k_indices.resize( indices.size() );
          k_sqr_distances.resize( indices.size() );

          float* data=new float[dim_*indices.size()];
          for( size_t i=0; i<indices.size(); ++i )
            point_representation_->copyToFloatArray(cloud[indices[i]],data+i*dim_);
          const flann::Matrix<float> m( data ,cloud.size(), point_representation_->getNumberOfDimensions());

          flann::SearchParams p;
          p["eps"]=eps_;
          index_->knnSearch(m,k_indices,k_sqr_distances,k, p);

          delete[] data;
        }
        if (!identity_mapping_)
        {
          for( size_t j=0; j<k_indices.size(); ++j )
          {
            for (size_t i = 0; i < (size_t)k; ++i)
            {
              int& neighbor_index = k_indices[j][i];
              neighbor_index = index_mapping_[neighbor_index];
            }
          }
        }
      }

      /** \brief Search for the k-nearest neighbors for the given query point.
       * \param[in] cloud the point cloud data
       * \param[in] index the index in \a cloud representing the query point
       * \param[in] k the number of neighbors to search for
       * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
       * \param[out] k_distances the resultant squared distances to the neighboring points (must be resized to \a k
       * a priori!)
       * \return number of neighbors found
       */
      inline int
      nearestKSearch (const PointCloud &cloud, int index, int k, std::vector<int> &k_indices,
          std::vector<float> &k_distances)
      {
          return nearestKSearch( cloud[index],k,k_indices,k_distances );
      }

      /** \brief Search for the k-nearest neighbors for the given query point (zero-copy).
       *
       * \param[in] index the index representing the query point in the
       * dataset given by \a setInputCloud if indices were given in
       * setInputCloud, index will be the position in the indices vector
       * \param[in] k the number of neighbors to search for
       * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
       * \param[out] k_distances the resultant squared distances to the neighboring points (must be resized to \a k
       * a priori!)
       * \return number of neighbors found
       */
      inline int
      nearestKSearch (int index, int k, std::vector<int> &k_indices, std::vector<float> &k_distances)
      {
          return nearestKSearch( (*input_)[index],k,k_indices,k_distances );
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius.
       * \param[in] point the given query point
       * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
       * \param[out] k_indices the resultant indices of the neighboring points
       * \param[out] k_distances the resultant squared distances to the neighboring points
       * \param[in] max_nn if given, bounds the maximum returned neighbors to this value
       * \return number of neighbors found in radius
       */
      int
      radiusSearch (const PointT& point, double radius,
          std::vector<int> &indices, std::vector<float> &distances,
          int max_nn = -1) const
      {
        bool can_cast = point_representation_->isTrivial();

        float* data=0;
        if( !can_cast )
        {
          data = new float[point_representation_->getNumberOfDimensions()];
          point_representation_->copyToFloatArray(point,data);
        }

        float* cdata = can_cast ? const_cast<float*>( reinterpret_cast<const float*>(&point) ): data;
        const flann::Matrix<float> m( cdata ,1, point_representation_->getNumberOfDimensions());

        flann::SearchParams p;
        p["eps"]=eps_;
        p["max_neighbors"]=max_nn;
        std::vector<std::vector<int> > i(1);
        std::vector<std::vector<float> > d(1);
        int result = index_->radiusSearch(m,i,d,radius*radius, p);

        delete [] data;
        indices=i[0];
        distances=d[0];

        if (!identity_mapping_)
        {
          for (size_t i = 0; i < indices.size(); ++i)
          {
            int& neighbor_index = indices[i];
            neighbor_index = index_mapping_[neighbor_index];
          }
        }
        return result;
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius.
       * \param[in] cloud the point cloud data
       * \param[in] index the index in \a cloud representing the query point
       * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
       * \param[out] k_indices the resultant indices of the neighboring points
       * \param[out] k_distances the resultant squared distances to the neighboring points
       * \param[in] max_nn if given, bounds the maximum returned neighbors to this value
       * \return number of neighbors found in radius
       */
      inline int
      radiusSearch (const PointCloud& cloud, int index, double radius, std::vector<int> &k_indices,
          std::vector<float> &k_distances, int max_nn = -1)
      {
        return radiusSearch( cloud[index],radius,k_indices,k_distances,max_nn );
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius (zero-copy).
       * \param[in] index the index representing the query point in the dataset given by \a setInputCloud
       *        if indices were given in setInputCloud, index will be the position in the indices vector
       * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
       * \param[out] k_indices the resultant indices of the neighboring points
       * \param[out] k_distances the resultant squared distances to the neighboring points
       * \param[in] max_nn if given, bounds the maximum returned neighbors to this value
       * \return number of neighbors found in radius
       */
      inline int
      radiusSearch (int index, double radius, std::vector<int> &k_indices, std::vector<float> &k_distances,
          int max_nn = -1) const
      {
          return radiusSearch( (*input_)[index],radius,k_indices,k_distances,max_nn );
      }


      /** \brief Search for the k-nearest neighbors for the given query point.
        * \param[in] cloud the point cloud data
        * \param[in] index the index in \a cloud representing the query point
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points, k_indices[i] corresponds to the neighbors of the query point i
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points, k_sqr_distances[i] corresponds to the neighbors of the query point i
        */
      virtual void
      radiusSearch (const PointCloud& cloud, const std::vector<int>& indices, double radius, std::vector< std::vector<int> >& k_indices,
              std::vector< std::vector<float> >& k_sqr_distances, int max_nn=-1)
      {
        if( indices.empty() ) // full point cloud + trivial copy operation = no need to do any conversion/copying to the flann matrix!
        {
          k_indices.resize( cloud.size() );
          k_sqr_distances.resize( cloud.size() );

          bool can_cast = point_representation_->isTrivial();

          float* data=0;
          if( !can_cast )
          {
            data = new float[dim_*cloud.size()];
            for( size_t i=0; i<cloud.size(); ++i )
              point_representation_->copyToFloatArray(cloud[i],data+i*dim_);
          }

          float* cdata = can_cast ? const_cast<float*>( reinterpret_cast<const float*>(&cloud[0]) ): data;
          const flann::Matrix<float> m( cdata ,cloud.size(), dim_, sizeof(PointT)/sizeof(float) );

          flann::SearchParams p;
          p["eps"]=eps_;
          p["max_neighbors"]=max_nn;
          index_->radiusSearch(m,k_indices,k_sqr_distances,radius*radius, p);

          delete [] data;
        }
        else // if indices are present, the cloud has to be copied anyway. Only copy the relevant parts of the points here.
        {
          k_indices.resize( indices.size() );
          k_sqr_distances.resize( indices.size() );

          float* data=new float[dim_*indices.size()];
          for( size_t i=0; i<indices.size(); ++i )
            point_representation_->copyToFloatArray(cloud[indices[i]],data+i*dim_);
          const flann::Matrix<float> m( data ,cloud.size(), point_representation_->getNumberOfDimensions());

          flann::SearchParams p;
          p["eps"]=eps_;
          p["max_neighbors"]=max_nn;
          index_->radiusSearch(m,k_indices,k_sqr_distances,radius*radius, p);

          delete[] data;
        }
        if (!identity_mapping_)
        {
          for( size_t j=0; j<k_indices.size(); ++j )
          {
            for (size_t i = 0; i < k_indices[j].size(); ++i)
            {
              int& neighbor_index = k_indices[j][i];
              neighbor_index = index_mapping_[neighbor_index];
            }
          }
        }
      }

      /** \brief Provide a pointer to the point representation to use to convert points into k-D vectors.
        * \param[in] point_representation the const boost shared pointer to a PointRepresentation
        */
      inline void
      setPointRepresentation (const PointRepresentationConstPtr &point_representation)
      {
        point_representation_ = point_representation;
        setInputCloud (input_, indices_);  // Makes sense in derived classes to reinitialize the tree
      }

      /** \brief Get a pointer to the point representation used when converting points into k-D vectors. */
      inline PointRepresentationConstPtr const
      getPointRepresentation ()
      {
        return (point_representation_);
      }

    protected:

      void convertInputToFlannMatrix()
      {
        int original_no_of_points = indices_&&!indices_->empty() ? indices_->size():input_->size();



        if( input_copied_for_flann_ )
          delete input_flann_.data;
        input_copied_for_flann_=true;
        identity_mapping_=true;

        input_flann_=flann::Matrix<float>(new float[original_no_of_points*point_representation_->getNumberOfDimensions()], original_no_of_points, point_representation_->getNumberOfDimensions() );

        //cloud_ = (float*)malloc (original_no_of_points * dim_ * sizeof (float));
        float* cloud_ptr = input_flann_.data;
        //index_mapping_.reserve(original_no_of_points);
        //identity_mapping_ = true;

        if( !indices_ || indices_->empty()  )
        {
          /// TODO: implement "no NaN check" option that just re-uses the cloud data in the flann matrix
          for (int i = 0; i < original_no_of_points; ++i)
          {
            const PointT& point = (*input_)[i];
            // Check if the point is invalid
            if (!point_representation_->isValid(point)) {
              identity_mapping_ = false;
              continue;
            }

            index_mapping_.push_back(i);  // If the returned index should be for the indices vector

            point_representation_->vectorize(point, cloud_ptr);
            cloud_ptr += dim_;
          }

        }
        else
        {
          for (int indices_index = 0; indices_index < original_no_of_points; ++indices_index)
          {
            int cloud_index = (*indices_)[indices_index];
            const PointT&  point = (*input_)[cloud_index];
            // Check if the point is invalid
            if (!point_representation_->isValid(point)) {
              identity_mapping_ = false;
              continue;
            }

            index_mapping_.push_back(indices_index);  // If the returned index should be for the indices vector

            point_representation_->vectorize(point, cloud_ptr);
            cloud_ptr += dim_;
          }
        }
      }



      Index *index_;
      FlannIndexCreator *creator_;
      flann::Matrix<float> input_flann_;
      float eps_;
      bool input_copied_for_flann_;

      PointRepresentationConstPtr point_representation_;

      int dim_;

      PointCloudConstPtr input_;
      IndicesConstPtr indices_;
      std::vector<int> index_mapping_;
      bool identity_mapping_;

    };
  }
}

#define PCL_INSTANTIATE_FlannSearch(T) template class PCL_EXPORTS pcl::search::FlannSearch<T>;

#endif    // PCL_SEARCH_KDTREE_H_

