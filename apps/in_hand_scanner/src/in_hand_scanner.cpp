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
 *    copyright notice, this list ofc conditions and the following
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

#include <pcl/apps/in_hand_scanner/in_hand_scanner.h>

#include <pcl/common/transforms.h>
#include <pcl/exceptions.h>
#include <pcl/io/openni_grabber.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/apps/in_hand_scanner/custom_interactor_style.h>
#include <pcl/apps/in_hand_scanner/icp.h>
#include <pcl/apps/in_hand_scanner/input_data_processing.h>

////////////////////////////////////////////////////////////////////////////////

pcl::ihs::InHandScanner::InHandScanner (int argc, char** argv)
  : mutex_                 (),
    run_                   (true),

    grabber_               (),
    input_data_processing_ (new InputDataProcessing ()),
    visualizer_            (),

    icp_                   (new ICP ()),
    transformation_        (Transformation::Identity ()),

    new_data_connection_   (),

    cloud_data_draw_       (),
    cloud_model_draw_      (new CloudProcessed ()),
    cloud_model_           (new CloudProcessed ()),

    running_mode_          (RM_UNPROCESSED),
    iteration_             (0),

    draw_crop_box_         (false)
{
  std::cerr << "Initializing the grabber ...\n  ";
  try
  {
    grabber_ = GrabberPtr (new Grabber ());
  }
  catch (const pcl::PCLException& e)
  {
    std::cerr << "ERROR in in_hand_scanner.cpp: " << e.what () << std::endl;
    exit (EXIT_FAILURE);
  }
  std::cerr << "DONE\n";

  // Visualizer
  visualizer_ = PCLVisualizerPtr (new PCLVisualizer (argc, argv, "PCL in-hand scanner", pcl::ihs::CustomInteractorStyle::New ()));

  // TODO: Adapt this to the resolution of the grabbed image
  // grabber_->getDevice ()->get??
  visualizer_->getRenderWindow ()->SetSize (640, 480);
}

////////////////////////////////////////////////////////////////////////////////

