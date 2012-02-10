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
 *
 */

#ifndef PCL_COMMON_CONVOLUTION_H_
#define PCL_COMMON_CONVOLUTION_H_

#include <Eigen/Core>
#include <pcl/common/point_operators.h>
#include <pcl/common/spring.h>
#include <pcl/exceptions.h>

namespace pcl
{
  namespace common
  {
    /** Abstract convolution class does not perform any computation but handles
      * side operations.
      *
      * Convolution is a mathematical operation on two functions f and g, 
      * producing a third function that is typically viewed as a modified 
      * version of one of the original functions.
      * see http://en.wikipedia.org/wiki/Convolution.
      *
      * The class provides rows, column and separate convolving operations
      * of a point cloud.
      * Columns and separate convolution is only allowed on organised 
      * point clouds.
      *
      * When convolving, computing the rows and cols elements at 1/2 kernel 
      * width distance from the borders is not defined. We allow for 3 
      * policies:
      * - Ignoring: elements at special locations are filled with zero 
      * (default behaviour)
      * - Mirroring: the original point cloud is extending by 1/2 kernel 
      * width amount mirroring borders
      * - Duplicating: the original point cloud is extending by 1/2 kernel 
      * width amount using duplicating the top and bottom rows and/or left
      * and right columns.
      * 
      * After convolving the original point cloud is shrinked to the 
      * original size.
      *
      * \author Nizar Sallem
      * \ingroup common
      */
    template <typename PointIn, typename PointOut>
    class AbstractConvolution
    {
      public:
        typedef typename pcl::PointCloud<PointIn> PointCloudIn;
        typedef typename PointCloudIn::Ptr PointCloudInPtr;
        typedef typename PointCloudIn::ConstPtr PointCloudInConstPtr;
        typedef typename pcl::PointCloud<PointOut> PointCloudOut;
        typedef typename PointCloudOut::Ptr PointCloudOutPtr;
        typedef typename PointCloudOut::ConstPtr PointCloudOutConstPtr;

        /// The borders policy available
        enum BORDERS_POLICY { IGNORE = -1, MIRROR = 0, DUPLICATE = 1};
        /// Constructor
        AbstractConvolution ()
          : borders_policy_ (IGNORE)
          , convolve_direction_ (-1)
          , distance_threshold_ (std::numeric_limits<float>::infinity ())
        {}
        /// Empty destructor
        virtual ~AbstractConvolution () {}
        /// Set the borders policy
        void 
        setBordersPolicy (int policy) { borders_policy_ = policy; }
        /// Get the borders policy
        int getBordersPolicy () { return (borders_policy_); }
        /** Convolve point cloud with an horizontal kernel along rows 
          * then vertical kernel along columns : convolve separately.
          * \param[in] h_kernel kernel for convolving rows
          * \param[in] v_kernel kernel for convolving columns
          * \param[out] out the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        virtual void
        convolve (const Eigen::ArrayXf& h_kernel, 
                  const Eigen::ArrayXf& v_kernel,
                  PointCloudOut& output) = 0;
        /** Convolve point cloud with an same kernel along rows and columns separately.
          * \param[in] kernel kernel for convolving rows and columns
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        virtual void
        convolve (PointCloudOut& output) = 0;
        /** Convolve a float image rows by a given kernel.
          * \param[in] kernel convolution kernel
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolveRows (PointCloudOut& output)
        {
          try
          {
            convolve_direction_ = HORIZONTAL;
            initCompute (output);
            convolve_rows (output);
            deinitCompute (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolveRows] init failed " << e.what ());
          }
        }
        /** Convolve a float image columns by a given kernel.
          * \param[in] kernel convolution kernel
          * \param[out] output the convolved image
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolveCols (PointCloudOut& output)
        {
          try
          {
            convolve_direction_ = VERTICAL;
            initCompute (output);
            convolve_cols (output);
            deinitCompute (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolveCols] init failed " << e.what ());
          }          
        } 
        /** \brief Provide a pointer to the input dataset
          * \param cloud the const boost shared pointer to a PointCloud message
          * \remark Will perform a deep copy
          */
        inline void
        setInputCloud (const PointCloudInConstPtr& cloud)
        {
          input_.reset (new PointCloudIn (*cloud));
        }

