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
 * $Id: test_plane_intersection.cpp 5686 2012-05-11 20:59:00Z gioia $
 */

#include <gtest/gtest.h>
#include <pcl/common/common.h>
#include <pcl/common/intersections.h>
#include <pcl/pcl_tests.h>


using namespace pcl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST (PCL, lineWithLineIntersection)
{
  Eigen::Vector4f line_a;
  Eigen::Vector4f line_b;

  //case 1
  line_a.x() = 0.09100f;
  line_a.y() = 0.00200f;
  line_a.z() = 0.00300f;
  line_a.w() = 0.00400f;

  line_b.x() = 0.0157f;
  line_b.y() = 0.0333f;
  line_b.z() = 0.0678f;
  line_b.w() = 0.0995f;

  Eigen::Vector4f p1, p2;
  lineToLineSegment (line_a, line_b, p1, p2);

  Eigen::Vector4f point_case_1;
  bool result_case_1 = lineWithLineIntersection(line_a, line_b, point_case_1);

  double sqr_dist_case_1 = (p1 - p2).squaredNorm ();

  double default_sqr_eps = 1e-4;
  EXPECT_GT(sqr_dist_case_1, default_sqr_eps);
  Eigen::Vector4f zero(0.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_EQ(point_case_1, zero);
  EXPECT_FALSE(result_case_1);

  pcl::ModelCoefficients line_a_mod;
  pcl::ModelCoefficients line_b_mod;

  std::vector<float> values_a_case_1;
  values_a_case_1.push_back(line_a.x());
  values_a_case_1.push_back(line_a.y());
  values_a_case_1.push_back(line_a.z());
  values_a_case_1.push_back(line_a.w());

  std::vector<float> values_b_case_1;
  values_b_case_1.push_back(line_b.x());
  values_b_case_1.push_back(line_b.y());
  values_b_case_1.push_back(line_b.z());
  values_b_case_1.push_back(line_b.w());

  line_a_mod.values = values_a_case_1;
  line_b_mod.values = values_b_case_1;

  Eigen::Vector4f point_mod_1;
  EXPECT_FALSE(lineWithLineIntersection(line_a_mod, line_b_mod, point_mod_1));
  EXPECT_EQ(result_case_1, lineWithLineIntersection(line_a_mod, line_b_mod, point_mod_1));
  EXPECT_EQ(point_mod_1, zero);

  //case 2
  line_a.x() = 0.09100f;
  line_a.y() = 0.00200f;
  line_a.z() = 0.00300f;
  line_a.w() = 0.00400f;

  line_b.x() = 0.00157f;
  line_b.y() = 0.00333f;
  line_b.z() = 0.00678f;
  line_b.w() = 0.00995f;

  lineToLineSegment (line_a, line_b, p1, p2);

  Eigen::Vector4f point_case_2;
  double sqr_eps_case_2 = 1e-1;
  bool result_case_2 = lineWithLineIntersection(line_a, line_b, point_case_2, sqr_eps_case_2);

  double sqr_dist_case_2 = (p1 - p2).squaredNorm ();
  EXPECT_LT(sqr_dist_case_2, sqr_eps_case_2);
  EXPECT_EQ(point_case_2, p1);
  EXPECT_TRUE(result_case_2);

  pcl::ModelCoefficients line_a_mod_2;
  pcl::ModelCoefficients line_b_mod_2;

  std::vector<float> values_a_case_2;
  values_a_case_2.push_back(0.1000f);
  values_a_case_2.push_back(0.2000f);
  values_a_case_2.push_back(0.3000f);
  values_a_case_2.push_back(0.4000f);

  std::vector<float> values_b_case_2;
  values_b_case_2.push_back(0.1001f);
  values_b_case_2.push_back(0.2001f);
  values_b_case_2.push_back(0.3001f);
  values_b_case_2.push_back(0.4001f);

  line_a_mod_2.values = values_a_case_2;
  line_b_mod_2.values = values_b_case_2;

  Eigen::Vector4f point_mod_2;
  Eigen::Vector4f point_mod_case_2;
  double sqr_mod_case_2 = 1e-1;

  Eigen::Vector4f coeff1 = Eigen::Vector4f::Map (&line_a_mod_2.values[0], line_a_mod_2.values.size ());
  Eigen::Vector4f coeff2 = Eigen::Vector4f::Map (&line_b_mod_2.values[0], line_b_mod_2.values.size ());

  Eigen::Vector4f p1_mod, p2_mod;
  lineToLineSegment (coeff1, coeff2, p1_mod, p2_mod);
  double sqr_mod_act_2 = (p1_mod - p2_mod).squaredNorm ();

  EXPECT_LT(sqr_mod_act_2, sqr_mod_case_2);
  EXPECT_EQ(lineWithLineIntersection (coeff1, coeff2, point_mod_case_2, sqr_mod_case_2),
                      lineWithLineIntersection(line_a_mod_2, line_b_mod_2, point_mod_2, sqr_mod_case_2));
  EXPECT_TRUE(lineWithLineIntersection(line_a_mod_2, line_b_mod_2, point_mod_2, sqr_mod_case_2));
  EXPECT_EQ(point_mod_2, point_mod_case_2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST (PCL, planeWithPlaneIntersection)
{
  //Testing parallel planes
  const int k = 2;
  float x = 1.0f;
  float y = 2.0f;
  float z = 3.0f;
  float w = 4.0f;
  Eigen::Vector4f plane_a;
  plane_a.x() = x;
  plane_a.y() = y;
  plane_a.z() = z;
  plane_a.w() = w;

  EXPECT_EQ(1.0f, plane_a.x());
  EXPECT_EQ(2.0f, plane_a.y());
  EXPECT_EQ(3.0f, plane_a.z());
  EXPECT_EQ(4.0f, plane_a.w());

  Eigen::Vector4f plane_b;
  plane_b.x() = x;
  plane_b.y() = y;
  plane_b.z() = z;
  plane_b.w() = w + k;

  EXPECT_EQ(1.0f, plane_b.x());
  EXPECT_EQ(2.0f, plane_b.y());
  EXPECT_EQ(3.0f, plane_b.z());
  EXPECT_EQ(6.0f, plane_b.w());

  Eigen::VectorXf line;

  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 45));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 90));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 180));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 360));

  plane_b.x() = k * x;
  plane_b.y() = k * y;
  plane_b.z() = k * z;
  plane_b.w() = k * w;

  EXPECT_EQ(2.0f, plane_b.x());
  EXPECT_EQ(4.0f, plane_b.y());
  EXPECT_EQ(6.0f, plane_b.z());
  EXPECT_EQ(8.0f, plane_b.w());

  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 45));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 90));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 180));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 360));

  //overlapping planes
  plane_b.w() = w;
  EXPECT_EQ(4.0f, plane_b.w());

  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 45));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 90));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 180));
  std::cout << std::endl;
  EXPECT_FALSE(planeWithPlaneIntersection(plane_a, plane_b, line, 360));

  //orthogonal planes
  plane_a.x() = 2.0f;
  plane_a.y() = 1.0f;
  plane_a.z() = -5.0f;
  plane_a.w() = 6.0f;
  EXPECT_EQ(2.0f, plane_a.x());
  EXPECT_EQ(1.0f, plane_a.y());
  EXPECT_EQ(-5.0f, plane_a.z());
  EXPECT_EQ(6.0f, plane_a.w());

  plane_b.x() = 2.0f;
  plane_b.y() = 1.0f;
  plane_b.z() = 1.0f;
  plane_b.w() = 7.0f;

  EXPECT_EQ(2.0f, plane_b.x());
  EXPECT_EQ(1.0f, plane_b.y());
  EXPECT_EQ(1.0f, plane_b.z());
  EXPECT_EQ(7.0f, plane_b.w());

  std::cout << std::endl;
  EXPECT_TRUE(planeWithPlaneIntersection(plane_a, plane_b, line, 0));

  //general planes
  plane_a.x() = 1.555f;
  plane_a.y() = 0.894f;
  plane_a.z() = 1.234f;
  plane_a.w() = 3.567f;

  plane_b.x() = 0.743f;
  plane_b.y() = 1.890f;
  plane_b.z() = 6.789f;
  plane_b.w() = 5.432f;

  std::cout << std::endl;
  EXPECT_TRUE(planeWithPlaneIntersection(plane_a, plane_b, line, 0));

}

//* ---[ */
int
main (int argc, char** argv)
{
  testing::InitGoogleTest (&argc, argv);
  return (RUN_ALL_TESTS ());
}
/* ]--- */

