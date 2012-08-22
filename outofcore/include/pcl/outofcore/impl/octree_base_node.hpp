/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2012, Willow Garage, Inc.
 *  Copyright (c) 2012, Urban Robotics, Inc.
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
 *  $Id$
 *
 */

#ifndef PCL_OUTOFCORE_OCTREE_BASE_NODE_IMPL_H_
#define PCL_OUTOFCORE_OCTREE_BASE_NODE_IMPL_H_

// C++
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>

// PCL (Urban Robotics)
#include <pcl/outofcore/octree_base_node.h>

#include <pcl/filters/random_sample.h>
#include <pcl/filters/extract_indices.h>

// JSON
#include <pcl/outofcore/cJSON.h>

namespace pcl
{
  namespace outofcore
  {

    template<typename ContainerT, typename PointT>
    const std::string octree_base_node<ContainerT, PointT>::node_index_basename = "node";

    template<typename ContainerT, typename PointT>
    const std::string octree_base_node<ContainerT, PointT>::node_container_basename = "node";

    template<typename ContainerT, typename PointT>
    const std::string octree_base_node<ContainerT, PointT>::node_index_extension = ".oct_idx";

    template<typename ContainerT, typename PointT>
    const std::string octree_base_node<ContainerT, PointT>::node_container_extension = ".oct_dat";

    template<typename ContainerT, typename PointT>
    boost::mutex octree_base_node<ContainerT, PointT>::rng_mutex_;

    template<typename ContainerT, typename PointT>
    boost::mt19937 octree_base_node<ContainerT, PointT>::rand_gen_;

    template<typename ContainerT, typename PointT>
    const double octree_base_node<ContainerT, PointT>::sample_precent = .125;