        inline void 
        setKernel (const Eigen::ArrayXf& kernel)
        {
          kernel_ = kernel;
        }
        /** \remark this is critical so please read it carefully.
          * In 3D the next point in (u,v) coordinate can be really far so a distance 
          * threshold is used to keep us from ghost points.
          * The value you set here is strongly related to the sensor. A good value for
          * kinect data is 0.001 \default is std::numeric<float>::infinity ()
          * \param[in] threshold: maximum allowed distance between 2 juxtaposed points
          */
        inline void
        setDistanceThreshold (const float& threshold)
        {
          distance_threshold_ = threshold;
        }
        ///\return the distance threshold
        inline const float &
        getDistanceThreshold () const
        {
          return (distance_threshold_);
        }

      protected:
        /// direction of the expansion
        enum DIRECTION { HORIZONTAL = 0, VERTICAL };
        /** init compute is an internal method called before computation
          * \param[in] kernel convolution kernel to be used
          * \throw pcl::InitFailedException
          */
        inline void
        initCompute (PointCloudOut& output);
        /** terminate computation shrinking the input point cloud if 
          * necessary.
          */
        void
        deinitCompute (PointCloudOut& output);
        /// convolve rows and ignore borders
        virtual void
        convolve_rows (PointCloudOut& output) = 0;
        /// convolve cols and ignore borders
        virtual void
        convolve_cols (PointCloudOut& output) = 0;
        /// Border policy
        int borders_policy_;
        /// Convolving direction
        int convolve_direction_;
        /// Threshold distance between adjacent points
        float distance_threshold_;
        /// Pointer to the input cloud
        PointCloudInPtr input_;
        /// Pointer to the input cloud
        PointCloudOutPtr output_;
        /// convolution kernel
        Eigen::ArrayXf kernel_;
    };

    /** Class Convolution
      * Performs convolution on point clouds.
      *
      * \author Nizar Sallem
      * \ingroup common
      */
    template <typename PointT>
    class Convolution : public AbstractConvolution<PointT, PointT>
    {
      public:
        typedef AbstractConvolution<PointT, PointT> ConvolutionBase;
        typedef typename PointCloud<PointT>::Ptr PointCloudPtr;

        using ConvolutionBase::setInputCloud;
        using ConvolutionBase::setKernel;
        using ConvolutionBase::input_;
        using ConvolutionBase::kernel_;
        using ConvolutionBase::distance_threshold_;

        Convolution () : ConvolutionBase () {}
        
