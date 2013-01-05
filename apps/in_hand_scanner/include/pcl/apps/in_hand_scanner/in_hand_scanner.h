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

#ifndef PCL_APPS_IN_HAND_SCANNER_IN_HAND_SCANNER_H
#define PCL_APPS_IN_HAND_SCANNER_IN_HAND_SCANNER_H

#include <string>
#include <sstream>
#include <iomanip>

#include <pcl/pcl_exports.h>
#include <pcl/common/time.h>
#include <pcl/apps/in_hand_scanner/boost.h>
#include <pcl/apps/in_hand_scanner/common_types.h>
#include <pcl/apps/in_hand_scanner/opengl_viewer.h>

////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////

namespace pcl
{
  class OpenNIGrabber;

  namespace ihs
  {
    class ICP;
    class InputDataProcessing;
    class Integration;
  } // End namespace ihs
} // End namespace pcl

////////////////////////////////////////////////////////////////////////////////
// InHandScanner
////////////////////////////////////////////////////////////////////////////////

namespace pcl
{
  namespace ihs
  {
    /** \brief
      * \todo Add Documentation
      */
    class PCL_EXPORTS InHandScanner : public pcl::ihs::OpenGLViewer
    {
      Q_OBJECT

      public:

        typedef pcl::ihs::OpenGLViewer  Base;
        typedef pcl::ihs::InHandScanner Self;

        /** \brief Switch between different branches of the scanning pipeline. */
        typedef enum RunningMode
        {
          RM_SHOW_MODEL          = 0, /**< Show the model shape (if one is available). */
          RM_UNPROCESSED         = 1, /**< Shows the unprocessed input data. */
          RM_PROCESSED           = 2, /**< Shows the processed input data. */
          RM_REGISTRATION_CONT   = 3, /**< Registers new data to the first acquired data continuously. */
          RM_REGISTRATION_SINGLE = 4  /**< Registers new data once and returns to showing the processed data. */
        } RunningMode;

        /** \brief Constructor. */
        explicit InHandScanner (Base* parent=0);

        /** \brief Destructor. */
        ~InHandScanner ();

        /** \see http://doc.qt.digia.com/qt/qwidget.html#minimumSizeHint-prop */
        virtual QSize
        minimumSizeHint () const;

        /** \see http://doc.qt.digia.com/qt/qwidget.html#sizeHint-prop */
        virtual QSize
        sizeHint () const;

     public slots:

        /** \brief Start the grabber (enables the scanning pipeline). */
        void
        startGrabber ();

        /** \brief Set which branches of the scanning pipeline is executed. */
        void
        setRunningMode (const RunningMode& mode);

        /** \brief Reset the scanning pipeline. */
        void
        reset ();

      private:

        typedef pcl::PointXYZRGBA              PointXYZRGBA;
        typedef pcl::PointCloud <PointXYZRGBA> CloudXYZRGBA;
        typedef CloudXYZRGBA::Ptr              CloudXYZRGBAPtr;
        typedef CloudXYZRGBA::ConstPtr         CloudXYZRGBAConstPtr;

        typedef pcl::PointXYZRGBNormal              PointXYZRGBNormal;
        typedef pcl::PointCloud <PointXYZRGBNormal> CloudXYZRGBNormal;
        typedef CloudXYZRGBNormal::Ptr              CloudXYZRGBNormalPtr;
        typedef CloudXYZRGBNormal::ConstPtr         CloudXYZRGBNormalConstPtr;

        typedef pcl::ihs::PointIHS         PointIHS;
        typedef pcl::ihs::CloudIHS         CloudIHS;
        typedef pcl::ihs::CloudIHSPtr      CloudIHSPtr;
        typedef pcl::ihs::CloudIHSConstPtr CloudIHSConstPtr;

        typedef pcl::ihs::Mesh         Mesh;
        typedef pcl::ihs::MeshPtr      MeshPtr;
        typedef pcl::ihs::MeshConstPtr MeshConstPtr;

        typedef pcl::OpenNIGrabber                Grabber;
        typedef boost::shared_ptr <Grabber>       GrabberPtr;
        typedef boost::shared_ptr <const Grabber> GrabberConstPtr;

        typedef pcl::ihs::InputDataProcessing                 InputDataProcessing;
        typedef boost::shared_ptr <InputDataProcessing>       InputDataProcessingPtr;
        typedef boost::shared_ptr <const InputDataProcessing> InputDataProcessingConstPtr;

