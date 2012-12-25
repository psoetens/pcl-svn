/*
 * Software License Agreement (BSD License)
 *
 * Point Cloud Library (PCL) - www.pointclouds.org
 * Copyright (c) 2009-2012, Willow Garage, Inc.
 * Copyright (c) 2012-, Open Perception, Inc.
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
 *  * Neither the name of the copyright holder(s) nor the names of its
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
 * $Id$
 *
 */

#include <pcl/apps/in_hand_scanner/integration.h>

#include <iostream>
#include <vector>
#include <limits>

#include <pcl/console/print.h>
#include <pcl/kdtree/kdtree_flann.h>

////////////////////////////////////////////////////////////////////////////////
// Integration::Dome
////////////////////////////////////////////////////////////////////////////////

pcl::ihs::Integration::Dome::Dome ()
{
  vertices_.col ( 0) = Eigen::Vector4f (-0.000000119f,  0.000000000f, 1.000000000f, 0.f);
  vertices_.col ( 1) = Eigen::Vector4f ( 0.894427180f,  0.000000000f, 0.447213739f, 0.f);
  vertices_.col ( 2) = Eigen::Vector4f ( 0.276393145f,  0.850650907f, 0.447213650f, 0.f);
  vertices_.col ( 3) = Eigen::Vector4f (-0.723606884f,  0.525731146f, 0.447213531f, 0.f);
  vertices_.col ( 4) = Eigen::Vector4f (-0.723606884f, -0.525731146f, 0.447213531f, 0.f);
  vertices_.col ( 5) = Eigen::Vector4f ( 0.276393145f, -0.850650907f, 0.447213650f, 0.f);
  vertices_.col ( 6) = Eigen::Vector4f ( 0.343278527f,  0.000000000f, 0.939233720f, 0.f);
  vertices_.col ( 7) = Eigen::Vector4f ( 0.686557174f,  0.000000000f, 0.727075875f, 0.f);
  vertices_.col ( 8) = Eigen::Vector4f ( 0.792636156f,  0.326477438f, 0.514918089f, 0.f);
  vertices_.col ( 9) = Eigen::Vector4f ( 0.555436373f,  0.652954817f, 0.514918029f, 0.f);
  vertices_.col (10) = Eigen::Vector4f ( 0.106078848f,  0.326477438f, 0.939233720f, 0.f);
  vertices_.col (11) = Eigen::Vector4f ( 0.212157741f,  0.652954817f, 0.727075756f, 0.f);
  vertices_.col (12) = Eigen::Vector4f (-0.065560505f,  0.854728878f, 0.514917910f, 0.f);
  vertices_.col (13) = Eigen::Vector4f (-0.449357629f,  0.730025530f, 0.514917850f, 0.f);
  vertices_.col (14) = Eigen::Vector4f (-0.277718395f,  0.201774135f, 0.939233661f, 0.f);
  vertices_.col (15) = Eigen::Vector4f (-0.555436671f,  0.403548241f, 0.727075696f, 0.f);
  vertices_.col (16) = Eigen::Vector4f (-0.833154857f,  0.201774105f, 0.514917850f, 0.f);
  vertices_.col (17) = Eigen::Vector4f (-0.833154857f, -0.201774150f, 0.514917850f, 0.f);
  vertices_.col (18) = Eigen::Vector4f (-0.277718395f, -0.201774135f, 0.939233661f, 0.f);
  vertices_.col (19) = Eigen::Vector4f (-0.555436671f, -0.403548241f, 0.727075696f, 0.f);
  vertices_.col (20) = Eigen::Vector4f (-0.449357659f, -0.730025649f, 0.514917910f, 0.f);
  vertices_.col (21) = Eigen::Vector4f (-0.065560460f, -0.854728937f, 0.514917850f, 0.f);
  vertices_.col (22) = Eigen::Vector4f ( 0.106078848f, -0.326477438f, 0.939233720f, 0.f);
  vertices_.col (23) = Eigen::Vector4f ( 0.212157741f, -0.652954817f, 0.727075756f, 0.f);
  vertices_.col (24) = Eigen::Vector4f ( 0.555436373f, -0.652954757f, 0.514917970f, 0.f);
  vertices_.col (25) = Eigen::Vector4f ( 0.792636156f, -0.326477349f, 0.514918089f, 0.f);
  vertices_.col (26) = Eigen::Vector4f ( 0.491123378f,  0.356822133f, 0.794654608f, 0.f);
  vertices_.col (27) = Eigen::Vector4f (-0.187592626f,  0.577350259f, 0.794654429f, 0.f);
  vertices_.col (28) = Eigen::Vector4f (-0.607062101f, -0.000000016f, 0.794654369f, 0.f);
  vertices_.col (29) = Eigen::Vector4f (-0.187592626f, -0.577350378f, 0.794654489f, 0.f);
  vertices_.col (30) = Eigen::Vector4f ( 0.491123348f, -0.356822133f, 0.794654548f, 0.f);

  for (unsigned int i=0; i<vertices_.cols (); ++i)
  {
    vertices_.col (i).head <3> ().normalize ();
  }
}

