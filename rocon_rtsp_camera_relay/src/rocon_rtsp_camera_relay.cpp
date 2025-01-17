/*
 License: BSD
   https://raw.github.com/robotics-in-concert/rocon_devices/license/LICENSE
*/

#include <rocon_rtsp_camera_relay/rocon_rtsp_camera_relay.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

namespace rocon {

RoconRtspCameraRelay::RoconRtspCameraRelay(ros::NodeHandle& n) : nh_(n)
{
  image_transport::ImageTransport it(nh_);    
  pub_video_ = it.advertise("/camera/image_raw", 1000);
  // pub_camera_info_ = nh_.advertise<sensor_msgs::CameraInfo>("camera_info", 1000);
  pub_status_ = nh_.advertise<std_msgs::String>("status", 1000);
  frameCnt = 0;
}

RoconRtspCameraRelay::~RoconRtspCameraRelay()
{
  vcap_.release();
}

bool RoconRtspCameraRelay::init(const std::string video_stream_url, const std::string path) {
  video_stream_address_ = video_stream_url;
  filePath = path;
  if (!vcap_.open(video_stream_address_)) 
    return false; 
  else
    return true;
}

bool RoconRtspCameraRelay::reset(const std::string video_stream_url)
{
  vcap_.release();
  return init(video_stream_url, filePath);
}

/*
  Convert cv::Mat to sensor_msgs:Image and CameraInfo
 */
void RoconRtspCameraRelay::convertCvToRosImg(const cv::Mat& mat, sensor_msgs::Image& ros_img, sensor_msgs::CameraInfo& ci)
{
  cv_bridge::CvImage cv_img;

  cv_img.encoding = sensor_msgs::image_encodings::BGR8;
  cv_img.image = mat;
  cv_img.toImageMsg(ros_img);
  ros_img.header.stamp = ros::Time(frameCnt / 30.0);
  frameCnt++;
  // ci.header = ros_img.header;
  // ci.width = ros_img.width;
  // ci.height = ros_img.height;
  
  return;
}


void RoconRtspCameraRelay::spin()
{
  cv::Mat mat;
  sensor_msgs::CameraInfo ci;
  sensor_msgs::Image ros_img;
  std_msgs::String ros_str;

  while(ros::ok())
  {
    if(!vcap_.read(mat)) {
      // status_ = "No frame from camera";
      // cv::waitKey();
      status_ = "finish";
      // break;
    }
    else {
      status_ = "live";
    }

    ros_str.data = status_;
    pub_status_.publish(ros_str);

    if (status_ == "finish") break;

    convertCvToRosImg(mat, ros_img, ci);
    stringstream ss;
    ss << "/home/gamelab/TrafficAccident/project/" << filePath << "/images/" << fixed << setprecision(6) << frameCnt / 30.0 << ".jpg";
    string str;
    ss >> str;
    cv::imwrite(str, mat);
    pub_video_.publish(ros_img);
    // pub_camera_info_.publish(ci);
    cv::waitKey(30);
  }
}
}
