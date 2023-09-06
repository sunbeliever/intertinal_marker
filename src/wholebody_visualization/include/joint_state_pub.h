#ifndef MYACTIONROBOT_H
#define MYACTIONROBOT_H

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "tf2/LinearMath/Transform.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Vector3.h"
#include "tf2/transform_datatypes.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "visualization_msgs/msg/interactive_marker.hpp"
#include "visualization_msgs/msg/interactive_marker_control.hpp"
#include "visualization_msgs/msg/interactive_marker_feedback.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "interactive_markers/interactive_marker_server.hpp"
#include "interactive_markers/menu_handler.hpp"

#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>


class JointStatePub : public rclcpp::Node {
 public:
  using FollowJointTrajectory = control_msgs::action::FollowJointTrajectory;
  using GoalHandleFJT = rclcpp_action::ServerGoalHandle<FollowJointTrajectory>;
  explicit JointStatePub(std::string name);
  ~JointStatePub();

  // void setCallback(const geometry_msgs::msg::Pose::SharedPtr msg){
  //  geometry_msgs::msg::Pose position;
  //  position.position.x=msg->position.x;
  //  position.position.y=msg->position.y;
  //  position.position.z=msg->position.z;
  //  }

  
    // void publish_joint_command();

    std::unique_ptr<interactive_markers::InteractiveMarkerServer> server_;

  
 private:
  void publish_joint_command();
  
  void joint_state_subscription_cb(sensor_msgs::msg::JointState::UniquePtr msg);
  
  void JointStatePublishThread();
  
  void processFeedback( const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr & feedback);

 

 private:
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_states_publisher;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_subscription;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_publisher;
  

  rclcpp::Publisher<visualization_msgs::msg::InteractiveMarker>::SharedPtr interactive_marker_publisher;
  


  
  
  std::mutex joint_state_mutex_;
  std::mutex joint_command_mutex_;

  sensor_msgs::msg::JointState command_joint_state_msg_;
  sensor_msgs::msg::JointState cur_joint_state_msg_;


  
  

  
 std::vector<geometry_msgs::msg::Point> trajectory_points;
visualization_msgs::msg::Marker trajectory_marker;
rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr trajectory_publisher;




  bool stop_flag = false;
  std::shared_ptr<std::thread> joint_state_publish_thread_ptr;
 

  int count;
 
  visualization_msgs::msg::Marker line_strip;
  visualization_msgs::msg::Marker line_list;
  visualization_msgs::msg::Marker markers;

  visualization_msgs::msg::InteractiveMarker interactive_markers;


   

  
};

#endif  // MYACTIONROBOT_H