    template<typename ContainerT, typename PointT>
    const std::string octree_base_node<ContainerT, PointT>::pcd_extension = ".pcd";

    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::octree_base_node ()
      : parent_ (NULL),
        root_ (NULL),
        depth_ (0),
        num_child_ (0),
        node_metadata_ ()
    {
      node_metadata_ = boost::shared_ptr<OutofcoreOctreeNodeMetadata> (new OutofcoreOctreeNodeMetadata ());
      node_metadata_->setOutofcoreVersion (3);
      memset (children_, 0, 8 * sizeof(octree_base_node<ContainerT, PointT>*));
    }

////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::octree_base_node (const boost::filesystem::path& directory_path, octree_base_node<ContainerT, PointT>* super, bool load_all)
      : m_tree_ ()
      , root_ ()
      , parent_ ()
      , depth_ ()
      , children_ ()
      , num_child_ ()
      , payload_ ()
      , node_metadata_ ()
    {
      node_metadata_ = boost::shared_ptr<OutofcoreOctreeNodeMetadata> (new OutofcoreOctreeNodeMetadata ());
      node_metadata_->setOutofcoreVersion (3);

      //Check if this is the first node created (this is true if super, i.e. node's parent is NULL)
      if (super == NULL)
      {
        node_metadata_->setDirectoryPathname (directory_path.parent_path ());
        
//        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Case not implemented\n");

        node_metadata_->setMetadataFilename (directory_path);

        //Check if the specified directly currently exists, and if not, don't continue
        if (!boost::filesystem::exists (node_metadata_->getDirectoryPathname ()))
        {
          PCL_ERROR ("[pcl::outofcore::octree_base_node] Could not find dir %s\n", node_metadata_->getDirectoryPathname ().c_str ());
          PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Exception: missing directory");
        }
        
        depth_ = 0;
        root_ = this;
      }
      else
      {
        node_metadata_->setDirectoryPathname (directory_path);

        depth_ = super->depth_ + 1;
        root_ = super->root_;

        //empty constructor creates end of iterator
        boost::filesystem::directory_iterator directory_it_end;

        int loaded = 0;

        for (boost::filesystem::directory_iterator directory_it (node_metadata_->getDirectoryPathname ()); directory_it != directory_it_end; ++directory_it)
        {
          const boost::filesystem::path& file = *directory_it;

          if (!boost::filesystem::is_directory (file))
          {
            if (boost::filesystem::extension (file) == node_index_extension)
            {
//              PCL_INFO ("[pcl:outofcore::octree_base_node] Loading metadata from %s\n", file.c_str ());
              loaded = node_metadata_->loadMetadataFromDisk (file);
              break;
            }
          }

        }

        if (!loaded)
        {
          PCL_ERROR ("[pcl::outofcore::octree_base_node] Could not find index\n");
          PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore: Could not find node index");
        }

      }
      

      loadFromFile (node_metadata_->getMetadataFilename (), super);

      if (load_all)
      {
        loadChildren (true);
      }
    }
//////////////////////////////////////////////////////////////////////////////// 

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::init_root_node (const Eigen::Vector3d& bb_min, const Eigen::Vector3d& bb_max, octree_base<ContainerT, PointT> * const tree, const boost::filesystem::path& root_name)
    {
      parent_ = NULL;
      root_ = this;
      m_tree_ = tree;
      depth_ = 0;

      // Allocate space for 8 child nodes
      memset (children_, 0, 8 * sizeof(octree_base_node<ContainerT, PointT>*));
      num_child_ = 0;

      //?? Should this be done in octree_base ??
      // Make the bounding box square based on the largest axis
      Eigen::Vector3d tmp_max = bb_max;
      Eigen::Vector3d tmp_min = bb_min;
      enlargeToCube (tmp_min, tmp_max);

      // Need to make the bounding box slightly bigger so points that fall on the max side aren't excluded
      double epsilon = 1e-8;
      tmp_max = tmp_max + epsilon*Eigen::Vector3d (1.0, 1.0, 1.0);

      node_metadata_->setBoundingBox (tmp_min, tmp_max);
      node_metadata_->setDirectoryPathname (root_name.parent_path ());
      node_metadata_->setOutofcoreVersion (3);

      // If the root directory doesn't exist create it
      if (!boost::filesystem::exists (node_metadata_->getDirectoryPathname ()))
      {
        boost::filesystem::create_directory (node_metadata_->getDirectoryPathname ());
      }
      // If the root directory is a file
      else if (!boost::filesystem::is_directory (node_metadata_->getDirectoryPathname ()))
      {
        PCL_ERROR ("[pcl::outofcore::octree_base_node] Need empty directory structure. Dir %s exists and is a file.\n",node_metadata_->getDirectoryPathname ().c_str ());
        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Bad Path: Directory Already Exists");
      }

      // Create a unique id for node file name
      /** \todo: getRandomUUIDString shouldn't be in class octree_disk_container; also this is pretty slow*/
      std::string uuid;
      
      octree_disk_container<PointT>::getRandomUUIDString (uuid);

      std::string node_container_name;

      node_container_name = uuid + std::string ("_") + node_container_basename + pcd_extension;

      node_metadata_->setMetadataFilename (node_metadata_->getDirectoryPathname () / root_name.filename ());
      node_metadata_->setPCDFilename (node_metadata_->getDirectoryPathname () / boost::filesystem::path (node_container_name));

      boost::filesystem::create_directory (node_metadata_->getDirectoryPathname ());
      node_metadata_->serializeMetadataToDisk ();

      // Create data container, ie octree_disk_container, octree_ram_container
      payload_ = boost::shared_ptr<ContainerT> (new ContainerT (node_metadata_->getPCDFilename ()));
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::saveIdx (bool recursive)
    {
      node_metadata_->serializeMetadataToDisk ();

      if (recursive)
      {
        for (size_t i = 0; i < 8; i++)
        {
          if (children_[i])
            children_[i]->saveIdx (true);
        }
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::octree_base_node (const Eigen::Vector3d& bb_min, const Eigen::Vector3d& bb_max, const double node_dim_meters, octree_base<ContainerT, PointT> * const tree, const boost::filesystem::path& root_name)
      : m_tree_ ()
      , root_ ()
      , parent_ ()
      , depth_ ()
      , children_ ()
      , num_child_ ()
      , payload_ ()
      , node_metadata_ ()
    {
      node_metadata_ = boost::shared_ptr<OutofcoreOctreeNodeMetadata> (new OutofcoreOctreeNodeMetadata ());
      node_metadata_->setOutofcoreVersion (3);
      
      init_root_node (bb_min, bb_max, tree, root_name);

      // Calculate the max depth but don't create nodes
      tree->max_depth_ = calcDepthForDim (bb_min, bb_max, node_dim_meters);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::octree_base_node (const int max_depth, const Eigen::Vector3d& bb_min, const Eigen::Vector3d& bb_max, octree_base<ContainerT, PointT> * const tree, const boost::filesystem::path& root_name)
      : m_tree_ ()
      , root_ ()
      , parent_ ()
      , depth_ ()
      , children_ ()
      , num_child_ ()
      , payload_ ()
      , node_metadata_ ()
    {
      node_metadata_ = boost::shared_ptr<OutofcoreOctreeNodeMetadata> (new OutofcoreOctreeNodeMetadata ());
      node_metadata_->setOutofcoreVersion (3);

      init_root_node(bb_min, bb_max, tree, root_name);

      // Set max depth but don't create nodes
      tree->max_depth_ = max_depth;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::~octree_base_node ()
    {
      // Recursively delete all children and this nodes data
      recFreeChildren ();
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> inline bool
    octree_base_node<ContainerT, PointT>::hasUnloadedChildren () const
    {
      unsigned int num_child_dirs = 0;
      // Check nodes directory for children directories 0-7
      for (int i = 0; i < 8; i++)
      {
        boost::filesystem::path child_dir = node_metadata_->getDirectoryPathname () / boost::filesystem::path (boost::lexical_cast<std::string> (i));
        if (boost::filesystem::exists (child_dir))
        {
          num_child_dirs++;
        }
      }

      // If found directories is less than the current nodes loaded children
      if (num_child_dirs > num_child_)
        return (true);

      return (false);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::loadChildren (bool recursive)
    {
      /** todo: hasChildrenLoaded checks \ref num_child_ against how many child
          directories live on disk.  This just bails if anything is loaded? */
      if (num_child_ != 0)
      {
        PCL_ERROR ("[pcl::outofcore::octree_base_node] Calling loadChildren on a node that already has loaded children! - skipping\n");
        return;
      }

      // Create a new node for each child directory that exists
      for (int i = 0; i < 8; i++)
      {
        boost::filesystem::path child_dir = node_metadata_->getDirectoryPathname () / boost::filesystem::path (boost::lexical_cast<std::string> (i));
        if (boost::filesystem::exists (child_dir))
        {
          this->children_[i] = new octree_base_node<ContainerT, PointT> (child_dir, this, recursive);
          num_child_++;
        }
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::recFreeChildren ()
    {
      if (num_child_ == 0)
      {
        return;
      }

      for (size_t i = 0; i < 8; i++)
      {
        if (children_[i])
        {
          octree_base_node<ContainerT, PointT>* current = children_[i];
          delete current;
        }
      }
      memset (children_, 0, 8 * sizeof(octree_base_node<ContainerT, PointT>*));
      num_child_ = 0;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> uint64_t
    octree_base_node<ContainerT, PointT>::addDataToLeaf (const AlignedPointTVector& p, const bool skip_bb_check)
    {
      ///\todo consider using fr/p 02 locational codes

      //quit if there are no points to add
      if (p.empty ())
      {
        return (0);
      }

      //if this depth is the max depth of the tree, then add the points
      if (this->depth_ == root_->m_tree_->max_depth_)
        return (addDataAtMaxDepth( p, skip_bb_check));

      if (num_child_ < 8)
        if (hasUnloadedChildren ())
          loadChildren (false);

      std::vector < std::vector<const PointT*> > c;
      c.resize (8);
      for (size_t i = 0; i < 8; i++)
      {
        c[i].reserve (p.size () / 8);
      }

      const size_t len = p.size ();
      for (size_t i = 0; i < len; i++)
      {
        const PointT& pt = p[i];

        if (!skip_bb_check)
        {
          if (!this->pointInBoundingBox (pt))
          {
            PCL_ERROR ( "[pcl::outofcore::octree_base_node::%s] Failed to place point within bounding box\n", __FUNCTION__ );
            continue;
          }
        }

        uint8_t box = 0;
        Eigen::Vector3d mid_xyz = node_metadata_->getVoxelCenter ();
        
        box = static_cast<uint8_t>(((pt.z >= mid_xyz[2]) << 2) | ((pt.y >= mid_xyz[1]) << 1) | ((pt.x >= mid_xyz[0]) << 0));
        c[static_cast<size_t>(box)].push_back (&pt);
      }
      
      boost::uint64_t points_added = 0;
      for (int i = 0; i < 8; i++)
      {
        if (c[i].empty ())
          continue;
        if (!children_[i])
          createChild (i);
        points_added += children_[i]->addDataToLeaf (c[i], true);
        c[i].clear ();
      }
      return (points_added);
    }
////////////////////////////////////////////////////////////////////////////////


    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addDataToLeaf (const std::vector<const PointT*>& p, const bool skip_bb_check)
    {
      ///\todo deprecate this method
      
      if (p.empty ())
      {
        return (0);
      }

      if (this->depth_ == root_->m_tree_->max_depth_)
      {
        if (skip_bb_check)//trust me, just add the points
        {
          root_->m_tree_->incrementPointsInLOD (this->depth_, p.size ());
          
          payload_->insertRange (p.data (), p.size ());
          
          return (p.size ());
        }
        else//check which points belong to this node, throw away the rest
        {
          std::vector<const PointT*> buff;
          BOOST_FOREACH(const PointT* pt, p)
          {
            if(pointInBoundingBox(*pt))
            {
              buff.push_back(pt);
            }
          }

          if (!buff.empty ())
          {
            root_->m_tree_->incrementPointsInLOD (this->depth_, buff.size ());
            payload_->insertRange (buff.data (), buff.size ());
//            payload_->insertRange ( buff );
            
          }
          return (buff.size ());
        }
      }
      else
      {
        if (num_child_ < 8)
        {
          if (hasUnloadedChildren ())
          {
            loadChildren (false);
          }
        }

        std::vector < std::vector<const PointT*> > c;
        c.resize (8);
        for (int i = 0; i < 8; i++)
        {
          c[i].reserve (p.size () / 8);
        }

        const size_t len = p.size ();
        for (size_t i = 0; i < len; i++)
        {
          //const PointT& pt = p[i];
          if (!skip_bb_check)
          {
            if (!this->pointInBoundingBox (*p[i]))
            {
              //	std::cerr << "failed to place point!!!" << std::endl;
              continue;
            }
          }

          uint8_t box = 00;
          Eigen::Vector3d mid_xyz = node_metadata_->getVoxelCenter ();
          //hash each coordinate to the appropriate octant
          box = static_cast<uint8_t> (((p[i]->z >= mid_xyz[2]) << 2) | ((p[i]->y >= mid_xyz[1]) << 1) | ((p[i]->x >= mid_xyz[0] )));
          //3 bit, 8 octants
          c[box].push_back (p[i]);
        }
        
        boost::uint64_t points_added = 0;
        for (int i = 0; i < 8; i++)
        {
          if (c[i].empty ())
            continue;
          if (!children_[i])
            createChild (i);
          points_added += children_[i]->addDataToLeaf (c[i], true);
          c[i].clear ();
        }
        return (points_added);
      }
      // std::cerr << "failed to place point!!!" << std::endl;
      return (0);
    }
////////////////////////////////////////////////////////////////////////////////


    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addPointCloud (const sensor_msgs::PointCloud2::Ptr& input_cloud, const bool skip_bb_check = false)
    {
      
      if (input_cloud->height*input_cloud->width == 0)
        return (0);
      
      if (this->depth_ == root_->m_tree_->max_depth_)
        return (addDataAtMaxDepth (input_cloud, true));
      
      if( num_child_ < 8 )
        if(hasUnloadedChildren ())
          loadChildren (false);

      if( skip_bb_check == false )
      {

        //indices to store the points for each bin
        //these lists will be used to copy data to new point clouds and pass down recursively
        std::vector < std::vector<int> > indices;
        indices.resize (8);
        
        this->sortOctantIndices (input_cloud, indices, node_metadata_->getVoxelCenter ());

        for(int k=0; k<indices.size (); k++)
        {
//          PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Computed %d indices in octact %d\n", __FUNCTION__, indices[k].size (), k);
        }

        boost::uint64_t points_added = 0;

        for(int i=0; i<8; i++)
        {
          if ( indices[i].empty () )
            continue;

          if ( children_[i] == false )
          {
            createChild (i);
          }

          sensor_msgs::PointCloud2::Ptr dst_cloud (new sensor_msgs::PointCloud2 () );

//              PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Extracting indices to bins\n", __FUNCTION__);
              
          //copy the points from extracted indices from input cloud to destination cloud
          pcl::copyPointCloud ( *input_cloud, indices[i], *dst_cloud ) ;
          
          //recursively add the new cloud to the data
          points_added += children_[i]->addPointCloud ( dst_cloud );
          indices[i].clear ();
        }
        
        return (points_added);
      }
      
      PCL_ERROR ("[pcl::outofcore::octree_base_node] Skipped bounding box check. Points not inserted\n");
      
      return 0;
    }


////////////////////////////////////////////////////////////////////////////////
/** todo: Need to refactor this further as to not pass in a BBCheck */
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::randomSample(const AlignedPointTVector& p, AlignedPointTVector& insertBuff, const bool skip_bb_check)
    {
      AlignedPointTVector sampleBuff;
      if (!skip_bb_check)
      {
        BOOST_FOREACH (const PointT& pt, p)
        if(pointInBoundingBox(pt))
          sampleBuff.push_back(pt);
      }
      else
      {
        sampleBuff = p;
      }

      // Derive percentage from specified sample_precent and tree depth
      const double percent = pow(sample_precent, double((root_->m_tree_->max_depth_ - depth_)));
      const uint64_t samplesize = static_cast<uint64_t>(percent * static_cast<double>(sampleBuff.size()));
      const uint64_t inputsize = sampleBuff.size();

      if(samplesize > 0)
      {
        // Resize buffer to sample size
        insertBuff.resize(samplesize);

        // Create random number generator
        boost::mutex::scoped_lock lock(rng_mutex_);
        boost::uniform_int<boost::uint64_t> buffdist(0, inputsize-1);
        boost::variate_generator<boost::mt19937&, boost::uniform_int<boost::uint64_t> > buffdie(rand_gen_, buffdist);

        // Randomly pick sampled points
        for(boost::uint64_t i = 0; i < samplesize; ++i)
        {
          boost::uint64_t buffstart = buffdie();
          insertBuff[i] = ( sampleBuff[buffstart] );
        }
      }
      // Have to do it the slow way
      else
      {
        boost::mutex::scoped_lock lock(rng_mutex_);
        boost::bernoulli_distribution<double> buffdist(percent);
        boost::variate_generator<boost::mt19937&, boost::bernoulli_distribution<double> > buffcoin(rand_gen_, buffdist);

        for(boost::uint64_t i = 0; i < inputsize; ++i)
          if(buffcoin())
            insertBuff.push_back( p[i] );
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addDataAtMaxDepth (const AlignedPointTVector& p, const bool skip_bb_check)
    {
      // Trust me, just add the points
      if (skip_bb_check)
      {
        // Increment point count for node
        root_->m_tree_->incrementPointsInLOD (this->depth_, p.size ());

        // Insert point data
        payload_->insertRange ( p );
        
        ///\todo this is not a failsafe way to know all the points were written (!!!!)
        return (p.size ());
      }

      // Add points found within the current node's bounding box
      /// \todo standardize the boundary case
      else
      {
        AlignedPointTVector buff;
        const size_t len = p.size ();

        for (size_t i = 0; i < len; i++)
        {
          if (pointInBoundingBox (p[i]))
          {
            buff.push_back (p[i]);
          }
        }
        
        if (!buff.empty ())
        {
          root_->m_tree_->incrementPointsInLOD (this->depth_, buff.size ());
          payload_->insertRange ( buff );
          
        }
        return (buff.size ());
      }
    }
////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addDataAtMaxDepth ( const sensor_msgs::PointCloud2::Ptr input_cloud, const bool skip_bb_check = true )
    {
      //this assumes data is already in the correct bin
      if(skip_bb_check == true)
      {
//            PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Adding %u points at max depth, %u\n",__FUNCTION__, input_cloud->width*input_cloud->height, this->depth_);
            
        this->root_->m_tree_->incrementPointsInLOD (this->depth_, input_cloud->width * input_cloud->height );
        payload_->insertRange (input_cloud);            
        return (input_cloud->width * input_cloud->height);
      }
      else
      {
        PCL_ERROR ("[pcl::outofcore::octree_base_node] Not implemented\n");
        return (0);
      }
    }


////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::subdividePoints (const AlignedPointTVector &p, std::vector< AlignedPointTVector > &c, const bool skip_bb_check)
    {
      // Reserve space for children nodes
      c.resize(8);
      for(int i = 0; i < 8; i++)
        c[i].reserve(p.size() / 8);

      const size_t len = p.size();
      for(size_t i = 0; i < len; i++)
      {
        const PointT& pt = p[i];

        if(!skip_bb_check)
          if(!this->pointInBoundingBox(pt))
            continue;

        subdividePoint (pt, c);
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::subdividePoint (const PointT& point, std::vector< AlignedPointTVector >& c)
    {
      Eigen::Vector3d mid_xyz = node_metadata_->getVoxelCenter ();
      size_t octant = 0;
      octant = ((point.z >= mid_xyz[2]) << 2) | ((point.y >= mid_xyz[1]) << 1) | ((point.x >= mid_xyz[0]) << 0);
      c[octant].push_back (point);
    }

////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addPointCloud_and_genLOD (const sensor_msgs::PointCloud2::Ptr input_cloud) //, const bool skip_bb_check = false )
    {
      /// \todo reduce number of copies in @addPointCloud_and_genLOD@ here by keeping the cloud on the heap in the first place
      boost::uint64_t points_added = 0;
      
      if ( input_cloud->width * input_cloud->height == 0 )
      {
        return (0);
      }
      
      if ( this->depth_ == root_->m_tree_->max_depth_ || input_cloud->width*input_cloud->height < 8 )
      {
        uint64_t points_added = addDataAtMaxDepth (input_cloud);
        assert (points_added > 0);
        return (points_added);        
      }
      
      if (num_child_ < 8 )
      {
        if ( hasUnloadedChildren () )
        {
          loadChildren (false);
        }
      }

      //------------------------------------------------------------
      //subsample data:
      //   1. Get indices from a random sample
      //   2. Extract those indices with the extract indices class (in order to also get the complement)
      //------------------------------------------------------------
      pcl::RandomSample<sensor_msgs::PointCloud2> random_sampler;
      random_sampler.setInputCloud ( input_cloud );

      //set sample size to 1/8 of total points (12.5%)
      uint64_t sample_size = input_cloud->width*input_cloud->height / 8;
      random_sampler.setSample (static_cast<unsigned int> (sample_size));
      
      //create our destination
      sensor_msgs::PointCloud2::Ptr downsampled_cloud ( new sensor_msgs::PointCloud2 () );
      //create destination for indices
      pcl::IndicesPtr downsampled_cloud_indices ( new std::vector< int > () );
      random_sampler.filter (*downsampled_cloud_indices);
      //extract the "random subset", size by setSampleSize
      pcl::ExtractIndices<sensor_msgs::PointCloud2> extractor;
      extractor.setInputCloud ( input_cloud );
      extractor.setIndices ( downsampled_cloud_indices );
      extractor.filter ( *downsampled_cloud );
      //extract the complement of those points (i.e. everything remaining)
      sensor_msgs::PointCloud2::Ptr remaining_points ( new sensor_msgs::PointCloud2 () );
      extractor.setNegative (true);
      extractor.filter ( *remaining_points );

//      PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Random sampled: %lu of %lu\n", __FUNCTION__, downsampled_cloud->width * downsampled_cloud->height, input_cloud->width * input_cloud->height );
      
      //insert subsampled data to the node's disk container payload
      if ( downsampled_cloud->width * downsampled_cloud->height != 0 )
      {
        root_->m_tree_->incrementPointsInLOD ( this->depth_, downsampled_cloud->width * downsampled_cloud->height );
        payload_->insertRange (downsampled_cloud);
        points_added += downsampled_cloud->width*downsampled_cloud->height ;
      }

//      PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Remaining points are %u\n",__FUNCTION__, remaining_points->width*remaining_points->height);

      //subdivide remaining data by destination octant
      std::vector<std::vector<int> > indices;
      indices.resize (8);

      this->sortOctantIndices (remaining_points, indices, node_metadata_->getVoxelCenter ());

      //pass each set of points to the appropriate child octant
      for(int i=0; i<8; i++)
      {

        if(indices[i].empty ())
          continue;

        if( children_[i] == false )
          createChild (i);

        //copy correct indices into a temporary cloud
        sensor_msgs::PointCloud2::Ptr tmp_local_point_cloud ( new sensor_msgs::PointCloud2 () );
        pcl::copyPointCloud ( *remaining_points, indices[i], *tmp_local_point_cloud );

        //recursively add points and keep track of how many were successfully added to the tree
        points_added += children_[i]->addPointCloud_and_genLOD ( tmp_local_point_cloud );
//        PCL_INFO ("[pcl::outofcore::octree_base_node::%s] points_added: %lu, indices[i].size: %lu, tmp_local_point_cloud size: %lu\n", __FUNCTION__, points_added, indices[i].size (), tmp_local_point_cloud->width*tmp_local_point_cloud->height);

      }
      assert (points_added == input_cloud->width*input_cloud->height);
      return (points_added);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> boost::uint64_t
    octree_base_node<ContainerT, PointT>::addDataToLeaf_and_genLOD (const AlignedPointTVector& p, const bool skip_bb_check)
    {
      // If there's no points return
      if (p.empty ())
        return (0);

      /// \todo: Why is skip_bb_check set to false when adding points at max depth
      //  when adding data and generating sampled LOD 
      // If the max depth has been reached
      if (this->depth_ == root_->m_tree_->max_depth_)
        return (addDataAtMaxDepth(p, false));

      // Create child nodes of the current node but not grand children+
      if (num_child_ < 8)
        if (hasUnloadedChildren ())
          loadChildren (false);

      // Randomly sample data
      AlignedPointTVector insertBuff;
      randomSample(p, insertBuff, skip_bb_check);

      if(!insertBuff.empty())
      {
        // Increment point count for node
        root_->m_tree_->incrementPointsInLOD (this->depth_, insertBuff.size());
        // Insert sampled point data
//        payload_->insertRange ( &(insertBuff.front ()), insertBuff.size());
        payload_->insertRange (insertBuff);
        
      }

      //subdivide vec to pass data down lower
      std::vector< AlignedPointTVector > c;
      subdividePoints(p, c, skip_bb_check);

      /// \todo: Perhaps do a quick loop through the lists here and dealloc the reserved mem for empty lists
      boost::uint64_t points_added = 0;
      for(int i = 0; i < 8; i++)
      {
        // If child doesn't have points
        if(c[i].empty())
          continue;

        // If child doesn't exist
        if(!children_[i])
          createChild(i);

        /// \todo: Why are there no bounding box checks on the way down?
        // Recursively build children
        points_added += children_[i]->addDataToLeaf_and_genLOD(c[i], true);
        c[i].clear();
      }

      return (points_added);

      // todo: Make sure I didn't break anything by removing the if/else above
      // std::cerr << "failed to place point!!!" << std::endl;
      //return 0;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::createChild (const int idx)
    {
      //if already has 8 children, return
      if (children_[idx] || (num_child_ == 8))
        return;

      Eigen::Vector3d start = node_metadata_->getBoundingBoxMin ();

      Eigen::Vector3d step = (node_metadata_->getBoundingBoxMax () - start)/static_cast<double>(2.0);

      Eigen::Vector3d childbb_min;
      Eigen::Vector3d childbb_max;

      int x, y, z;
      if (idx > 3)
      {
        x = ((idx == 5) || (idx == 7)) ? 1 : 0;
        y = ((idx == 6) || (idx == 7)) ? 1 : 0;
        z = 1;
      }
      else
      {
        x = ((idx == 1) || (idx == 3)) ? 1 : 0;
        y = ((idx == 2) || (idx == 3)) ? 1 : 0;
        z = 0;
      }

      childbb_min[2] = start[2] + static_cast<double> (z) * step[2];
      childbb_max[2] = start[2] + static_cast<double> (z + 1) * step[2];

      childbb_min[1] = start[1] + static_cast<double> (y) * step[1];
      childbb_max[1] = start[1] + static_cast<double> (y + 1) * step[1];

      childbb_min[0] = start[0] + static_cast<double> (x) * step[0];
      childbb_max[0] = start[0] + static_cast<double> (x + 1) * step[0];

      boost::filesystem::path childdir = node_metadata_->getDirectoryPathname () / boost::filesystem::path (boost::lexical_cast<std::string> (idx));
      children_[idx] = new octree_base_node<ContainerT, PointT> (childbb_min, childbb_max, childdir.string ().c_str (), this);

      num_child_++;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> int
    octree_base_node<ContainerT, PointT>::calcDepthForDim (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, const double dim)
    {
      double volume = 1;
      double diagonal = 0;

      for (int i = 0; i < 3; i++)
      {
        double side = max_bb[i] - min_bb[i];
        diagonal += side * side;
        volume *= side;
      };

      diagonal = sqrt (diagonal);
      double dim_volume = dim * dim * dim;

      if ((diagonal <= dim) || (volume <= dim_volume))
      {
        return (0);
      }
      else
      {
        Eigen::Vector3d start = min_bb;
        Eigen::Vector3d step = (max_bb - min_bb) / static_cast<double> (2.0);
        
        Eigen::Vector3d childbb_min = start;
        Eigen::Vector3d childbb_max = start + step;

        return (1 + calcDepthForDim (childbb_min, childbb_max, dim));
      }

    }
////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> bool
    pointInBoundingBox (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, const Eigen::Vector3d& point)
    {
      // won't <= lead to points being added to more than one voxel?
      if (((min_bb[0] <= point[0]) && (point[0] < max_bb[0])) &&
          ((min_bb[1] <= point[1]) && (point[1] < max_bb[1])) &&
          ((min_bb[2] <= point[2]) && (point[2] < max_bb[2])))
      {
        return (true);
    
      }
      return (false);
    }
    


////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> inline bool
    octree_base_node<ContainerT, PointT>::pointInBoundingBox (const PointT& p) const
    {
      Eigen::Vector3d min; 
      Eigen::Vector3d max;

      node_metadata_->getBoundingBox (min, max);
      
      // won't <= lead to points being added to more than one voxel?
      if (((min[0] <= p.x) && (p.x < max[0])) &&
          ((min[1] <= p.y) && (p.y < max[1])) &&
          ((min[2] <= p.z) && (p.z < max[2])))
      {
        return (true);
    
      }
      return (false);
    }

////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::printBoundingBox (const size_t query_depth) const
    {
      Eigen::Vector3d min;
      Eigen::Vector3d max;
      node_metadata_->getBoundingBox (min, max);

      ///\todo move \ref printBoundingBox to node_metadata_ file
      if (this->depth_ < query_depth){
        for (size_t i = 0; i < this->depth_; i++)
          std::cout << "  ";

        std::cout << "[" << min[0] << ", " << min[1] << ", " << min[2] << "] - ";
        std::cout << "[" << max[0] << ", " << max[1] << ", " << max[2] << "] - ";
        std::cout <<  "[" << max[0] - min[0] << ", " << max[1] - min[1];
        std::cout << ", " << max[2] - min[2] << "]" << std::endl;

        if (num_child_ > 0)
        {
          for (size_t i = 0; i < 8; i++)
          {
            if (children_[i])
              children_[i]->printBoundingBox (query_depth);
          }
        }
      }
    }

////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::getVoxelCentersRecursive (AlignedPointTVector &voxel_centers, const size_t query_depth)
    {
      if (this->depth_ < query_depth){
        if (num_child_ > 0)
        {
          for (size_t i = 0; i < 8; i++)
          {
            if (children_[i])
              children_[i]->getVoxelCenters (voxel_centers, query_depth);
          }
        }
      }
      else
      {
        PointT voxel_center;
        Eigen::Vector3d mid_xyz = node_metadata_->getVoxelCenter ();
        voxel_center.x = static_cast<float>(mid_xyz[0]);
        voxel_center.y = static_cast<float>(mid_xyz[1]);
        voxel_center.z = static_cast<float>(mid_xyz[2]);

        voxel_centers.push_back(voxel_center);
      }
    }

////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::getVoxelCentersRecursive (std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > &voxel_centers, const size_t query_depth)
    {
      if (this->depth_ < query_depth){
        if (num_child_ > 0)
        {
          for (size_t i = 0; i < 8; i++)
          {
            if (children_[i])
              children_[i]->getVoxelCentersRecursive (voxel_centers, query_depth);
          }
        }
      }
      else
      {
        Eigen::Vector3d voxel_center = node_metadata_->getVoxelCenter ();
        voxel_centers.push_back(voxel_center);
      }
    }


////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::queryBBIntersects (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, const boost::uint32_t query_depth, std::list<std::string>& file_names)
    {
      
      Eigen::Vector3d my_min = min_bb;
      Eigen::Vector3d my_max = max_bb;
      
      if (intersectsWithBoundingBox (my_min, my_max))
      {
        if (this->depth_ < query_depth)
        {
          if (num_child_ > 0)
          {
            for (size_t i = 0; i < 8; i++)
            {
              if (children_[i])
                children_[i]->queryBBIntersects (my_min, my_max, query_depth, file_names);
            }
          }
          else if (hasUnloadedChildren ())
          {
            loadChildren (false);

            for (size_t i = 0; i < 8; i++)
            {
              if (children_[i])
                children_[i]->queryBBIntersects (my_min, my_max, query_depth, file_names);
            }
          }
          return;
        }

        if (!payload_->empty ())
        {
          file_names.push_back (payload_->path ());
        }
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::queryBBIncludes (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, size_t query_depth, const sensor_msgs::PointCloud2::Ptr& dst_blob) 
    {
      uint64_t startingSize = dst_blob->width*dst_blob->height;
      PCL_DEBUG ("[pcl::outofcore::octree_base_node::%s] Starting points in destination blob: %ul\n", __FUNCTION__, startingSize );

      // If the queried bounding box has any intersection with this node's bounding box
      if (intersectsWithBoundingBox (min_bb, max_bb))
      {
        // If we aren't at the max desired depth
        if (this->depth_ < query_depth)
        {
          //if this node doesn't have any children, we are at the max depth for this query
          if ((num_child_ == 0) && (hasUnloadedChildren ()))
            loadChildren (false);

          //if this node has children
          if (num_child_ > 0)
          {
            //recursively store any points that fall into the queried bounding box into v and return
            for (size_t i = 0; i < 8; i++)
            {
              if (children_[i])
                children_[i]->queryBBIncludes (min_bb, max_bb, query_depth, dst_blob);
            }
//            PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Points in dst_blob: %ul\n", __FUNCTION__, dst_blob->width*dst_blob->height );
            return;
          }
        }
        else //otherwise if we are at the max depth
        {
          //get all the points from the payload and return (easy with PointCloud2)
          sensor_msgs::PointCloud2::Ptr tmp_blob (new sensor_msgs::PointCloud2 ());
          sensor_msgs::PointCloud2::Ptr tmp_dst_blob (new sensor_msgs::PointCloud2 ());
          //load all the data in this node from disk
          payload_->readRange (0, payload_->size (), tmp_blob);

          if( tmp_blob->width*tmp_blob->height == 0 )
            return;

          //if this node's bounding box falls completely within the queried bounding box, keep all the points
          if (inBoundingBox (min_bb, max_bb))
          {
            //concatenate all of what was just read into the main dst_blob
            //(is it safe to do in place?)
            
            //if there is already something in the destination blob (remember this method is recursive)
            if( dst_blob->width*dst_blob->height != 0 )
            {
//              PCL_INFO ("[pcl::outofocre::octree_base_node::%s] Size of cloud before: %lu\n", __FUNCTION__, dst_blob->width*dst_blob->height );

              //can this be done in place?
//              PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Concatenating point cloud\n", __FUNCTION__);
              int res = pcl::concatenatePointCloud (*dst_blob, *tmp_blob, *dst_blob);
              (void)res;
              assert (res == 1);

//              PCL_INFO ("[pcl::outofocre::octree_base_node::%s] Size of cloud after: %lu\n", __FUNCTION__, dst_blob->width*dst_blob->height );
            }
            //otherwise, just copy the tmp_blob into the dst_blob
            else 
            {
//              PCL_INFO ( "[pcl::outofcore::octree_base_node] Copying point cloud into the destination blob\n");
              pcl::copyPointCloud (*tmp_blob, *dst_blob);
              assert (tmp_blob->width*tmp_blob->height == dst_blob->width*dst_blob->height);
            }
//            PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Points in dst_blob: %ul\n", __FUNCTION__, dst_blob->width*dst_blob->height );
            return;
          }
          //otherwise queried bounding box only partially intersects this
          //node's bounding box, so we have to check all the points in
          //this box for intersection with queried bounding box
          else
          {
//            PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Partial extraction of points in bounding box. Desired: %.2lf %.2lf %2lf, %.2lf %.2lf %.2lf; This node BB: %.2lf %.2lf %.2lf, %.2lf %.2lf %.2lf\n", __FUNCTION__, min_bb[0], min_bb[1], min_bb[2], max_bb[0], max_bb[1], max_bb[2], min_[0], min_[1], min_[2], max_[0], max_[1], max_[2] );
            
            //put the ros message into a pointxyz point cloud (just to get the indices by using getPointsInBox)
            pcl::PointCloud<pcl::PointXYZ>::Ptr tmp_cloud ( new pcl::PointCloud<pcl::PointXYZ> () );
            pcl::fromROSMsg ( *tmp_blob, *tmp_cloud );
            assert (tmp_blob->width*tmp_blob->height == tmp_cloud->width*tmp_cloud->height );

            Eigen::Vector4f min_pt ( static_cast<float> ( min_bb[0] ), static_cast<float> ( min_bb[1] ), static_cast<float> ( min_bb[2] ), 1.0f);
            Eigen::Vector4f max_pt ( static_cast<float> ( max_bb[0] ), static_cast<float> ( max_bb[1] ) , static_cast<float>( max_bb[2] ), 1.0f );
                
            std::vector<int> indices;

            pcl::getPointsInBox ( *tmp_cloud, min_pt, max_pt, indices );
//            PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Points in box: %d\n", __FUNCTION__, indices.size () );
//            PCL_INFO ( "[pcl::outofcore::octree_base_node::%s] Points remaining: %d\n", __FUNCTION__, tmp_cloud->width*tmp_cloud->height - indices.size () );

            if ( indices.size () > 0 )
            {
              if( dst_blob->width*dst_blob->height > 0 )
              {
                //need a new tmp destination with extracted points within BB
                sensor_msgs::PointCloud2::Ptr tmp_blob_within_bb (new sensor_msgs::PointCloud2 ());
                
                //copy just the points marked in indices
                pcl::copyPointCloud ( *tmp_blob, indices, *tmp_blob_within_bb );
                assert ( tmp_blob_within_bb->width*tmp_blob_within_bb->height == indices.size () );
                assert ( tmp_blob->fields.size () == tmp_blob_within_bb->fields.size () );
                //concatenate those points into the returned dst_blob
//                PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Concatenating point cloud in place\n", __FUNCTION__);
                boost::uint64_t orig_points_in_destination = dst_blob->width*dst_blob->height;
                (void)orig_points_in_destination;
                int res = pcl::concatenatePointCloud (*dst_blob, *tmp_blob_within_bb, *dst_blob);
                (void)res;
                assert (res == 1);
                assert (dst_blob->width*dst_blob->height == indices.size () + orig_points_in_destination);

              }
              else
              {
                pcl::copyPointCloud ( *tmp_blob, indices, *dst_blob );
                assert ( dst_blob->width*dst_blob->height == indices.size () );
              }
            }
          }
        }
      }

//      PCL_INFO ("[pcl::outofcore::octree_base_node::%s] Points added by function call: %ul\n", __FUNCTION__, dst_blob->width*dst_blob->height - startingSize );
    }

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::queryBBIncludes (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, size_t query_depth, AlignedPointTVector& v)
    {

      //if the queried bounding box has any intersection with this node's bounding box
      if (intersectsWithBoundingBox (min_bb, max_bb))
      {
        //if we aren't at the max desired depth
        if (this->depth_ < query_depth)
        {
          //if this node doesn't have any children, we are at the max depth for this query
          if ((num_child_ == 0) && (hasUnloadedChildren ()))
          {
            loadChildren (false);
          }

          //if this node has children
          if (num_child_ > 0)
          {
            //recursively store any points that fall into the queried bounding box into v and return
            for (size_t i = 0; i < 8; i++)
            {
              if (children_[i])
                children_[i]->queryBBIncludes (min_bb, max_bb, query_depth, v);
            }
            return;
          }
        }
        //otherwise if we are at the max depth
        else
        {
          //if this node's bounding box falls completely within the queried bounding box
          if (inBoundingBox (min_bb, max_bb))
          {
            //get all the points from the payload and return
            AlignedPointTVector payload_cache;
            payload_->readRange (0, payload_->size (), payload_cache);
            v.insert (v.end (), payload_cache.begin (), payload_cache.end ());
            return;
          }
          //otherwise queried bounding box only partially intersects this
          //node's bounding box, so we have to check all the points in
          //this box for intersection with queried bounding box
          else
          {
            //read _all_ the points in from the disk container
            AlignedPointTVector payload_cache;
            payload_->readRange (0, payload_->size (), payload_cache);
        
            uint64_t len = payload_->size ();
            //iterate through each of them
            for (uint64_t i = 0; i < len; i++)
            {
              const PointT& p = payload_cache[i];
              //if it falls within this bounding box
              if (pointInBoundingBox (min_bb, max_bb, p))
              {
                //store it in the list
                v.push_back (p);
              }
            }
          }
        }
      }
    }
    
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::queryBBIncludes_subsample (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb, int query_depth, const double percent, AlignedPointTVector& dst)
    {
      //check if the queried bounding box has any intersection with this node's bounding box
      if (intersectsWithBoundingBox (min_bb, max_bb))
      {
        //if we are not at the max depth for queried nodes
        if (this->depth_ < query_depth)
        {
          //check if we don't have children
          if ((num_child_ == 0) && (hasUnloadedChildren ()))
          {
            loadChildren (false);
          }
          //if we do have children
          if (num_child_ > 0)
          {
            //recursively add their valid points within the queried bounding box to the list v
            for (size_t i = 0; i < 8; i++)
            {
              if (children_[i])
                children_[i]->queryBBIncludes_subsample (min_bb, max_bb, query_depth, percent, dst);
            }
            return;
          }
        }
        //otherwise we are at the max depth, so we add all our points or some of our points
        else
        {
          //if this node's bounding box falls completely within the queried bounding box
          if (inBoundingBox (min_bb, max_bb))
          {
            //add a random sample of all the points
            AlignedPointTVector payload_cache;
            payload_->readRangeSubSample (0, payload_->size (), percent, payload_cache);
            dst.insert (dst.end (), payload_cache.begin (), payload_cache.end ());
            return;
          }
          //otherwise the queried bounding box only partially intersects with this node's bounding box
          else
          {
            //brute force selection of all valid points
            AlignedPointTVector payload_cache_within_region;
            {
              AlignedPointTVector payload_cache;
              payload_->readRange (0, payload_->size (), payload_cache);
              for (size_t i = 0; i < payload_->size (); i++)
              {
                const PointT& p = payload_cache[i];
                if (pointInBoundingBox (min_bb, max_bb, p))
                {
                  payload_cache_within_region.push_back (p);
                }
              }
            }//force the payload cache to deconstruct here

            //** \todo: this is likely to be very slow.*/

            //use STL random_shuffle and push back a random selection of the points onto our list
            std::random_shuffle (payload_cache_within_region.begin (), payload_cache_within_region.end ());
            size_t numpick = percent * payload_cache_within_region.size ();

            for (size_t i = 0; i < numpick; i++)
            {
              dst.push_back (payload_cache_within_region[i]);
            }
          }
        }
      }
    }
////////////////////////////////////////////////////////////////////////////////

//dir is current level. we put this nodes files into it
    template<typename ContainerT, typename PointT>
    octree_base_node<ContainerT, PointT>::octree_base_node (const Eigen::Vector3d& bb_min, const Eigen::Vector3d& bb_max, const char* dir, octree_base_node<ContainerT,PointT>* super)
      : m_tree_ ()
      , root_ ()
      , parent_ ()
      , depth_ ()
      , children_ ()
      , num_child_ ()
      , payload_ ()
      , node_metadata_ ()
    {
      node_metadata_ = boost::shared_ptr<OutofcoreOctreeNodeMetadata> (new OutofcoreOctreeNodeMetadata ());
      node_metadata_->setOutofcoreVersion (3);
      
      if (super == NULL)
      {
        PCL_ERROR ( "[pc::outofcore::octree_base_node] Super is null - don't make a root node this way!\n" );
        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Exception: Bad parent");
      }

      this->parent_ = super;
      root_ = super->root_;
      depth_ = super->depth_ + 1;
      memset (children_, 0, 8 * sizeof(octree_base_node<ContainerT, PointT>*));
      num_child_ = 0;

      node_metadata_->setBoundingBox (bb_min, bb_max);

      std::string uuid_idx;
      std::string uuid_cont;
      octree_disk_container<PointT>::getRandomUUIDString (uuid_idx);
      octree_disk_container<PointT>::getRandomUUIDString (uuid_cont);

      std::string node_index_name = uuid_idx + std::string ("_") + node_index_basename + node_index_extension;

      std::string node_container_name;
      if( true )//OUTOFCORE_VERSION_ >= 3 )
      {
        node_container_name = uuid_cont + std::string ("_") + node_container_basename + pcd_extension;
      }
      else
      {
        node_container_name = uuid_cont + std::string ("_") + node_container_basename + node_container_extension;
      }

      node_metadata_->setDirectoryPathname (boost::filesystem::path (dir));
      node_metadata_->setPCDFilename (node_metadata_->getDirectoryPathname () / boost::filesystem::path (node_container_name));
      node_metadata_->setMetadataFilename ( node_metadata_->getDirectoryPathname ()/boost::filesystem::path (node_index_name));

      boost::filesystem::create_directory (node_metadata_->getDirectoryPathname ());

      payload_ = boost::shared_ptr<ContainerT> (new ContainerT (node_metadata_->getPCDFilename ()));
      saveIdx (false);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::copyAllCurrentAndChildPointsRec (std::list<PointT>& v)
    {
      if ((num_child_ == 0) && (hasUnloadedChildren ()))
      {
        loadChildren (false);
      }

      for (size_t i = 0; i < num_child_; i++)
      {
        children_[i]->copyAllCurrentAndChildPointsRec (v);
      }

      AlignedPointTVector payload_cache;
      payload_->readRange (0, payload_->size (), payload_cache);

      {
        //boost::mutex::scoped_lock lock(queryBBIncludes_vector_mutex);
        v.insert (v.end (), payload_cache.begin (), payload_cache.end ());
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::copyAllCurrentAndChildPointsRec_sub (std::list<PointT>& v, const double percent)
    {
      if ((num_child_ == 0) && (hasUnloadedChildren ()))
      {
        loadChildren (false);
      }

      for (size_t i = 0; i < 8; i++)
      {
        if (children_[i])
          children_[i]->copyAllCurrentAndChildPointsRec_sub (v, percent);
      }

      std::vector<PointT> payload_cache;
      payload_->readRangeSubSample (0, payload_->size (), percent, payload_cache);

      for (size_t i = 0; i < payload_cache.size (); i++)
      {
        v.push_back (payload_cache[i]);
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> inline bool
    octree_base_node<ContainerT, PointT>::intersectsWithBoundingBox (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb) const
    {
      Eigen::Vector3d min, max;
      node_metadata_->getBoundingBox (min, max);
      
      if (((min[0] <= min_bb[0]) && (min_bb[0] <= max[0])) || ((min_bb[0] <= min[0]) && (min[0] <= max_bb[0])))
      {
        if (((min[1] <= min_bb[1]) && (min_bb[1] <= max[1])) || ((min_bb[1] <= min[1]) && (min[1] <= max_bb[1])))
        {
          if (((min[2] <= min_bb[2]) && (min_bb[2] <= max[2])) || ((min_bb[2] <= min[2]) && (min[2] <= max_bb[2])))
          {
            return (true);
          }
        }
      }

      return (false);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> inline bool
    octree_base_node<ContainerT, PointT>::inBoundingBox (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb) const
    {
      Eigen::Vector3d min, max;

      node_metadata_->getBoundingBox (min, max);

      if ((min_bb[0] <= min[0]) && (max[0] <= max_bb[0]))
      {
        if ((min_bb[1] <= min[1]) && (max[1] <= max_bb[1]))
        {
          if ((min_bb[2] <= min[2]) && (max[2] <= max_bb[2]))
          {
            return (true);
          }
        }
      }

      return (false);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> inline bool
    octree_base_node<ContainerT, PointT>::pointInBoundingBox (const Eigen::Vector3d& min_bb, const Eigen::Vector3d& max_bb,
                                                         const PointT& p)
    {
      //by convention, minimum boundary is included; maximum boundary is not
      /// \todo go through all of the code to standardize this
      if ((min_bb[0] <= p.x) && (p.x < max_bb[0]))
      {
        if ((min_bb[1] <= p.y) && (p.y < max_bb[1]))
        {
          if ((min_bb[2] <= p.z) && (p.z < max_bb[2]))
          {
            return (true);
          }
        }
      }
      return (false);
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::writeVPythonVisual (std::ofstream& file)
    {
      Eigen::Vector3d min;
      Eigen::Vector3d max;
      node_metadata_->getBoundingBox (min, max);

      double l = max[0] - min[0];
      double h = max[1] - min[1];
      double w = max[2] - min[2];
      file << "box( pos=(" << min[0] << ", " << min[1] << ", " << min[2] << "), length=" << l << ", height=" << h
           << ", width=" << w << " )\n";

      for (size_t i = 0; i < num_child_; i++)
      {
        children_[i]->writeVPythonVisual (file);
      }
    }

////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::loadFromFile (const boost::filesystem::path& path, octree_base_node<ContainerT, PointT>* super)
    {
      node_metadata_->loadMetadataFromDisk (path);

      //Validate
      /* //Ignore for now

      if (!((version) && (bb_min) && (bb_max) && (bin)))
      {
        PCL_ERROR ( "[pcl::outofcore::octree_base_node] index %s failed to parse! Doesn't contain all attributes\n", path.c_str () );
        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Octree Parse Failure: Metadata does not contain all attributes");
      }
      if ((version->type != cJSON_Number) || (bb_min->type != cJSON_Array) || (bb_max->type != cJSON_Array) || (bin->type != cJSON_String))
      {
        PCL_ERROR ( "[pcl::outofcore::octree_base_node] index %s failed to parse! Invalid data types\n", path.c_str () );
        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Octree Parse Failure: Metadata contains invalid data types");
      }
      if (version->valuedouble != 2.0 && version->valuedouble != 3.0)
      {
        PCL_ERROR ( "[pcl::outofcore::octree_base_node] index %s failed to parse!\n  Incompatible version", path.c_str () );
        PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Octree Parse Failure: Incompatible version");
      }

      //	version->valuedouble;
      */
      //this shouldn't be part of 'loadFromFile'
      this->parent_ = super;
      memset (children_, 0, 8 * sizeof(octree_base_node<ContainerT, PointT>*));
      this->num_child_ = 0;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::convertToXYZRecursive ()
    {
      std::string fname = boost::filesystem::basename (node_metadata_->getPCDFilename ()) + std::string (".dat.xyz");
      boost::filesystem::path xyzfile = node_metadata_->getDirectoryPathname () / fname;
      payload_->convertToXYZ (xyzfile);

      if (hasUnloadedChildren ())
      {
        loadChildren (false);
      }

      for (size_t i = 0; i < 8; i++)
      {
        if (children_[i])
          children_[i]->convertToXYZ ();
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::flushToDiskRecursive ()
    {
      for (size_t i = 0; i < 8; i++)
      {
        if (children_[i])
          children_[i]->flushToDiskRecursive ();
      }
    }
////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> octree_base_node<ContainerT, PointT>*
    makenode_norec (const boost::filesystem::path& path, octree_base_node<ContainerT, PointT>* super)
    {
      octree_base_node<ContainerT, PointT>* thisnode = new octree_base_node<octree_disk_container < PointT > , PointT > ();
//octree_disk_node ();

      if (super == NULL)
      {
        thisnode->thisdir_ = path.parent_path ();

        if (!boost::filesystem::exists (thisnode->thisdir_))
        {
          PCL_ERROR ( "[pcl::outofcore::octree_base_node] could not find dir %s\n",thisnode->thisdir_.c_str () );
          PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Outofcore Octree Exception: Could not find directory");
        }

        thisnode->thisnodeindex_ = path;

        thisnode->depth_ = 0;
        thisnode->root_ = thisnode;
      }
      else
      {
        thisnode->thisdir_ = path;
        thisnode->depth_ = super->depth_ + 1;
        thisnode->root_ = super->root_;

        if (thisnode->depth_ > thisnode->root->max_depth_)
        {
          thisnode->root->max_depth_ = thisnode->depth_;
        }

        boost::filesystem::directory_iterator diterend;
        bool loaded = false;
        for (boost::filesystem::directory_iterator diter (thisnode->thisdir_); diter != diterend; ++diter)
        {
          const boost::filesystem::path& file = *diter;
          if (!boost::filesystem::is_directory (file))
          {
            if (boost::filesystem::extension (file) == octree_base_node<ContainerT, PointT>::node_index_extension)
            {
              thisnode->thisnodeindex_ = file;
              loaded = true;
              break;
            }
          }
        }

        if (!loaded)
        {
          PCL_ERROR ( "[pcl::outofcore::octree_base_node] Could not find index!\n");
          PCL_THROW_EXCEPTION (PCLException, "[pcl::outofcore::octree_base_node] Could not find node metadata index file");
        }

      }
      thisnode->max_depth_ = 0;

      {
        std::ifstream f (thisnode->thisnodeindex_.string ().c_str (), std::ios::in);

        f >> thisnode->min_[0];
        f >> thisnode->min_[1];
        f >> thisnode->min_[2];
        f >> thisnode->max_[0];
        f >> thisnode->max_[1];
        f >> thisnode->max_[2];

        std::string filename;
        f >> filename;
        thisnode->thisnodestorage_ = thisnode->thisdir_ / filename;

        f.close ();

        thisnode->payload_ = boost::shared_ptr<ContainerT> (new ContainerT (thisnode->thisnodestorage_));
      }

      thisnode->parent_ = super;
      memset (thisnode->children_, 0, 8 * sizeof(octree_base_node<octree_disk_container < PointT > , PointT >*));//octree_disk_node*));
      thisnode->num_child_ = 0;

      return (thisnode);
    }
////////////////////////////////////////////////////////////////////////////////

//accelerate search
    template<typename ContainerT, typename PointT> void
    queryBBIntersects_noload (const boost::filesystem::path& root_node, const Eigen::Vector3d& min, const Eigen::Vector3d& max, const boost::uint32_t query_depth, std::list<std::string>& bin_name)
    {
      ///\todo this class already has a private "root" member
      //it also has min[3] and max[3] members
      octree_base_node<ContainerT, PointT>* root = makenode_norec<ContainerT, PointT> (root_node, NULL);
      if (root == NULL)
      {
        std::cout << "test";
      }
      if (root->intersectsWithBoundingBox (min, max))
      {
        if (query_depth == root->max_depth_)
        {
          if (!root->payload_->empty ())
          {
            bin_name.push_back (root->thisnodestorage_.string ());
          }
          return;
        }

        for (int i = 0; i < 8; i++)
        {
          boost::filesystem::path child_dir = root->thisdir_
          / boost::filesystem::path (boost::lexical_cast<std::string> (i));
          if (boost::filesystem::exists (child_dir))
          {
            root->children_[i] = makenode_norec (child_dir, root);
            root->num_child_++;
            queryBBIntersects_noload (root->children_[i], min, max, root->max_depth_ - query_depth, bin_name);
          }
        }
      }
      delete root;
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    queryBBIntersects_noload (octree_base_node<ContainerT, PointT>* current, const Eigen::Vector3d& min, const Eigen::Vector3d& max, const boost::uint32_t query_depth, std::list<std::string>& bin_name)
    {
      if (current->intersectsWithBoundingBox (min, max))
      {
        if (current->depth_ == query_depth)
        {
          if (!current->payload_->empty ())
          {
            bin_name.push_back (current->thisnodestorage_.string ());
          }
        }
        else
        {
          for (int i = 0; i < 8; i++)
          {
            boost::filesystem::path child_dir = current->thisdir_ / boost::filesystem::path (boost::lexical_cast<std::string> (i));
            if (boost::filesystem::exists (child_dir))
            {
              current->children_[i] = makenode_norec<ContainerT, PointT> (child_dir, current);
              current->num_child_++;
              queryBBIntersects_noload (current->children_[i], min, max, query_depth, bin_name);
            }
          }
        }
      }
    }
////////////////////////////////////////////////////////////////////////////////

    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::sortOctantIndices (const sensor_msgs::PointCloud2::Ptr &input_cloud, std::vector< std::vector<int> > &indices, const Eigen::Vector3d &mid_xyz)
    {
      if (indices.size () < 8)
        indices.resize (8);

      int x_idx = pcl::getFieldIndex (*input_cloud , std::string ("x") );
      int y_idx = pcl::getFieldIndex (*input_cloud, std::string ("y") );
      int z_idx = pcl::getFieldIndex (*input_cloud, std::string ("z") );

      int x_offset = input_cloud->fields[x_idx].offset;
      int y_offset = input_cloud->fields[y_idx].offset;
      int z_offset = input_cloud->fields[z_idx].offset;
      
      for ( size_t point_idx =0; point_idx < input_cloud->data.size (); point_idx +=input_cloud->point_step )
      {
        PointXYZ local_pt;

        local_pt.x = * (reinterpret_cast<float*>(&input_cloud->data[point_idx + x_offset]));
        local_pt.y = * (reinterpret_cast<float*>(&input_cloud->data[point_idx + y_offset]));
        local_pt.z = * (reinterpret_cast<float*>(&input_cloud->data[point_idx + z_offset]));

        if( !this->pointInBoundingBox (local_pt) )
        {
          PCL_ERROR ( "[pcl::outofcore::octree_base_node::%s] Failed to place point ( %.2f,%.2f,%.2f) within bounding box\n", __FUNCTION__, local_pt.x, local_pt.y, local_pt.z );
          continue;
        }

        //compute the box we are in
        size_t box = 0;
        box = ((local_pt.z >= mid_xyz[2]) << 2) | ((local_pt.y >= mid_xyz[1]) << 1) | ((local_pt.x >= mid_xyz[0]) << 0);
        assert ( box < 8 );
              
        //insert to the vector of indices
        indices[box].push_back ( static_cast<int> ( point_idx / input_cloud->point_step ) );
      }
    }

////////////////////////////////////////////////////////////////////////////////
    template<typename ContainerT, typename PointT> void
    octree_base_node<ContainerT, PointT>::enlargeToCube (Eigen::Vector3d &bb_min, Eigen::Vector3d &bb_max)
    {
      Eigen::Vector3d diff = bb_max - bb_min;
      
      // X is largest, increase y/z in both +/- directions
      if (diff[0] > diff[1] && diff[0] > diff[2])
      {
        bb_min[1] -= (diff[0] - diff[1])/2.0;
        bb_max[1] += (diff[0] - diff[1])/2.0;
        bb_min[2] -= (diff[0] - diff[2])/2.0;
        bb_max[2] += (diff[0] - diff[2])/2.0;
        // Y is largest, increase y/z in both +/- directions
      }
      else if (diff[1] > diff[0] && diff[1] > diff[2])
      {
        bb_min[0] -= (diff[1] - diff[0])/2.0;
        bb_max[0] += (diff[1] - diff[0])/2.0;
        bb_min[2] -= (diff[1] - diff[2])/2.0;
        bb_max[2] += (diff[1] - diff[2])/2.0;
        // Z is largest, increase y/z in both +/- directions
      }
      else if (diff[2] > diff[0] && diff[2] > diff[1])
      {
        bb_min[0] -= (diff[2] - diff[0])/2.0;
        bb_max[0] += (diff[2] - diff[0])/2.0;
        bb_min[1] -= (diff[2] - diff[1])/2.0;
        bb_max[1] += (diff[2] - diff[1])/2.0;
      }
    }
////////////////////////////////////////////////////////////////////////////////
  }//namespace outofcore
}//namespace pcl

//#define PCL_INSTANTIATE....

#endif //PCL_OUTOFCORE_OCTREE_BASE_NODE_IMPL_H_
