#include <boost/thread.hpp>
#include <boost/format.hpp>

#include <QApplication>

#include "proctor/detector.h"
#include "proctor/detector_visualizer.h"
#include "proctor/proctor.h"
#include "proctor/scanning_model_source.h"
#include "proctor/primitive_model_source.h"

#include "proctor/basic_proposer.h"
#include "proctor/registration_proposer.h"
#include "proctor/hough_proposer.h"

#include "proctor/uniform_sampling_wrapper.h"
#include "proctor/harris_wrapper.h"

#include "proctor/fpfh_wrapper.h"
#include "proctor/shot_wrapper.h"
#include "proctor/si_wrapper.h"
#include "proctor/3dsc_wrapper.h"
#include "proctor/feature_wrapper.h"

#include <pcl/features/feature.h>
#include <pcl/features/fpfh.h>

#include <Eigen/Dense>

using pcl::proctor::Detector;
using pcl::proctor::Proctor;
using pcl::proctor::ScanningModelSource;
using pcl::proctor::PrimitiveModelSource;

using pcl::proctor::Proposer;
using pcl::proctor::BasicProposer;
using pcl::proctor::RegistrationProposer;
using pcl::proctor::HoughProposer;

using pcl::proctor::KeypointWrapper;
using pcl::proctor::UniformSamplingWrapper;
using pcl::proctor::HarrisWrapper;

using pcl::proctor::FeatureWrapper;
using pcl::proctor::FPFHWrapper;
using pcl::proctor::SHOTWrapper;
using pcl::proctor::SIWrapper;
using pcl::proctor::ShapeContextWrapper;

struct run_proctor
{
  DetectorVisualizer *vis_;

  run_proctor()
  {
    vis_ = NULL;
  }

  run_proctor(DetectorVisualizer &vis)
  {
    vis_ = &vis;
  }

  void operator()()
  {
    //unsigned int model_seed = 2;
    unsigned int test_seed = 0; //time(NULL);

    // Gets rid of warnings
    //pcl::console::setVerbosityLevel(pcl::console::L_ALWAYS);

    std::vector<FeatureWrapper::Ptr> features;
    FeatureWrapper::Ptr fpfh_wrap (new FPFHWrapper);
    features.push_back(fpfh_wrap);
    FeatureWrapper::Ptr shot_wrap (new SHOTWrapper);
    //features.push_back(shot_wrap);
    FeatureWrapper::Ptr si_wrap (new SIWrapper);
    //features.push_back(si_wrap);
    FeatureWrapper::Ptr shape_context_wrapper (new ShapeContextWrapper);
    //features.push_back(shape_context_wrapper);

    std::vector<KeypointWrapper::Ptr> keypoints;
    KeypointWrapper::Ptr us_wrap (new UniformSamplingWrapper);
    keypoints.push_back(us_wrap);
    KeypointWrapper::Ptr harris_wrap (new HarrisWrapper);
    //keypoints.push_back(harris_wrap);

    for (unsigned int i = 0; i < features.size(); i++)
    {
      for (unsigned int j = 0; j < keypoints.size(); j++)
      {
        //for (int num_x = 1; num_x < 10; num_x++)
        //{
          //for (int num_angles = 1; num_angles < 10; num_angles++)
          //{

            // Configure Detector
            Detector detector;
            
            // Enable visualization if available
            if (vis_)
              detector.enableVisualization(vis_);

            // Get current parameters
            FeatureWrapper::Ptr feature = features[i];
            KeypointWrapper::Ptr keypoint = keypoints[j];

            std::cout << boost::format("Evaluating with feature: %|30t| %s") % feature->name_ << std::endl;
            std::cout << boost::format("Evaluating with keypoint: %|30t| %s") % keypoint->name_ << std::endl;

            std::vector<Proposer::Ptr> proposers;
            //proposers.push_back(BasicProposer::Ptr(new BasicProposer));
            //proposers.push_back(RegistrationProposer::Ptr(new RegistrationProposer));
            HoughProposer::Ptr hough(new HoughProposer(&detector));
            hough->num_angles_ = 60;
            hough->bins_ = 20;

            proposers.push_back(hough);
            detector.setProposers(proposers);

            detector.setKeypointWrapper(keypoint);

            detector.setFeatureEstimator(feature);

            // Configure Proctor
            Proctor proctor;

            ScanningModelSource model_source("princeton", "/home/justin/Documents/benchmark/db");
            model_source.loadModels();

            //PrimitiveModelSource model_source("primitive");
            //model_source.loadModels();

            proctor.setModelSource(&model_source);
            proctor.train(detector);
            proctor.test(detector, test_seed);
            //double correct = proctor.test(detector, test_seed);
            //outputFile << "Num Angles: " << num_angles * 5 << endl;
            //outputFile << "Num Bins: " << 10 << endl;
            proctor.printResults(detector);
            //outputFile << "Num Correct: " << correct  << endl;
            //outputFile << endl;
          //}
        //}
      }
    }
  }
};

int main(int argc, char **argv)
{

  //if (argc >= 2)
  //{
    //model_seed = atoi(argv[1]);
  //}

  //if (argc >= 3)
  //{
    //test_seed = atoi(argv[2]);
  //}

  bool enable_vis = false;
  if (enable_vis)
  {
    QApplication app (argc, argv); 

    DetectorVisualizer v;
    v.show ();

    run_proctor x(v);
    boost::thread proctor_thread(x);

    return (app.exec ());
  }
  else
  {
    run_proctor no_vis;
    boost::thread proctor_thread(no_vis);
    proctor_thread.join();

    return 0;
  }
}