////////////////////////////////////////////////////////////////////////////////

const pcl::ihs::Integration::Dome::Vertices&
pcl::ihs::Integration::Dome::getVertices () const
{
  return (vertices_);
}

////////////////////////////////////////////////////////////////////////////////
// Integration
////////////////////////////////////////////////////////////////////////////////

pcl::ihs::Integration::Integration ()
  : kd_tree_              (new pcl::KdTreeFLANN <PointXYZ> ()),
    squared_distance_max_ (4e-2f),
    dot_normal_min_       (.6f),
    weight_min_           (.3f),
    age_max_              (30),
    count_min_            (5),
    dome_                 ()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
pcl::ihs::Integration::reconstructMesh (const CloudXYZRGBNormalConstPtr& cloud_data,
                                        const MeshPtr&                   mesh_model) const
{
  if (!cloud_data->isOrganized ())
  {
    std::cerr << "ERROR in integration.cpp: Cloud is not organized\n";
    return (false);
  }
  const int width  = static_cast <int> (cloud_data->width);
  const int height = static_cast <int> (cloud_data->height);

  mesh_model->clear ();
  mesh_model->reserveVertices (cloud_data->size ());
  mesh_model->reserveFaces (2 * (width-1) * (height-1));

  // Store which vertex is set at which position (initialized with invalid indices)
  VertexIndices vertex_indices (cloud_data->size (), VertexIndex ());

  // Convert to the model cloud type. This is actually not needed but avoids code duplication (see merge). And reconstructMesh is called only the first reconstruction step anyway.
  // NOTE: The default constructor of PointIHS has to initialize with NaNs!
  CloudIHSPtr cloud_model (new CloudIHS ());
  cloud_model->resize (cloud_data->size ());

  // Set the model points not reached by the main loop
  for (int c=0; c<width; ++c)
  {
    const PointXYZRGBNormal& pt_d = cloud_data->operator [] (c);
    const float weight = -pt_d.normal_z; // weight = -dot (normal, [0; 0; 1])

    if (pcl::isFinite (pt_d) && weight >= weight_min_)
    {
      cloud_model->operator [] (c) = PointIHS (pt_d, weight);
    }
  }
  for (int r=1; r<height; ++r)
  {
    for (int c=0; c<2; ++c)
    {
      const PointXYZRGBNormal& pt_d = cloud_data->operator [] (r*width + c);
      const float weight = -pt_d.normal_z; // weight = -dot (normal, [0; 0; 1])

      if (pcl::isFinite (pt_d) && weight >= weight_min_)
      {
        cloud_model->operator [] (r*width + c) = PointIHS (pt_d, weight);
      }
    }
  }

  // 4   2 - 1  //
  //     |   |  //
  // *   3 - 0  //
  //            //
  // 4 - 2   1  //
  //   \   \    //
  // *   3 - 0  //
  static const int offset_1 = -width;
  static const int offset_2 = -width - 1;
  static const int offset_3 =        - 1;
  static const int offset_4 = -width - 2;

  for (int r=1; r<height; ++r)
  {
    for (int c=2; c<width; ++c)
    {
      const int ind_0 = r*width + c;
      const int ind_1 = ind_0 + offset_1;
      const int ind_2 = ind_0 + offset_2;
      const int ind_3 = ind_0 + offset_3;
      const int ind_4 = ind_0 + offset_4;

      assert (ind_0 >= 0 && ind_0 < static_cast <int> (cloud_data->size ()));
      assert (ind_1 >= 0 && ind_1 < static_cast <int> (cloud_data->size ()));
      assert (ind_2 >= 0 && ind_2 < static_cast <int> (cloud_data->size ()));
      assert (ind_3 >= 0 && ind_3 < static_cast <int> (cloud_data->size ()));
      assert (ind_4 >= 0 && ind_4 < static_cast <int> (cloud_data->size ()));

      const PointXYZRGBNormal& pt_d_0 = cloud_data->operator  [] (ind_0);
      PointIHS&                pt_m_0 = cloud_model->operator [] (ind_0);
      const PointIHS&          pt_m_1 = cloud_model->operator [] (ind_1);
      const PointIHS&          pt_m_2 = cloud_model->operator [] (ind_2);
      const PointIHS&          pt_m_3 = cloud_model->operator [] (ind_3);
      const PointIHS&          pt_m_4 = cloud_model->operator [] (ind_4);

      VertexIndex& vi_0 = vertex_indices [ind_0];
      VertexIndex& vi_1 = vertex_indices [ind_1];
      VertexIndex& vi_2 = vertex_indices [ind_2];
      VertexIndex& vi_3 = vertex_indices [ind_3];
      VertexIndex& vi_4 = vertex_indices [ind_4];

      const float weight = -pt_d_0.normal_z; // weight = -dot (normal, [0; 0; 1])

      if (pcl::isFinite (pt_d_0) && weight >= weight_min_)
      {
        pt_m_0 = PointIHS (pt_d_0, weight);
      }

      this->addToMesh (pt_m_0,pt_m_1,pt_m_2,pt_m_3, vi_0,vi_1,vi_2,vi_3, mesh_model);
      this->addToMesh (pt_m_0,pt_m_2,pt_m_4,pt_m_3, vi_0,vi_2,vi_4,vi_3, mesh_model);
    }
  }

  return (true);
}

