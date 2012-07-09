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
 */

#include <pcl/visualization/pcl_plotter.h>
#include <pcl/common/common_headers.h>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::PCLPlotter::PCLPlotter (char const *name)
{
  view_ = vtkSmartPointer<vtkContextView>::New ();
  //chart_=vtkSmartPointer<vtkChartXY>::New();
  color_series_ = vtkSmartPointer<vtkColorSeries>::New ();
  
  view_->GetScene ()->AddItem (this);
  
  //initializing default state values
  view_->GetRenderWindow ()->SetWindowName (name);
  win_width_ = 640;
  win_height_ = 480;
  bkg_color_[0] = 1; bkg_color_[1] = 1; bkg_color_[2] = 1;
  current_plot_ = -1;
  color_series_->SetColorScheme (vtkColorSeries::SPECTRUM);
  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (
    double const* array_X, double const* array_Y, 
    unsigned long size, char const * name /* = "Y Axis" */, 
    int type, char const* color)
{
  //updating the current plot ID
  current_plot_++;
  
  //creating a permanent copy of the arrays
  double *permanent_X = new double[size];
  double *permanent_Y = new double[size];
  memcpy(permanent_X, array_X, size*sizeof(double));
  memcpy(permanent_Y, array_Y, size*sizeof(double));
  
  //transforming data to be fed to the vtkChartXY
  VTK_CREATE (vtkTable, table);

  VTK_CREATE (vtkDoubleArray, varray_X);
  varray_X->SetName ("X Axis");
  varray_X->SetArray (permanent_X, size, 1);
  table->AddColumn (varray_X);

  VTK_CREATE (vtkDoubleArray, varray_Y);
  varray_Y->SetName (name);
  varray_Y->SetArray (permanent_Y, size, 1);
  table->AddColumn (varray_Y);

  //adding to chart
  //vtkPlot *line = chart_->AddPlot(vtkChart::LINE);
  vtkPlot *line = this->AddPlot (type);
  line->SetInput (table, 0, 1);
  line->SetWidth (1);

  if (color == NULL)    //color automatically based on the ColorScheme
  {
    vtkColor3ub vcolor = color_series_->GetColorRepeating (current_plot_);
    line->SetColor (vcolor[0], vcolor[1], vcolor[2], 255);
  }
  else                  //add the specific color
    line->SetColor (color[0], color[1], color[2], color[3]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (std::vector<double> const &array_X, std::vector<double> const &array_Y, char const * name /* = "Y Axis" */, int type /* = vtkChart::LINE */, std::vector<char> const &color)
{
  this->addPlotData (&array_X[0], &array_Y[0], static_cast<unsigned long> (array_X.size ()), name, type, (color.size () == 0) ? NULL : &color[0]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (std::vector<std::pair<double, double> > const &plot_data, char const * name /* = "Y Axis" */, int type, std::vector<char> const &color)
{
  double *array_x = new double[plot_data.size ()];
  double *array_y = new double[plot_data.size ()];

  for (unsigned int i = 0; i < plot_data.size (); i++)
  {
    array_x[i] = plot_data[i].first;
    array_y[i] = plot_data[i].second;
  }
  this->addPlotData (array_x, array_y, static_cast<unsigned long> (plot_data.size ()), name, type, (color.size () == 0) ? NULL : &color[0]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (
    PolynomialFunction const & p_function,
    double x_min, double x_max,
    char const *name,
    int num_points,
    std::vector<char> const &color)
{
  std::vector<double> array_x(num_points), array_y(num_points);
  double incr = (x_max - x_min)/num_points;
  
  for (int i = 0; i < num_points; i++)
  {
    double xval = i*incr + x_min;
    array_x[i] = xval;
    array_y[i] = compute(p_function, xval);
  }
  
  this->addPlotData (array_x, array_y, name, vtkChart::LINE, color);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (RationalFunction const & r_function,
                    double x_min, double x_max,
                    char const *name,
                    int num_points,
                    std::vector<char> const &color)
{
  std::vector<double> array_x(num_points), array_y(num_points);
  double incr = (x_max - x_min)/num_points;
  
  for (int i = 0; i < num_points; i++)
  {
    double xval = i*incr + x_min;
    double yval = compute(r_function, xval);
    //if (yval == DBL_MAX) continue; //handling dived by zero 
    
    array_x[i] = xval;
    array_y[i] = yval;
  }
  
  this->addPlotData (array_x, array_y, name, vtkChart::LINE, color);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addPlotData (double (*function)(double),
                    double x_min, double x_max,
                    char const *name,
                    int num_points,
                    std::vector<char> const &color)
{
  std::vector<double> array_x(num_points), array_y(num_points);
  double incr = (x_max - x_min)/num_points;
  
  for (int i = 0; i < num_points; i++)
  {
    double xval = i*incr + x_min;
    array_x[i] = xval;
    array_y[i] = function(xval);
  }
  
  this->addPlotData (array_x, array_y, name, vtkChart::LINE, color);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::addHistogramData (std::vector<double> const& data, int const nbins, char const * name, std::vector<char> const &color)
{
  std::vector<std::pair<double, double> > histogram;
  computeHistogram (data, nbins, histogram);
  this->addPlotData (histogram, name, vtkChart::BAR, color);
  
}

////////////////////////////////HistVizualizer Functions//////////////////////////////////////
bool
pcl::visualization::PCLPlotter::addFeatureHistogram (
    const sensor_msgs::PointCloud2 &cloud, const std::string &field_name, 
    const std::string &id, int win_width, int win_height)
{
  // Get the field
  int field_idx = pcl::getFieldIndex (cloud, field_name);
  if (field_idx == -1)
  {
    PCL_ERROR ("[addFeatureHistogram] Invalid field (%s) given!", field_name.c_str ());
    return (false);
  }

  int hsize = cloud.fields[field_idx].count;
  std::vector<double> array_x (hsize), array_y (hsize);
  
  // Parse the cloud data and store it in the array
  for (int i = 0; i < hsize; ++i)
  {
    array_x[i] = i;
    float data;
    // TODO: replace float with the real data type
    memcpy (&data, &cloud.data[cloud.fields[field_idx].offset + i * sizeof (float)], sizeof (float));
    array_y[i] = data;
  }
  
  this->addPlotData(array_x, array_y, id.c_str(), vtkChart::LINE);
  setWindowSize (win_width, win_height);
  return (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLPlotter::addFeatureHistogram (
    const sensor_msgs::PointCloud2 &cloud, 
    const std::string &field_name, 
    const int index,
    const std::string &id, int win_width, int win_height)
{
  if (index < 0 || index >= static_cast<int> (cloud.width * cloud.height))
  {
    PCL_ERROR ("[addFeatureHistogram] Invalid point index (%d) given!\n", index);
    return (false);
  }
  
  // Get the field
  int field_idx = pcl::getFieldIndex (cloud, field_name);
  if (field_idx == -1)
  {
    PCL_ERROR ("[addFeatureHistogram] Invalid field (%s) given!", field_name.c_str ());
    return (false);
  }

  // Compute the total size of the fields
  unsigned int fsize = 0;
  for (size_t i = 0; i < cloud.fields.size (); ++i)
    fsize += cloud.fields[i].count * pcl::getFieldSize (cloud.fields[i].datatype);
  
  int hsize = cloud.fields[field_idx].count;
  std::vector<double> array_x (hsize), array_y (hsize);
  
  // Parse the cloud data and store it in the array
  for (int i = 0; i < hsize; ++i)
  {
    array_x[i] = i;
    float data;
    // TODO: replace float with the real data type
    memcpy (&data, &cloud.data[index * fsize + cloud.fields[field_idx].offset + i * sizeof (float)], sizeof (float));
    array_y[i] = data;
  }
  
  this->addPlotData(array_x, array_y, id.c_str(), vtkChart::LINE);
  setWindowSize (win_width, win_height);
  return (true);
}

///////////////////end of PCLHistogramVisualizer functions/////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::setColorScheme (int scheme)
{
  color_series_->SetColorScheme (scheme);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
pcl::visualization::PCLPlotter::getColorScheme ()
{
  return (color_series_->GetColorScheme ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::plot ()
{
  view_->GetRenderer ()->SetBackground (bkg_color_[0], bkg_color_[1], bkg_color_[2]);
  view_->GetRenderWindow ()->SetSize (win_width_, win_height_);
  view_->GetInteractor ()->Start ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::setBackgroundColor (const double r, const double g, const double b)
{
  bkg_color_[0] = r;
  bkg_color_[1] = g;
  bkg_color_[2] = b;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::setBackgroundColor (const double color[3])
{
  bkg_color_[0] = color[0];
  bkg_color_[1] = color[1];
  bkg_color_[2] = color[2];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double*
pcl::visualization::PCLPlotter::getBackgroundColor ()
{
  double *bc = new double[3];
  bc[0] = bkg_color_[0];
  bc[1] = bkg_color_[1];
  bc[2] = bkg_color_[2];
  return (bc);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLPlotter::setWindowSize (int w, int h)
{
  win_width_ = w;
  win_height_ = h;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int*
pcl::visualization::PCLPlotter::getWindowSize ()
{
  int *sz = new int[2];
  sz[0] = win_width_;
  sz[1] = win_height_;
  return (sz);
}

///////////////////////IMPORTANT PRIVATE FUNCTIONS///////////////////////////////
void
pcl::visualization::PCLPlotter::computeHistogram (std::vector<double> const &data, int const nbins, std::vector<std::pair<double, double> > &histogram)
{
  //resizing the vector to nbins to store histogram;
  histogram.resize (nbins);

  //find min and max in the data
  double min = data[0], max = data[0];
  for (int i = 1; i < data.size (); i++)
  {
    if (data[i] < min) min = data[i];
    if (data[i] > max) max = data[i];
  }

  //finding the size of each bins
  double size = (max - min) / nbins;

  //fill x values of each bins by bin center
  for (int i = 0; i < nbins; i++)
  {
    histogram[i].first = min + (size * i) + size / 2; //size/2 for the middle of the bins
    histogram[i].second = 0; //initializing the freq to zero
  }

  //fill the freq for each data
  for (int i = 0; i < data.size (); i++)
  {
    int index = int (data[i] - min) / int (size);
    if (index == nbins) index = nbins - 1; //including right boundary
    histogram[index ].second++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double 
pcl::visualization::PCLPlotter::compute (
    pcl::visualization::PCLPlotter::PolynomialFunction const & p_function, 
    double val)

{
  double res = 0;
  for (size_t i = 0; i < p_function.size (); i++)
  {
    res += (p_function[i] * pow (val, static_cast<double> (i)) );
  }
  return (res);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double 
pcl::visualization::PCLPlotter::compute(RationalFunction const & r_function, double val)
{
  PolynomialFunction numerator = r_function.first, denominator = r_function.second;
  
  double dres = this->compute (denominator,val);
  //if (dres == 0) return DBL_MAX;  //return the max possible double value to represent infinity
  
  double nres = this->compute (numerator,val);
  
  return (nres/dres);

}

