/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2009-2012, Willow Garage, Inc.
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
#ifndef PCL_PCL_VISUALIZER_VTK_H_
#define PCL_PCL_VISUALIZER_VTK_H_

#if defined __GNUC__
#  pragma GCC system_header 
#endif

#include <vtkMath.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkPlatonicSolidSource.h>
#include <vtkGeneralTransform.h>
#include <vtkSmartPointer.h>
#include <vtkDataSet.h>
#include <vtkPolygon.h>
#include <vtkUnstructuredGrid.h>
#include <vtkConeSource.h>
#include <vtkDiskSource.h>
#include <vtkPlaneSource.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLineSource.h>
#include <vtkTubeFilter.h>
#include <vtkCubeSource.h>
#include <vtkAxes.h>
#include <vtkFloatArray.h>
#include <vtkAppendPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkCellLocator.h>
#include <vtkPLYReader.h>
#include <vtkTransformFilter.h>
#include <vtkPolyLine.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkDataArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPoints.h>
#include <vtkRendererCollection.h>
#include <vtkLegendScaleActor.h>
#include <vtkPNGWriter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAssemblyPath.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget.h>
#include <vtkBoxWidget2.h>
#include <vtkClipPolyData.h>
#include <vtkPlanes.h>
#include <vtkImageImport.h>
#include <vtkImageViewer.h>
#include <vtkImageFlip.h>
#include <vtkTIFFWriter.h>
#include <vtkBMPWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkXYPlotActor.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>
#include <vtkActor.h>
#include <vtkPolyDataWriter.h>
#include <vtkTextActor.h>
#include <vtkCleanPolyData.h>
#include <vtkRenderer.h>
#if ((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION <= 4))
#  include <pcl/visualization/interactor.h>
#else
#  include <vtkRenderWindowInteractor.h>
#endif

#endif    // PCL_PCL_VISUALIZER_VTK_H_