        /** Convolve point cloud with an horizontal kernel along rows 
          * then vertical kernel along columns : convolve separately.
          * \param[in] h_kernel kernel for convolving rows
          * \param[in] v_kernel kernel for convolving columns
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolve (const Eigen::ArrayXf& h_kernel, 
                  const Eigen::ArrayXf& v_kernel,
                  PointCloud<PointT>& output)
        {
          try
          {
            PointCloudPtr tmp; 
            tmp.reset (new PointCloud<PointT> ());
            setKernel (h_kernel);
            convolveRows (*tmp);
            setInputCloud (tmp);
            setKernel (v_kernel);
            convolveCols (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolve] init failed " << e.what ());
          }
        }
        /** Convolve point cloud with same kernel along rows and columns separately.
          * \param[in] kernel kernel for convolving rows and columns
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolve (PointCloud<PointT>& output)
        {
          try
          {
            PointCloudPtr tmp;
            tmp.reset (new PointCloud<PointT> ());
            convolveRows (*tmp);
            setInputCloud (tmp);
            convolveCols (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolve] init failed " << e.what ());
          }
        }

      protected:
        /// convolve rows and ignore borders
        void
        convolve_rows (PointCloud<PointT>& output);
        /// convolve cols and ignore borders
        void
        convolve_cols (PointCloud<PointT>& output);
    };

    /** Class ConvolutionWithTransform
      * Applies convolution with different types of input point type and 
      * output point type by the mean of a conversion structure.
      * Conversion structure must provide:
      * - 2 typedefs:
      *   - PointIn input point type
      *   - PointOut output point type
      * - 2 methodes:
      *   - inline PointOut operator() () that returns a "zero" PointOut
      *   - inline PointOut operator() (const PointIn& in) that returns a 
      *     PointOut computed from PointIn structure.
      * Have a look at pcl/common/point_types.h for examples.
      *
      * \author Nizar Sallem
      * \ingroup common
      */
    template <typename Conversion>
    class ConvolutionWithTransform
      : public AbstractConvolution<typename Conversion::PointIn, typename Conversion::PointOut>
    {
      public:
        typedef AbstractConvolution<typename Conversion::PointIn, 
                                    typename Conversion::PointOut> ConvolutionBase;
        typedef typename ConvolutionBase::PointCloudOut PointCloudOut;
        typedef typename ConvolutionBase::PointCloudOutPtr PointCloudOutPtr;

        using ConvolutionBase::borders_policy_;
        using ConvolutionBase::setInputCloud;
        using ConvolutionBase::convolveRows;
        using ConvolutionBase::setKernel;
        using ConvolutionBase::input_;
        using ConvolutionBase::kernel_;
        using ConvolutionBase::distance_threshold_;
        
        ConvolutionWithTransform () : ConvolutionBase () {}

        /** Convolve point cloud with an horizontal kernel along rows 
          * then vertical kernel along columns : convolve separately.
          * \param[in] h_kernel kernel for convolving rows
          * \param[in] v_kernel kernel for convolving columns
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolve (const Eigen::ArrayXf& h_kernel, 
                  const Eigen::ArrayXf& v_kernel,
                  PointCloudOut& output)
        {
          try
          {
            PointCloudOutPtr tmp; 
            tmp.reset (new PointCloudOut ());
            setKernel (h_kernel);
            convolveRows (*tmp);
            Convolution<typename Conversion::PointOut> convolution;
            convolution.setInputCloud (tmp);
            convolution.setKernel (v_kernel);
            convolution.setBordersPolicy (borders_policy_);
            convolution.convolveCols (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolve] init failed " << e.what ());
          }
        }
        /** Convolve point cloud with an same kernel along rows and columns separately.
          * \param[in] kernel kernel for convolving rows and columns
          * \param[out] output the convolved cloud
          * \note if output doesn't fit in input i.e. output.rows () < input.rows () or
          * output.cols () < input.cols () then output is resized to input sizes.
          */
        inline void
        convolve (PointCloudOut& output)
        {
          try
          {
            PointCloudOutPtr tmp; 
            tmp.reset (new PointCloudOut ());
            convolveRows (*tmp);
            Convolution<typename Conversion::PointOut> convolution;
            convolution.setInputCloud (tmp);
            convolution.setKernel (kernel_);
            convolution.setBordersPolicy (borders_policy_);
            convolution.convolveCols (output);
          }
          catch (InitFailedException& e)
          {
            PCL_THROW_EXCEPTION (InitFailedException,
                                 "[pcl::common::Convolution::convolve] init failed " << e.what ());
          }
        }
        
      protected:
        /// convolve rows and ignore borders
        void
        convolve_rows (PointCloudOut& output);
        /// convolve cols and ignore borders
        void
        convolve_cols (PointCloudOut& output);

      private:
        Conversion transform_;
    };
  }
}

#include <pcl/common/impl/convolution.hpp>

#endif