        typedef pcl::ihs::ICP                 ICP;
        typedef boost::shared_ptr <ICP>       ICPPtr;
        typedef boost::shared_ptr <const ICP> ICPConstPtr;

        typedef pcl::ihs::Integration                 Integration;
        typedef boost::shared_ptr <Integration>       IntegrationPtr;
        typedef boost::shared_ptr <const Integration> IntegrationConstPtr;

        /** \brief Please have a look at the documentation of calcFPS. */
        class FPS
        {
          public:

            FPS () : fps_ (0.) {}

            inline double& value ()       {return (fps_);}
            inline double  value () const {return (fps_);}

            inline std::string
            str () const
            {
              std::stringstream ss;
              ss << std::setprecision (1) << std::fixed << fps_;
              return (ss.str ());
            }

          protected:

            ~FPS () {}

          private:

            double fps_;
        };

        /** \brief Helper object for the computation thread. Please have a look at the documentation of calcFPS. */
        class ComputationFPS : public FPS
        {
          public:
            ComputationFPS () : FPS () {}
            ~ComputationFPS () {}
        };

        /** \brief Helper object for the visualization thread. Please have a look at the documentation of calcFPS. */
        class VisualizationFPS : public FPS
        {
          public:
            VisualizationFPS () : FPS () {}
            ~VisualizationFPS () {}
        };

        /** Measures the performance of the current thread (selected by passing the corresponding 'fps' helper object). The resulting value is stored in the fps object. */
        template <class FPS> void
        calcFPS (FPS& fps) const
        {
          static pcl::StopWatch sw;
          static unsigned int count = 0;

          ++count;
          if (sw.getTimeSeconds () >= 1.)
          {
            fps.value () = static_cast <double> (count) / sw.getTimeSeconds ();
            count = 0;
            sw.reset ();
          }
        }

        /** \brief Called when new data arries from the grabber. The grabbing - registration - integration pipeline is implemented here. */
        void
        newDataCallback (const CloudXYZRGBAConstPtr& cloud_in);

        /** \see http://doc.qt.digia.com/qt/qwidget.html#paintEvent
          * \see http://doc.qt.digia.com/qt/opengl-overpainting.html
          */
        void
        paintEvent (QPaintEvent* event);

        /** \brief Draw text over the opengl scene.
          * \see http://doc.qt.digia.com/qt/opengl-overpainting.html
          */
        void
        drawText ();

        /** \brief Actual implementeation of startGrabber (needed so it can be run in a different thread and doesn't block the application when starting up). */
        void
        startGrabberImpl ();

        /** \see http://doc.qt.digia.com/qt/qwidget.html#keyPressEvent */
        void
        keyPressEvent (QKeyEvent* event);

        ////////////////////////////////////////////////////////////////////////
        // Members
        ////////////////////////////////////////////////////////////////////////

        /** \brief Synchronization. */
        boost::mutex mutex_ihs_;

        /** \brief Please have a look at the documentation of ComputationFPS. */
        ComputationFPS computation_fps_;

        /** \brief Please have a look at the documentation of VisualizationFPS. */
        VisualizationFPS visualization_fps_;

        /** \brief Set to true if the scanning pipeline should be reset. */
        bool reset_;

        /** \brief Switch between different branches of the scanning pipeline. */
        RunningMode running_mode_;

        /** \brief The iteration of the scanning pipeline (grab - register - integrate). */
        unsigned int iteration_;

        /** \brief Used to get new data from the sensor. */
        GrabberPtr grabber_;

        /** \brief This variable is true if the grabber is starting. */
        bool starting_grabber_;

        /** \brief Connection of the grabber signal with the data processing thread. */
        boost::signals2::connection new_data_connection_;

        /** \brief Processes the data from the sensor. Output is input to the registration. */
        InputDataProcessingPtr input_data_processing_;

        /** \brief Registration (Iterative Closest Point). */
        ICPPtr icp_;

        /** \brief Transformation that brings the data cloud into model coordinates. */
        Eigen::Matrix4f transformation_;

        /** \brief Integrate the data cloud into a common model (model cloud). */
        IntegrationPtr integration_;

        /** \brief Model to which new data is registered to (stored as a mesh). */
        MeshPtr mesh_model_;

        /** \brief Prevent the application to crash while closing. */
        bool destructor_called_;

      public:

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
  } // End namespace ihs
} // End namespace pcl

#endif // PCL_APPS_IN_HAND_SCANNER_IN_HAND_SCANNER_H
