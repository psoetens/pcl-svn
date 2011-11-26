#include <GL/gl.h>
#include <Eigen/Geometry>
#include <pcl/common/common.h>
#include <pcl/io/pcd_io.h>
#include <cfloat>
#include <pcl/visualization/point_cloud_handlers.h>
#include <pcl/visualization/image_viewer.h>
#include <pcl/visualization/window.h>
#include <pcl/console/print.h>
#include <pcl/console/parse.h>
#include <pcl/console/time.h>
#include <vtkPolyDataReader.h>
#include <vtkProp.h>

namespace pcl
{
  namespace simulation
  {
    class OpenGLProp : public vtkProp {
      public:
        static OpenGLProp*
        New ();

        virtual int
        RenderOpaqueGeometry (vtkViewport * viewport);
    };

    int
    OpenGLProp::RenderOpaqueGeometry(vtkViewport * viewport)
    {
      glBegin(GL_TRIANGLES);
        glVertex3f(1.0, 1.0, 0);
        glVertex3f(0.0, 1.0, 0);
        glVertex3f(0.0, 0.0, 0);
      glEnd();

      return 1;
    }
    vtkStandardNewMacro(OpenGLProp);

    class RangeViewer : public pcl::visualization::Window
    {
      public:
        RangeViewer (const std::string& window_name);
        virtual ~RangeViewer ();

        boost::signals2::connection
        registerMouseCallback (boost::function<void (const pcl::visualization::MouseEvent&)> event)
        { return pcl::visualization::Window::registerMouseCallback (event); }

        boost::signals2::connection
        registerKeyboardCallback (boost::function<void (const pcl::visualization::KeyboardEvent&)> event)
        { return pcl::visualization::Window::registerKeyboardCallback (event); }

      private:
        vtkRenderer* renderer_;
        OpenGLProp* prop_;
    };

    RangeViewer::RangeViewer (const std::string& window_name) : pcl::visualization::Window(window_name)
    {
      prop_ = OpenGLProp::New();

      win_->SetSize (500, 500);

      renderer_ = vtkRenderer::New();
      renderer_->AddActor( prop_ );
      renderer_->SetBackground( 0.1, 0.2, 0.4 );
      renderer_->SetViewport(0.0, 0.0, 1.0, 1.0);

      win_->AddRenderer( renderer_ );
    }

    RangeViewer::~RangeViewer ()
    {
      renderer_->Delete();
      prop_->Delete();
    }
  }
}

class RangeVisualization
{
  public:
    RangeVisualization ();

    void
    spin ();

    void
    on_keyboard (const pcl::visualization::KeyboardEvent& event);

    void
    on_mouse (const pcl::visualization::MouseEvent& event);

  private:
    boost::shared_ptr<pcl::simulation::RangeViewer> viewer_;
};

RangeVisualization::RangeVisualization ()
{
  viewer_.reset (new pcl::simulation::RangeViewer ("Range Simulation"));
  viewer_->registerMouseCallback (boost::bind (&RangeVisualization::on_mouse, this, _1));
  viewer_->registerKeyboardCallback (boost::bind (&RangeVisualization::on_keyboard, this, _1));
}

void
RangeVisualization::on_keyboard (const pcl::visualization::KeyboardEvent& event)
{
}

void
RangeVisualization::on_mouse (const pcl::visualization::MouseEvent& event)
{
}

void
RangeVisualization::spin ()
{
  viewer_->spin ();
}

int
main (int argc, char** argv)
{
  RangeVisualization range_viz;
  range_viz.spin ();

  return 0;
}

