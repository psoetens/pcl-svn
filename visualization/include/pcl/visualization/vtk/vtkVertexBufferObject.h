/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPixelBufferObject.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVertexBufferObject - abstracts an OpenGL vertex buffer object.
// .SECTION Description
// Provides low-level access to GPU memory. Used to pass raw data to GPU. 
// The data is uploaded into a vertex buffer.
// .SECTION See Also
// OpenGL Vertex Buffer Object Extension Spec (ARB_vertex_buffer_object):
// http://www.opengl.org/registry/specs/ARB/vertex_buffer_object.txt
// .SECTION Caveats
// Since most GPUs don't support double format all double data is converted to
// float and then uploaded.
// DON'T PLAY WITH IT YET.

#ifndef __vtkVertexBufferObject_h
#define __vtkVertexBufferObject_h

#include <vector>

#include "vtkWeakPointer.h"

#include "vtkgl.h" // Needed for gl data types exposed in API

//class vtkCellArray;
class vtkDataArray;
class vtkObject;
class vtkPoints;
class vtkUnsignedCharArray;
class vtkOpenGLExtensionManager;
class vtkRenderWindow;

class VTK_RENDERING_EXPORT vtkVertexBufferObject : public vtkObject
{
public:
  
  static vtkVertexBufferObject* New();
  vtkTypeMacro(vtkVertexBufferObject, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the context. Context must be a vtkOpenGLRenderWindow.
  // This does not increase the reference count of the
  // context to avoid reference loops.
  // SetContext() may raise an error is the OpenGL context does not support the
  // required OpenGL extensions.
  void SetContext(vtkRenderWindow* context);
  vtkRenderWindow* GetContext();

  //BTX
  // Usage values.
  enum 
  {
    StreamDraw=0,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy,
    NumberOfUsages
  };
  //ETX
  
  // Description:
  // Usage is a performance hint.
  // Valid values are:
  // - StreamDraw specified once by A, used few times S
  // - StreamRead specified once by R, queried a few times by A
  // - StreamCopy specified once by R, used a few times S
  // - StaticDraw specified once by A, used many times S
  // - StaticRead specificed once by R, queried many times by A
  // - StaticCopy specified once by R, used many times S
  // - DynamicDraw respecified repeatedly by A, used many times S
  // - DynamicRead respecified repeatedly by R, queried many times by A
  // - DynamicCopy respecified repeatedly by R, used many times S
  // A: the application
  // S: as the source for GL drawing and image specification commands.
  // R: reading data from the GL
  // Initial value is StaticDraw, as in OpenGL spec.
  vtkGetMacro(Usage, int);
  vtkSetMacro(Usage, int);
  
  bool SetData(vtkPoints *points);
  //bool SetData(vtkCellArray *verts);
  bool SetData(std::vector<unsigned int> *indices);
  bool SetData(vtkUnsignedCharArray *colors);
  bool SetData(vtkDataArray *colors);
  bool SetData(GLvoid* data);

  // Description:
  // Get the size of the data loaded into the GPU. Size is in the number of
  // elements of the uploaded Type.
  vtkGetMacro(Size, unsigned int);

  // Description:
  // Get the size of the data loaded into the GPU. Size is in the number of
  // elements of the uploaded Type.
  vtkGetMacro(Count, unsigned int);

  // Description:
  // Get the openGL buffer handle.
  vtkGetMacro(Handle, unsigned int);

  // Description:
  // Inactivate the buffer.
  void UnBind();

//BTX

  // Description:
  // Make the buffer active.
  void Bind();

  // Description:
  // Allocate the memory. size is in number of bytes. type is a VTK type.
//  void Allocate(unsigned int size, int type);
  
  // Description:
  // Release the memory allocated without destroying the PBO handle.
  void ReleaseMemory();

  // Description:
  // Returns if the context supports the required extensions.
  static bool IsSupported(vtkRenderWindow* renWin);

//ETX
//BTX
protected:
  vtkVertexBufferObject();
  ~vtkVertexBufferObject();

  // Description:
  // Loads all required OpenGL extensions. Must be called every time a new
  // context is set.
  bool LoadRequiredExtensions(vtkOpenGLExtensionManager* mgr);

  // Description:
  // Create the pixel buffer object.
  void CreateBuffer();

  // Description:
  // Destroys the pixel buffer object.
  void DestroyBuffer();

//  int Type;
  int Usage;
  unsigned int Size;
  unsigned int Count;
  unsigned int Handle;
  unsigned int BufferTarget;
  vtkWeakPointer<vtkRenderWindow> Context;

private:
  vtkVertexBufferObject(const vtkVertexBufferObject&); // Not implemented.
  void operator=(const vtkVertexBufferObject&); // Not implemented.
  //ETX
};

#endif


