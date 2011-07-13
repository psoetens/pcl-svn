#include <pcl/visualization/window.h>
#include <vtkCallbackCommand.h>
#include <X11/X.h>
#include <vtkObject.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <pcl/visualization/keyboard_event.h>
#include <pcl/visualization/mouse_event.h>
#include <vtkRenderWindow.h>
#include <vtk-5.4/vtkRenderWindow.h>
#include <pcl/common/time.h>
#include <pcl/visualization/interactor.h>


pcl::visualization::Window::Window (const std::string& window_name)
: window_ (vtkRenderWindow::New())
, interactor_ (PCLVisualizerInteractor::New())
, mouse_command_ ( vtkCallbackCommand::New())
, keyboard_command_ ( vtkCallbackCommand::New())
{
  mouse_command_->SetClientData (this);
  mouse_command_->SetCallback (Window::MouseCallback);
  
  keyboard_command_->SetClientData (this);
  keyboard_command_->SetCallback (Window::KeyboardCallback);
  
  window_->SetWindowName(window_name.c_str ());
  window_->SetInteractor( interactor_ );
  interactor_->SetRenderWindow (window_);
  interactor_->SetDesiredUpdateRate (30.0);
  // Initialize and create timer
  interactor_->Initialize ();
  //interactor_->CreateRepeatingTimer (5000L);
  interactor_->timer_id_ = interactor_->CreateRepeatingTimer (5000L);
  
  exit_main_loop_timer_callback_ = vtkSmartPointer<ExitMainLoopTimerCallback>::New ();
  exit_main_loop_timer_callback_->window_ = this;
  exit_main_loop_timer_callback_->right_timer_id = -1;
  interactor_->AddObserver (vtkCommand::TimerEvent, exit_main_loop_timer_callback_);

  exit_callback_ = vtkSmartPointer<ExitCallback>::New ();
  exit_callback_->window_ = this;
  interactor_->AddObserver (vtkCommand::ExitEvent, exit_callback_);

  resetStoppedFlag ();
}

void pcl::visualization::Window::spin ()
{
  resetStoppedFlag ();
  // Render the window before we start the interactor
  window_->Render();
  interactor_->Start ();
}

void
pcl::visualization::Window::spinOnce (int time, bool force_redraw)
{
  resetStoppedFlag ();

  if (time <= 0)
    time = 1;
  
  if (force_redraw)
  {
    interactor_->Render ();
    exit_main_loop_timer_callback_->right_timer_id = interactor_->CreateRepeatingTimer (time);
    interactor_->Start ();
    interactor_->DestroyTimer (exit_main_loop_timer_callback_->right_timer_id);
    return;
  }
  
  DO_EVERY(1.0/interactor_->GetDesiredUpdateRate (),
    interactor_->Render ();
    exit_main_loop_timer_callback_->right_timer_id = interactor_->CreateRepeatingTimer (time);
    interactor_->Start ();
    interactor_->DestroyTimer (exit_main_loop_timer_callback_->right_timer_id);
  );
}

boost::signals2::connection pcl::visualization::Window::registerMouseCallback (boost::function<void (const pcl::visualization::MouseEvent&)> callback)
{
  // just add observer at first time when a callback is registered
  if (mouse_signal_.empty())
  {
    interactor_->AddObserver(vtkCommand::MouseMoveEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MiddleButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MiddleButtonReleaseEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MouseWheelBackwardEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MouseWheelForwardEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::LeftButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::LeftButtonReleaseEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::RightButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::RightButtonReleaseEvent, mouse_command_);
  }
  return mouse_signal_.connect(callback);
}

boost::signals2::connection pcl::visualization::Window::registerKeyboardCallback (boost::function<void (const pcl::visualization::KeyboardEvent&)> callback)
{
  // just add observer at first time when a callback is registered
  if (keyboard_signal_.empty())
  {
    interactor_->AddObserver(vtkCommand::KeyPressEvent, keyboard_command_);
    interactor_->AddObserver(vtkCommand::KeyReleaseEvent, keyboard_command_);
  }
  
  return keyboard_signal_.connect(callback);
}

void pcl::visualization::Window::emitMouseEvent (unsigned long event_id)
{
  int x,y;
  interactor_->GetMousePosition (&x, &y);
  MouseEvent event (MouseEvent::MouseMove, MouseEvent::NoButton, x, y, interactor_->GetAltKey (), interactor_->GetControlKey (), interactor_->GetShiftKey ());
  bool repeat = false;
  switch (event_id)
  {
    case vtkCommand::MouseMoveEvent :
      event.setType(MouseEvent::MouseMove);
      break;
    
    case vtkCommand::LeftButtonPressEvent :
      event.setButton(MouseEvent::LeftButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::LeftButtonReleaseEvent :
      event.setButton(MouseEvent::LeftButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
    case vtkCommand::RightButtonPressEvent :
      event.setButton(MouseEvent::RightButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::RightButtonReleaseEvent :
      event.setButton(MouseEvent::RightButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
    case vtkCommand::MiddleButtonPressEvent :
      event.setButton(MouseEvent::MiddleButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::MiddleButtonReleaseEvent :
      event.setButton(MouseEvent::MiddleButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
      case vtkCommand::MouseWheelBackwardEvent :
      event.setButton(MouseEvent::VScroll);
      event.setType(MouseEvent::MouseScrollDown);
      if (interactor_->GetRepeatCount () != 0)
        repeat = true;
      break;
      
    case vtkCommand::MouseWheelForwardEvent :
      event.setButton(MouseEvent::VScroll);
      event.setType(MouseEvent::MouseScrollUp);
      if (interactor_->GetRepeatCount () != 0)
        repeat = true;
      break;
    default:
      return;
  }
  
  mouse_signal_ (event);
  if (repeat)
    mouse_signal_ (event);
}

void pcl::visualization::Window::emitKeyboardEvent (unsigned long event_id)
{
  KeyboardEvent event (bool(event_id == vtkCommand::KeyPressEvent), interactor_->GetKeySym (), interactor_->GetKeyCode (), interactor_->GetAltKey (), interactor_->GetControlKey (), interactor_->GetShiftKey ());
  keyboard_signal_ (event);
}

void pcl::visualization::Window::MouseCallback(vtkObject*, unsigned long eid, void* clientdata, void* calldata)
{
  Window* window = reinterpret_cast<Window*> (clientdata);
  window->emitMouseEvent (eid);
}

void pcl::visualization::Window::KeyboardCallback(vtkObject*, unsigned long eid, void* clientdata, void* calldata)
{
  Window* window = reinterpret_cast<Window*> (clientdata);
  window->emitKeyboardEvent (eid);
}
