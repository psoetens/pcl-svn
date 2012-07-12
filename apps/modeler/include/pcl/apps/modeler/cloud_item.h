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
 *
 */
#ifndef PCL_MODELER_CLOUD_ITEM_H_
#define PCL_MODELER_CLOUD_ITEM_H_

#include <pcl/apps/modeler/geometry_item.h>
#include <pcl/visualization/point_cloud_handlers.h>

namespace pcl
{
  namespace modeler
  {
    class MainWindow;

    class CloudItem : public GeometryItem
    {
      public:
        typedef boost::shared_ptr<CloudItem> Ptr;
        typedef boost::shared_ptr<const CloudItem> ConstPtr;

        CloudItem (MainWindow* main_window, const Eigen::Vector4f& sensor_origin = Eigen::Vector4f (0, 0, 0, 0),
          const Eigen::Quaternion<float>& sensor_orientation = Eigen::Quaternion<float> (1, 0, 0 ,0));

        virtual ~CloudItem ();

      protected:
        virtual void
        prepareContextMenu(QMenu* menu) const;

        virtual void
        initHandlers();

        virtual bool
        createActor();

      private:
        /** \brief Internal method. Convert origin and orientation to vtkMatrix4x4.
          * \param[in] origin the point cloud origin
          * \param[in] orientation the point cloud orientation
          * \param[out] vtk_matrix the resultant VTK 4x4 matrix
          */
        static void
        convertToVtkMatrix (const Eigen::Vector4f &origin,
                            const Eigen::Quaternion<float> &orientation,
                            vtkSmartPointer<vtkMatrix4x4> &vtk_matrix);

        /** \brief Internal method. Converts a PCL templated PointCloud object to a vtk polydata object.
          * \param[in] geometry_handler the geometry handler object used to extract the XYZ data
          * \param[out] polydata the resultant polydata containing the cloud
          * \param[out] initcells a list of cell indices used for the conversion. This can be set once and then passed
          * around to speed up the conversion.
          */
        static void
        convertPointCloudToVTKPolyData (const GeometryHandlerConstPtr &geometry_handler,
                                        vtkSmartPointer<vtkPolyData> &polydata,
                                        vtkSmartPointer<vtkIdTypeArray> &initcells);

        /** \brief Internal method. Updates a set of cells (vtkIdTypeArray) if the number of points in a cloud changes
          * \param[out] cells the vtkIdTypeArray object (set of cells) to update
          * \param[out] initcells a previously saved set of cells. If the number of points in the current cloud is
          * higher than the number of cells in \a cells, and initcells contains enough data, then a copy from it
          * will be made instead of regenerating the entire array.
          * \param[in] nr_points the number of points in the new cloud. This dictates how many cells we need to
          * generate
          */
        static void
        updateCells (vtkSmartPointer<vtkIdTypeArray> &cells,
                     vtkSmartPointer<vtkIdTypeArray> &initcells,
                     vtkIdType nr_points);

        /** \brief Internal method. Creates a vtk actor from a vtk polydata object.
          * \param[in] data the vtk polydata object to create an actor for
          * \param[out] actor the resultant vtk actor object
          * \param[in] use_scalars set scalar properties to the mapper if it exists in the data. Default: true.
          */
        static void
        createActorFromVTKDataSet (const vtkSmartPointer<vtkDataSet> &data,
                                   vtkSmartPointer<vtkLODActor> &actor,
                                   bool use_scalars = true);
      private:
        /** \brief The viewpoint transformation matrix. */
        vtkSmartPointer<vtkMatrix4x4> viewpoint_transformation_;

        /** \brief Internal cell array. Used for optimizing updatePointCloud. */
        vtkSmartPointer<vtkIdTypeArray> cells_;
    };
  }
}

#endif // PCL_MODELER_CLOUD_ITEM_H_