////////////////////////////////////////////////////////////////////////////////

bool
pcl::ihs::Integration::merge (const CloudXYZRGBNormalConstPtr& cloud_data,
                              const MeshPtr&                   mesh_model,
                              const Eigen::Matrix4f&           T) const
{
  if (!cloud_data->isOrganized ())
  {
    std::cerr << "ERROR in integration.cpp: Data cloud is not organized\n";
    return (false);
  }
  const int width  = static_cast <int> (cloud_data->width);
  const int height = static_cast <int> (cloud_data->height);

  if (!mesh_model->sizeVertices ())
  {
    std::cerr << "ERROR in integration.cpp: Model mesh is empty\n";
    return (false);
  }

  // Nearest neighbor search
  // TODO: remove this unnecessary copy (I currently keep it because I'm not sure if
  CloudXYZPtr xyz_model (new CloudXYZ ());
  xyz_model->reserve (mesh_model->sizeVertices ());
  for (unsigned int i=0; i<mesh_model->sizeVertices (); ++i)
  {
    const PointIHS& pt = mesh_model->getVertexDataCloud () [i];
    xyz_model->push_back (PointXYZ (pt.x, pt.y, pt.z));
  }
  kd_tree_->setInputCloud (xyz_model);
  std::vector <int>   index (1);
  std::vector <float> squared_distance (1);

  mesh_model->reserveVertices (mesh_model->sizeVertices () + cloud_data->size ());
  mesh_model->reserveFaces (mesh_model->sizeFaces () + 2 * (width-1) * (height-1));

  // Data cloud in model coordinates (this does not change the connectivity information) and weights
  CloudIHSPtr cloud_data_transformed (new CloudIHS ());
  cloud_data_transformed->resize (cloud_data->size ());

  // Sensor position in model coordinates
  const Eigen::Vector4f& sensor_eye = T * Eigen::Vector4f (0.f, 0.f, 0.f, 1.f);

  // Store which vertex is set at which position (initialized with invalid indices)
  VertexIndices vertex_indices (cloud_data->size (), VertexIndex ());

  // Set the transformed points not reached by the main loop
  for (int c=0; c<width; ++c)
  {
    const PointXYZRGBNormal& pt_d = cloud_data->operator [] (c);
    const float weight = -pt_d.normal_z; // weight = -dot (normal, [0; 0; 1])

    if (pcl::isFinite (pt_d) && weight >= weight_min_)
    {
      PointIHS& pt_d_t = cloud_data_transformed->operator [] (c);
      pt_d_t = PointIHS (pt_d, weight);
      pt_d_t.getVector4fMap ()       = T * pt_d_t.getVector4fMap ();
      pt_d_t.getNormalVector4fMap () = T * pt_d_t.getNormalVector4fMap ();
    }
  }
  for (int r=1; r<height; ++r)
  {
    for (int c=0; c<2; ++c)
    {
      const PointXYZRGBNormal& pt_d = cloud_data->operator [] (r*width + c);
      const float weight = -pt_d.normal_z; // weight = -dot (normal, [0; 0; 1])

      if (pcl::isFinite (pt_d) && weight >= weight_min_)
      {
        PointIHS& pt_d_t = cloud_data_transformed->operator [] (r*width + c);
        pt_d_t = PointIHS (pt_d, weight);
        pt_d_t.getVector4fMap ()       = T * pt_d_t.getVector4fMap ();
        pt_d_t.getNormalVector4fMap () = T * pt_d_t.getNormalVector4fMap ();
      }
    }
  }

  // 4   2 - 1  //
  //     |   |  //
  // *   3 - 0  //
  //            //
  // 4 - 2   1  //
  //   \   \    //
  // *   3 - 0  //
  static const int offset_1 = -width;
  static const int offset_2 = -width - 1;
  static const int offset_3 =        - 1;
  static const int offset_4 = -width - 2;

  for (int r=1; r<height; ++r)
  {
    for (int c=2; c<width; ++c)
    {
      const int ind_0 = r*width + c;
      const int ind_1 = ind_0 + offset_1;
      const int ind_2 = ind_0 + offset_2;
      const int ind_3 = ind_0 + offset_3;
      const int ind_4 = ind_0 + offset_4;

      assert (ind_0 >= 0 && ind_0 < static_cast <int> (cloud_data->size ()));
      assert (ind_1 >= 0 && ind_1 < static_cast <int> (cloud_data->size ()));
      assert (ind_2 >= 0 && ind_2 < static_cast <int> (cloud_data->size ()));
      assert (ind_3 >= 0 && ind_3 < static_cast <int> (cloud_data->size ()));
      assert (ind_4 >= 0 && ind_4 < static_cast <int> (cloud_data->size ()));

      const PointXYZRGBNormal& pt_d_0   = cloud_data->operator             [] (ind_0);
      PointIHS&                pt_d_t_0 = cloud_data_transformed->operator [] (ind_0);
      const PointIHS&          pt_d_t_1 = cloud_data_transformed->operator [] (ind_1);
      const PointIHS&          pt_d_t_2 = cloud_data_transformed->operator [] (ind_2);
      const PointIHS&          pt_d_t_3 = cloud_data_transformed->operator [] (ind_3);
      const PointIHS&          pt_d_t_4 = cloud_data_transformed->operator [] (ind_4);

      VertexIndex& vi_0 = vertex_indices [ind_0];
      VertexIndex& vi_1 = vertex_indices [ind_1];
      VertexIndex& vi_2 = vertex_indices [ind_2];
      VertexIndex& vi_3 = vertex_indices [ind_3];
      VertexIndex& vi_4 = vertex_indices [ind_4];

      const float weight = -pt_d_0.normal_z; // weight = -dot (normal, [0; 0; 1])

      if (pcl::isFinite (pt_d_0) && weight >= weight_min_)
      {
        pt_d_t_0 = PointIHS (pt_d_0, weight);
        pt_d_t_0.getVector4fMap ()       = T * pt_d_t_0.getVector4fMap ();
        pt_d_t_0.getNormalVector4fMap () = T * pt_d_t_0.getNormalVector4fMap ();

        pcl::PointXYZ tmp; tmp.getVector4fMap () = pt_d_t_0.getVector4fMap ();

        // NN search
        if (!kd_tree_->nearestKSearch (tmp, 1, index, squared_distance))
        {
          std::cerr << "ERROR in integration.cpp: nearestKSearch failed!\n";
          return (false);
        }

        // Average out corresponding points
        if (squared_distance[0] <= squared_distance_max_)
        {
          PointIHS& pt_m = mesh_model->getVertexDataCloud () [index [0]]; // Non-const reference!

          if (pt_m.getNormalVector4fMap ().dot (pt_d_t_0.getNormalVector4fMap ()) >= dot_normal_min_)
          {
            vi_0 = VertexIndex (index [0]);

            const float W   = pt_m.weight;         // Old accumulated weight
            const float w   = pt_d_t_0.weight;    // Weight of new point
            const float WW  = pt_m.weight = W + w; // New accumulated weight

            const float r_m = static_cast <float> (pt_m.r);
            const float g_m = static_cast <float> (pt_m.g);
            const float b_m = static_cast <float> (pt_m.b);

            const float r_d = static_cast <float> (pt_d_t_0.r);
            const float g_d = static_cast <float> (pt_d_t_0.g);
            const float b_d = static_cast <float> (pt_d_t_0.b);

            pt_m.getVector4fMap ()       = ( W*pt_m.getVector4fMap ()       + w*pt_d_t_0.getVector4fMap ())       / WW;
            pt_m.getNormalVector4fMap () = ((W*pt_m.getNormalVector4fMap () + w*pt_d_t_0.getNormalVector4fMap ()) / WW).normalized ();
            pt_m.r                       = this->trimRGB ((W*r_m + w*r_d) / WW);
            pt_m.g                       = this->trimRGB ((W*g_m + w*g_d) / WW);
            pt_m.b                       = this->trimRGB ((W*b_m + w*b_d) / WW);

            // Point has been observed again -> give it some extra time to live
            pt_m.age = 0;

            // Add a direction
            this->addDirection (pt_m.getNormalVector4fMap (), sensor_eye-pt_m.getVector4fMap (), pt_m.directions);

          } // dot normals
        } // squared distance
      } // isfinite && min weight

      // Connect
      // 4   2 - 1  //
      //     |   |  //
      // *   3 - 0  //
      //            //
      // 4 - 2   1  //
      //   \   \    //
      // *   3 - 0  //
      this->addToMesh (pt_d_t_0,pt_d_t_1,pt_d_t_2,pt_d_t_3, vi_0,vi_1,vi_2,vi_3, mesh_model);
      this->addToMesh (pt_d_t_0,pt_d_t_2,pt_d_t_4,pt_d_t_3, vi_0,vi_2,vi_4,vi_3, mesh_model);
    }
  }

  return (true);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::age (const MeshPtr& mesh, const bool cleanup) const
{
  for (unsigned int i=0; i<mesh->sizeVertices (); ++i)
  {
    PointIHS& pt = mesh->getVertexDataCloud () [i];
    if (pt.age < age_max_)
    {
      // Point survives
      ++(pt.age);
    }
    else if (pt.age == age_max_) // Judgement Day
    {
      if (this->countDirections (pt.directions) < count_min_)
      {
        // Point dies (no need to transform it)
        mesh->deleteVertex (VertexIndex (i));
      }
      else
      {
        // Point becomes immortal
        pt.age = std::numeric_limits <unsigned int>::max ();
      }
    }
  }

  if (cleanup)
  {
    mesh->cleanUp ();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::setDistanceThreshold (const float squared_distance_max)
{
  if (squared_distance_max <= 0.f)
  {
    PCL_ERROR ("'squared_distance_max' must be greater than 0.\n");
  }
  else
  {
    squared_distance_max_ = squared_distance_max;
  }
}

////////////////////////////////////////////////////////////////////////////////

float
pcl::ihs::Integration::getDistanceThreshold () const
{
  return (squared_distance_max_);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::setAngleThreshold (const float dot_normal_min)
{
  if (dot_normal_min < -1.f || dot_normal_min > 1.f)
  {
    PCL_ERROR ("'dot_normal_min' must be between -1 and 1.\n");
  }
  else
  {
    dot_normal_min_ = dot_normal_min;
  }
}

////////////////////////////////////////////////////////////////////////////////

float
pcl::ihs::Integration::getAngleThreshold () const
{
  return (dot_normal_min_);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::setMinimumWeight (const float weight_min)
{
  if (weight_min <= 0.f || weight_min > 1.f)
  {
    PCL_ERROR ("'weight_min' must be between 0 (excluded) and 1.\n");
  }
  else
  {
    weight_min_ = weight_min;
  }
}

////////////////////////////////////////////////////////////////////////////////

float
pcl::ihs::Integration::getMinimumWeight () const
{
  return (weight_min_);
}

void
pcl::ihs::Integration::setMaximumAge (const unsigned int age_max)
{
  if (age_max <= 0)
  {
    PCL_ERROR ("'age_max' must be greater than 0\n");
  }
  else
  {
    age_max_ = age_max;
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int
pcl::ihs::Integration::getMaximumAge () const
{
  return (age_max_);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::setMinimumCount (const unsigned int count_min)
{
  count_min_ = count_min;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int
pcl::ihs::Integration::getMinimumCount () const
{
  return (count_min_);
}

////////////////////////////////////////////////////////////////////////////////

uint8_t
pcl::ihs::Integration::trimRGB (const float val) const
{
  return (static_cast <uint8_t> (val > 255.f ? 255 : val));
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::addToMesh (const PointIHS& pt_0,
                                  const PointIHS& pt_1,
                                  const PointIHS& pt_2,
                                  const PointIHS& pt_3,
                                  VertexIndex&    vi_0,
                                  VertexIndex&    vi_1,
                                  VertexIndex&    vi_2,
                                  VertexIndex&    vi_3,
                                  const MeshPtr&  mesh) const
{
  // Treated bitwise
  // 2 - 1
  // |   |
  // 3 - 0
  const unsigned char is_finite = static_cast <unsigned char> ((1 * pcl::isFinite (pt_0)) | (2 * pcl::isFinite (pt_1)) | (4 * pcl::isFinite (pt_2)) | (8 * pcl::isFinite (pt_3)));

  switch (is_finite)
  {
    case  7: this->addToMesh (pt_0, pt_1, pt_2, vi_0, vi_1, vi_2, mesh); break; // 0-1-2
    case 11: this->addToMesh (pt_0, pt_1, pt_3, vi_0, vi_1, vi_3, mesh); break; // 0-1-3
    case 13: this->addToMesh (pt_0, pt_2, pt_3, vi_0, vi_2, vi_3, mesh); break; // 0-2-3
    case 14: this->addToMesh (pt_1, pt_2, pt_3, vi_1, vi_2, vi_3, mesh); break; // 1-2-3
    case 15: // 0-1-2-3
    {
      if (!distanceThreshold (pt_0, pt_1, pt_2, pt_3)) break;
      if (!vi_0.isValid ()) vi_0 = mesh->addVertex (pt_0);
      if (!vi_1.isValid ()) vi_1 = mesh->addVertex (pt_1);
      if (!vi_2.isValid ()) vi_2 = mesh->addVertex (pt_2);
      if (!vi_3.isValid ()) vi_3 = mesh->addVertex (pt_3);
      mesh->addTrianglePair (vi_0, vi_1, vi_2, vi_3);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::addToMesh (const PointIHS& pt_0,
                                  const PointIHS& pt_1,
                                  const PointIHS& pt_2,
                                  VertexIndex&    vi_0,
                                  VertexIndex&    vi_1,
                                  VertexIndex&    vi_2,
                                  const MeshPtr&  mesh) const
{
  if (!distanceThreshold (pt_0, pt_1, pt_2)) return;

  if (!vi_0.isValid ()) vi_0 = mesh->addVertex (pt_0);
  if (!vi_1.isValid ()) vi_1 = mesh->addVertex (pt_1);
  if (!vi_2.isValid ()) vi_2 = mesh->addVertex (pt_2);

  mesh->addFace (vi_0, vi_1, vi_2);
}

////////////////////////////////////////////////////////////////////////////////

bool
pcl::ihs::Integration::distanceThreshold (const PointIHS& pt_0,
                                          const PointIHS& pt_1,
                                          const PointIHS& pt_2) const
{
  if ((pt_0.getVector3fMap () - pt_1.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  if ((pt_1.getVector3fMap () - pt_2.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  if ((pt_2.getVector3fMap () - pt_0.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  return (true);
}

////////////////////////////////////////////////////////////////////////////////

bool
pcl::ihs::Integration::distanceThreshold (const PointIHS& pt_0,
                                          const PointIHS& pt_1,
                                          const PointIHS& pt_2,
                                          const PointIHS& pt_3) const
{
  if ((pt_0.getVector3fMap () - pt_1.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  if ((pt_1.getVector3fMap () - pt_2.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  if ((pt_2.getVector3fMap () - pt_3.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  if ((pt_3.getVector3fMap () - pt_0.getVector3fMap ()).squaredNorm () > squared_distance_max_) return (false);
  return (true);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::Integration::addDirection (const Eigen::Vector4f& normal,
                                     const Eigen::Vector4f& direction,
                                     uint32_t&              directions) const
{
  // Find the rotation that aligns the normal with [0; 0; 1]
  const float dot = normal.z ();

  Eigen::Isometry3f R = Eigen::Isometry3f::Identity ();

  // No need to transform if the normal is already very close to [0; 0; 1] (also avoids numerical issues)
  // TODO: The value .969 (actually 0.9696) is hard coded for a frequency=3.
  //       It can be calculated with 1-(1-max(dome_vertices_.z))/2
  if (dot <= .969f)
  {
    R = Eigen::Isometry3f (Eigen::AngleAxisf(std::acos(dot), Eigen::Vector3f(normal.y(), -normal.x(), 0.f).normalized()));
  }

  // Transform the direction into the dome coordinate system (which is aligned with the normal)
  Eigen::Vector4f aligned_direction = (R * direction);
  aligned_direction.head <3> ().normalize ();

  // Find the closest viewing direction
  // NOTE: cos(0deg) = 1 = max
  //       acos(angle) = dot(a,b) / (norm(a) * norm(b)
  //       m_sphere_vertices are already normalized
  unsigned int index = 0;
  (aligned_direction.transpose () * dome_.getVertices ()).maxCoeff (&index);

  // Set the observed direction bit at 'index'
  // http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c/47990#47990
  directions |= 1 << index;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int
pcl::ihs::Integration::countDirections (const unsigned int directions) const
{
  // http://stackoverflow.com/questions/109023/best-algorithm-to-count-the-number-of-set-bits-in-a-32-bit-integer/109025#109025
  unsigned int i = directions - ((directions >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);

  return ((((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24);
}

////////////////////////////////////////////////////////////////////////////////
