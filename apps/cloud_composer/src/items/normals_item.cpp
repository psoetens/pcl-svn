#include <pcl/apps/cloud_composer/qt.h>
#include <pcl/apps/cloud_composer/items/normals_item.h>
#include <pcl/apps/cloud_composer/items/cloud_item.h>


pcl::cloud_composer::NormalsItem::NormalsItem (QString name, pcl::PointCloud<pcl::Normal>::Ptr normals_ptr, double radius)
  : CloudComposerItem (name)
  , normals_ptr_ (normals_ptr)

{
  
  this->setData (QVariant::fromValue (normals_ptr), NORMALS_CLOUD);
  
  properties_->addProperty ("Radius", QVariant (radius));
  properties_->addProperty ("Scale", QVariant (0.02), Qt::ItemIsEditable | Qt::ItemIsEnabled);
  properties_->addProperty ("Level", QVariant (100), Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

pcl::cloud_composer::NormalsItem*
pcl::cloud_composer::NormalsItem::clone () const
{
  pcl::PointCloud<pcl::Normal>::Ptr normals_copy (new pcl::PointCloud<pcl::Normal> (*normals_ptr_));
  //Vector4f and Quaternionf do deep copies using copy constructor
  NormalsItem* new_item = new NormalsItem (this->text (), normals_copy, 0);
  
  PropertiesModel* new_item_properties = new_item->getProperties ();
  new_item_properties->copyProperties (properties_);
  
  return new_item;  
}

pcl::cloud_composer::NormalsItem::~NormalsItem ()
{
  
}

void
pcl::cloud_composer::NormalsItem::paintView (boost::shared_ptr<pcl::visualization::PCLVisualizer> vis) const
{
  //Get the parent cloud, convert to XYZ 
  if (parent ()->type () == CLOUD_ITEM)
  {
    QVariant cloud_ptr = parent ()->data (CLOUD);
    sensor_msgs::PointCloud2::Ptr cloud_blob = cloud_ptr.value<sensor_msgs::PointCloud2::Ptr> ();
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromROSMsg (*cloud_blob, *cloud); 
    double scale = properties_->getProperty ("Scale").toDouble ();
    int level = properties_->getProperty ("Level").toInt ();
    qDebug () << "Removing old normals...";
    vis->removePointCloud (item_id_.toStdString ());
    qDebug () << QString("Adding point cloud normals, level=%1, scale=%2").arg(level).arg(scale);
    vis->addPointCloudNormals<pcl::PointXYZ, pcl::Normal> (cloud, normals_ptr_, level, scale, item_id_.toStdString ());
  }
  else
    qWarning () << "Normal item inserted, but parent not a cloud. Don't know how to draw that!";
}

void
pcl::cloud_composer::NormalsItem::removeFromView (boost::shared_ptr<pcl::visualization::PCLVisualizer> vis) const
{  
  qDebug () << "Removing Normals "<<item_id_;
  vis->removePointCloud (item_id_.toStdString ());
}