pcl::ihs::InHandScanner::~InHandScanner ()
{
  if (grabber_ && grabber_->isRunning ()) grabber_->stop ();
  if (new_data_connection_.connected ())  new_data_connection_.disconnect ();
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::run ()
{
  // Visualizer callbacks
  visualizer_->registerKeyboardCallback (&pcl::ihs::InHandScanner::keyboardCallback, *this);

  // Grabber callbacks
  boost::function <void (const CloudInputConstPtr&)> new_data_cb = boost::bind (&pcl::ihs::InHandScanner::newDataCallback, this, _1);
  new_data_connection_ = grabber_->registerCallback (new_data_cb);

  grabber_->start ();

  // Visualization loop
  while (run_ && !visualizer_->wasStopped ())
  {
    this->calcFPS (visualization_fps_);

    visualizer_->spinOnce ();

    this->drawClouds ();
    this->drawMesh ();
    this->drawCropBox ();
    this->drawFPS ();

    boost::this_thread::sleep (boost::posix_time::microseconds (100));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::quit ()
{
  boost::mutex::scoped_lock lock (mutex_);

  run_ = false;
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::setRunningMode (const RunningMode& mode)
{
  boost::mutex::scoped_lock lock (mutex_);

  running_mode_ = mode;

  switch (running_mode_)
  {
    case RM_UNPROCESSED:
    {
      std::cerr << "Showing the unprocessed input data\n";
      break;
    }
    case RM_PROCESSED:
    {
      std::cerr << "Showing the processed input data\n";
      break;
    }
    case RM_REGISTRATION_CONT:
    {
      std::cerr << "Continuous registration\n";
      break;
    }
    case RM_REGISTRATION_SINGLE:
    {
      std::cerr << "Single registration\n";
      break;
    }
    default:
    {
      std::cerr << "ERROR in in_hand_scanner.cpp: Unknown command!\n";
      exit (EXIT_FAILURE);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::resetRegistration ()
{
  boost::mutex::scoped_lock lock (mutex_);

  running_mode_     = RM_PROCESSED;
  iteration_        = 0;
  transformation_   = Transformation::Identity ();
  cloud_model_draw_ = CloudProcessedPtr (new CloudProcessed ());
  cloud_model_->clear ();
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::newDataCallback (const CloudInputConstPtr& cloud_in)
{
  boost::mutex::scoped_lock lock (mutex_);

  this->calcFPS (computation_fps_);

  if (running_mode_ < RM_UNPROCESSED)
  {
    return;
  }

  // Input data processing
  CloudProcessedPtr cloud_processed = running_mode_ >= RM_PROCESSED ?
                                        input_data_processing_->process (cloud_in) :
                                        input_data_processing_->calculateNormals (cloud_in);

  CloudProcessedPtr cloud_data = cloud_processed;

  // Registration & integration
  if (running_mode_ >= RM_REGISTRATION_CONT)
  {
    if(running_mode_ == RM_REGISTRATION_SINGLE)
    {
      running_mode_ = RM_PROCESSED;
    }

    if (iteration_ == 0)
    {
      transformation_ = Transformation::Identity ();
      cloud_model_    = cloud_processed;
      cloud_data      = CloudProcessedPtr (new CloudProcessed ());
    }
    else
    {
      // Registration
      Transformation T = Transformation::Identity ();
      if (!icp_->findTransformation (cloud_model_, cloud_processed, transformation_, T))
      {
        cloud_data = cloud_processed;
      }
      else
      {
        transformation_ = T;

        // test registration
        pcl::transformPointCloudWithNormals (*cloud_model_, *cloud_model_, T.inverse ().eval ());
        // end test
      }
    }

    ++iteration_;
  } // End if (running_mode_ >= RM_REGISTRATION_CONT)

  // Set the clouds for visualization
  cloud_data_draw_ = cloud_data;
  if (!cloud_model_draw_) cloud_model_draw_ = CloudProcessedPtr (new CloudProcessed ());
  pcl::copyPointCloud (*cloud_model_, *cloud_model_draw_);
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::drawClouds ()
{
  // Get the clouds
  CloudProcessedPtr cloud_data_temp;
  CloudProcessedPtr cloud_model_temp;
  if (mutex_.try_lock ())
  {
    cloud_data_temp.swap (cloud_data_draw_);
    cloud_model_temp.swap (cloud_model_draw_);
    mutex_.unlock ();
  }

  // Draw the clouds
  if (cloud_data_temp)
  {
    pcl::visualization::PointCloudColorHandlerRGBField <PointProcessed> ch (cloud_data_temp);
    if (!visualizer_->updatePointCloud <PointProcessed> (cloud_data_temp, ch, "cloud_data"))
    {
      visualizer_->addPointCloud <PointProcessed> (cloud_data_temp, ch, "cloud_data");
      visualizer_->resetCameraViewpoint ("cloud_data");
    }
  }

  if (cloud_model_temp)
  {
    pcl::visualization::PointCloudColorHandlerRGBField <PointProcessed> ch (cloud_model_temp);
    if (!visualizer_->updatePointCloud <PointProcessed> (cloud_model_temp, ch, "cloud_model"))
    {
      visualizer_->addPointCloud <PointProcessed> (cloud_model_temp, ch, "cloud_model");
      visualizer_->resetCameraViewpoint ("cloud_model");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::drawMesh ()
{

}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::drawCropBox ()
{
  static bool crop_box_added = false;
  if (draw_crop_box_ && !crop_box_added)
  {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    input_data_processing_->getCropBox (x_min, x_max, y_min, y_max, z_min, z_max);
    visualizer_->addCube (x_min, x_max, y_min, y_max, z_min, z_max, 1., 1., 1., "crop_box");
    crop_box_added = true;
  }
  else  if (!draw_crop_box_ && crop_box_added)
  {
    visualizer_->removeShape ("crop_box");
    crop_box_added = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::drawFPS ()
{
  std::string vis_fps ("Visualization: "), comp_fps ("Computation: ");
  bool draw = false;

  if (mutex_.try_lock ())
  {
    vis_fps.append (visualization_fps_.str ()).append (" fps");
    comp_fps.append (computation_fps_.str ()).append (" fps");
    draw = true;
    mutex_.unlock ();
  }

  if (!draw) return;

  if (!visualizer_->updateText (vis_fps, 1., 15., "visualization_fps"))
  {
    visualizer_->addText (vis_fps, 1., 15., "visualization_fps");
  }

  if (!visualizer_->updateText (comp_fps, 1., 30., "computation_fps"))
  {
    visualizer_->addText (comp_fps, 1., 30., "computation_fps");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
pcl::ihs::InHandScanner::keyboardCallback (const pcl::visualization::KeyboardEvent& event, void*)
{
  if(!event.keyDown ())
  {
    return;
  }

  switch (event.getKeyCode ())
  {
    case 'h': case 'H':
    {
      std::cerr << "======================================================================\n"
                << "Help:\n"
                << "----------------------------------------------------------------------\n"
                << "q, ESC: Quit the application\n"
                << "----------------------------------------------------------------------\n"
                << "1     : Shows the unprocessed input data\n"
                << "2     : Shows the processed input data\n"
                << "3     : Registers new data to the first acquired data continuously\n"
                << "4     : Registers new data once and returns to '2'\n"
                << "0     : Reset the registration\n"
                << "======================================================================\n";
      break;
    }
    case   0: // Special key
    {
      break;
    }
    case  27: // ESC
    case 'q': this->quit ();                                 break;
    case '1': this->setRunningMode (RM_UNPROCESSED);         break;
    case '2': this->setRunningMode (RM_PROCESSED);           break;
    case '3': this->setRunningMode (RM_REGISTRATION_CONT);   break;
    case '4': this->setRunningMode (RM_REGISTRATION_SINGLE); break;
    case '0': this->resetRegistration ();                    break;
    default:                                                 break;
  }
}

////////////////////////////////////////////////////////////////////////////////
