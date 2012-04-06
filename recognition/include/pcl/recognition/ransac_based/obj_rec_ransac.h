/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2012, Willow Garage, Inc.
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
 *
 */

#ifndef PCL_RECOGNITION_OBJ_REC_RANSAC_H_
#define PCL_RECOGNITION_OBJ_REC_RANSAC_H_

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <Eigen/Core>
#include <string>
#include <list>

namespace pcl
{
  namespace recognition
  {
    /** \brief This is a RANSAC-based 3D object recognition method. Do the following to use it: (i) call addModel() k times with k different models
      * representing the objects to be recognized and (ii) call recognize() with the 3D scene in which the objects should be recognized. Recognition means both
      * object identification and pose (position + orientation) estimation. Check the method descriptions for more details.
      *
      * \note If you use this code in any academic work, please cite:
      *
      *   - Chavdar Papazov, Sami Haddadin, Sven Parusel, Kai Krieger and Darius Burschka.
      *     Rigid 3D geometry matching for grasping of known objects in cluttered scenes.
      *     The International Journal of Robotics Research 2012. DOI: 10.1177/0278364911436019
      *
      *   - Chavdar Papazov and Darius Burschka.
      *     An Efficient RANSAC for 3D Object Recognition in Noisy and Occluded Scenes.
      *     In Proceedings of the 10th Asian Conference on Computer Vision (ACCV'10),
      *     November 2010.
      *
      *
      * \author Chavdar Papazov
      * \ingroup recognition
      */
    class ObjRecRANSAC
    {
      public:
        /** \brief This is an output item of the ObjRecRANSAC::recognize() method. It contains the recognized model, its name (the ones passed to
          * ObjRecRANSAC::addModel()), the rigid transform which aligns the model with the input scene and the match confidence which is a number
          * in the interval (0, 1] which gives the fraction of the model surface area matched to the scene. E.g., a match confidence of 0.3 means
          * that 30% of the object surface area was matched to the scene points. If the scene is represented by a single range image, the match
          * confidence can not be greater than 0.5 since the range scanner sees only one side of each object.
          */
        class Output
        {
          public:
            Output(const pcl::PointCloud<pcl::PointXYZ>& model, const std::string& object_name, const Eigen::Matrix4f& rigid_transform, double match_confidence) :
              model_ (model),
              object_name_ (object_name),
              rigid_transform_ (rigid_transform),
              match_confidence_ (match_confidence)
            { }
            virtual ~Output(){ }

          public:
            const pcl::PointCloud<pcl::PointXYZ>& model_;
            const std::string& object_name_;
            Eigen::Matrix4f rigid_transform_;
            double match_confidence_;
        };

      public:
        /** \brief Constructor with some important parameters which can not be changed once an instance of that class is created.
          *
          * \param[in] pair_width should be roughly half the extent of the visible object part. This means, for each object point p there should be (at least)
          * one point q (from the same object) such that ||p - q|| <= pair_width. Tradeoff: smaller values allow for detection in more occluded scenes but lead
          * to more imprecise alignment. Bigger values lead to better alignment but require large visible object parts (i.e., less occlusion).
          *
          * \param[in] voxel_size is the size of the leafs of the octree, i.e., the "size" of the discretization. Tradeoff: High values lead to less
          * computation time but ignore object details. Small values allow to better distinguish between objects, but will introduce more holes in the resulting
          * "voxel-surface" (especially for a sparsely sampled scene) and thus will make normal computation unreliable.
          *
          * \param[in] fraction_of_pairs_in_hash_table determines how many pairs (as fraction of the total number) will be kept in the hashtable. Use the default
          * value (check the papers above for more details). */
        ObjRecRANSAC(double pair_width, double voxel_size, double fraction_of_pairs_in_hash_table = 0.8);
        virtual ~ObjRecRANSAC();

        /** \brief Add an object model to be recognized.
          *
          * \param[in] model represents the object to be recognized.
          * \param[in] normals are the normals associated with 'model'.
          * \param[in] object_name is an identifier for 'model'. If 'model' is detected in the scene 'object_name' is returned by the recognition method and you
          * know which model has been detected. Note that 'object_name' has to be unique!
          *
          * The method returns true if the model was successfully added to the model library and false otherwise (e.g., if 'object_name' is already in use).
          */
        bool
        addModel(const pcl::PointCloud<pcl::PointXYZ>& model, const pcl::PointCloud<pcl::Normal>& normals, const std::string& object_name);

        /** \brief This method performs the recognition of the models loaded to the model library with the method addModel().
          *
          * \param[in]  scene is the 3d scene in which the object should be recognized.
          * \param[in]  normals are the scene normals.
          * \param[out] recognized_objects is the list of output items each one containing the recognized model instance, its name, the aligning rigid transform
          * and the match confidence (see ObjRecRANSAC::Output for further explanations).
          */
        void
        recognize(const pcl::PointCloud<pcl::PointXYZ>& scene, const pcl::PointCloud<pcl::Normal>& normals, std::list<ObjRecRANSAC::Output>& recognized_objects);
    };
  } // namespace recognition
} // namespace pcl

#endif // PCL_RECOGNITION_OBJ_REC_RANSAC_H_

