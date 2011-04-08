#ifndef PCL_MESSAGE_IMAGE_H
#define PCL_MESSAGE_IMAGE_H
#include <string>
#include <vector>
#include <ostream>

// Include the correct Header path here
#include "std_msgs/Header.h"

namespace sensor_msgs
{
  template <class ContainerAllocator>
  struct Image_ 
  {
    typedef Image_<ContainerAllocator> Type;

    Image_()
    : header()
    , height(0)
    , width(0)
    , encoding()
    , is_bigendian(0)
    , step(0)
    , data()
    {
    }

    Image_(const ContainerAllocator& _alloc)
    : header(_alloc)
    , height(0)
    , width(0)
    , encoding(_alloc)
    , is_bigendian(0)
    , step(0)
    , data(_alloc)
    {
    }

    typedef  ::std_msgs::Header_<ContainerAllocator>  _header_type;
     ::std_msgs::Header_<ContainerAllocator>  header;

    typedef uint32_t _height_type;
    uint32_t height;

    typedef uint32_t _width_type;
    uint32_t width;

    typedef std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  _encoding_type;
    std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  encoding;

    typedef uint8_t _is_bigendian_type;
    uint8_t is_bigendian;

    typedef uint32_t _step_type;
    uint32_t step;

    typedef std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other >  _data_type;
    std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other >  data;

    typedef boost::shared_ptr< ::sensor_msgs::Image_<ContainerAllocator> > Ptr;
    typedef boost::shared_ptr< ::sensor_msgs::Image_<ContainerAllocator>  const> ConstPtr;
  }; // struct Image
  typedef  ::sensor_msgs::Image_<std::allocator<void> > Image;

  typedef boost::shared_ptr< ::sensor_msgs::Image> ImagePtr;
  typedef boost::shared_ptr< ::sensor_msgs::Image const> ImageConstPtr;

  template<typename ContainerAllocator>
  std::ostream& stream_with_indentation (std::ostream& s, const std::string& indent, 
                                         const ::sensor_msgs::Image_<ContainerAllocator> & v)
  {
    s << indent << "header: " << std::endl;
    stream_with_indentation (s, indent + "  ", v.header);
    s << indent << "height: ";
    s << indent << "  " << v.height << std::endl;
    s << indent << "width: ";
    s << indent << "  " << v.width << std::endl;
    s << indent << "encoding: ";
    s << indent << "  " << v.encoding << std::endl;
    s << indent << "is_bigendian: ";
    s << indent << "  " << v.is_bigendian << std::endl;
    s << indent << "step: ";
    s << indent << "  " << v.step << std::endl;
    s << indent << "data[]" << std::endl;
    for (size_t i = 0; i < v.data.size (); ++i)
    {
      s << indent << "  data[" << i << "]: ";
      s << indent << "  " << v.data[i] << std::endl;
    }
    return (s);
  }

  template<typename ContainerAllocator>
  std::ostream& operator<<(std::ostream& s, const  ::sensor_msgs::Image_<ContainerAllocator> & v)
  {
    stream_with_indentation (s, "", v);
    return (s);
  }

} // namespace sensor_msgs

#endif // PCL_MESSAGE_IMAGE_H

