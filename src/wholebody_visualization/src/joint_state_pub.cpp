#include "joint_state_pub.h"



void print_time_point(std::chrono::system_clock::time_point timePoint) {
  std::time_t timeStamp = std::chrono::system_clock::to_time_t(timePoint);
  std::cout << std::ctime(&timeStamp) << std::endl;
}
std::vector<geometry_msgs::msg::Point>trajectory_points;


using InteractiveMarkerControl =
  visualization_msgs::msg::InteractiveMarkerControl_<std::allocator<void>>;
 




// ros2 action send_goal /my_group_controller/follow_joint_trajectory control_msgs/action/FollowJointTrajectory "
JointStatePub::JointStatePub(std::string name) : Node(name) {
  using namespace std::placeholders;  // NOLINT
  RCLCPP_INFO(this->get_logger(), "JointStatePub: construct.");
  
  joint_states_publisher = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 100);
  marker_publisher=this->create_publisher<visualization_msgs::msg::Marker>("/visualization_marker",100);
  
  
  interactive_marker_publisher = this->create_publisher<visualization_msgs::msg::InteractiveMarker>("/simple_marker", 100);
 
   trajectory_publisher = this->create_publisher<visualization_msgs::msg::Marker>(
    "/trajectory_marker", 1);

  joint_state_subscription =
      this->create_subscription<sensor_msgs::msg::JointState>("/joint_states", 100, std::bind(&JointStatePub::joint_state_subscription_cb, this, _1));
   
    server_ = std::make_unique<interactive_markers::InteractiveMarkerServer>(//指针赋值给server_
    "control_marker",
    get_node_base_interface(),
    get_node_clock_interface(),
    get_node_logging_interface(),
    get_node_topics_interface(),
    get_node_services_interface());

 

    trajectory_marker.header.frame_id = "world";
    trajectory_marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
    trajectory_marker.action = visualization_msgs::msg::Marker::ADD;
    trajectory_marker.scale.x = 0.05;
    trajectory_marker.color.r = 1.0;
    trajectory_marker.color.g = 0.0;
    trajectory_marker.color.b = 0.0;
    trajectory_marker.color.a = 1.0;



  interactive_markers.header.frame_id="world";
  interactive_markers.name="simple_marker";
  interactive_markers.description="markers_control";
  interactive_markers.pose=markers.pose;
  interactive_markers.scale=0.2;
  visualization_msgs::msg::Marker box_marker;
  box_marker.type=visualization_msgs::msg::Marker::CUBE;
  box_marker.scale.x=0.05;
  box_marker.scale.y=0.05;
  box_marker.scale.z=0.05;
  tf2::Vector3 position;
  box_marker.pose.position.x=position.getX();
  box_marker.pose.position.y=position.getY();
  box_marker.pose.position.z=position.getZ();
   

  box_marker.color.r=0;
  box_marker.color.g=1;
  box_marker.color.a=1;
  box_marker.color.b=0;

 
 
  InteractiveMarkerControl control;
  
   tf2::Quaternion orien(1.0, 0.0, 0.0, 1.0);
    orien.normalize();
    control.orientation = tf2::toMsg(orien);
  control.name = "rotate_x";
  control.interaction_mode = InteractiveMarkerControl::ROTATE_AXIS;
  interactive_markers.controls.push_back(control);
  control.name = "move_x";
  control.interaction_mode = InteractiveMarkerControl::MOVE_AXIS;
  interactive_markers.controls.push_back(control);

  orien = tf2::Quaternion(0.0, 1.0, 0.0, 1.0);
    orien.normalize();
    control.orientation = tf2::toMsg(orien);
  control.name = "rotate_z";
  control.interaction_mode = InteractiveMarkerControl::ROTATE_AXIS;
  interactive_markers.controls.push_back(control);
  control.name = "move_z";
  control.interaction_mode = InteractiveMarkerControl::MOVE_AXIS;
  interactive_markers.controls.push_back(control);

  orien = tf2::Quaternion(0.0, 0.0, 1.0, 1.0);
    orien.normalize();
    control.orientation = tf2::toMsg(orien);
  control.name = "rotate_y";
  control.interaction_mode = InteractiveMarkerControl::ROTATE_AXIS;
  interactive_markers.controls.push_back(control);
  control.name = "move_y";
  control.interaction_mode = InteractiveMarkerControl::MOVE_AXIS;
  interactive_markers.controls.push_back(control);
  

  visualization_msgs::msg::InteractiveMarkerControl initialcontrol;
  initialcontrol.always_visible=true;
  initialcontrol.markers.push_back(box_marker);
  interactive_markers.controls.push_back(initialcontrol);
 

    


  server_->insert(interactive_markers);
  server_->setCallback(interactive_markers.name, std::bind(&JointStatePub::processFeedback, this, _1));//用户交互时需要进行回调参数，才会得到feedback

  server_->applyChanges();
 
  interactive_marker_publisher->publish(interactive_markers);  
  //marker_subscription=
    //this->create_subscription<visualization_msgs::msg::Marker>("/visualization_marker",100,std::bind(&JointStatePub::marker_subscription_cb,this,_1));
  
count=1;
  command_joint_state_msg_.position.resize(24, 0.0);
  command_joint_state_msg_.velocity.resize(24, 0.0);
  command_joint_state_msg_.effort.resize(24, 0.0);
  command_joint_state_msg_.position.resize(24, 0.0);

  cur_joint_state_msg_.position.resize(24, 0.0);
  cur_joint_state_msg_.velocity.resize(24, 0.0);
  cur_joint_state_msg_.effort.resize(24, 0.0);
  cur_joint_state_msg_.position.resize(24, 0.0);

  joint_state_publish_thread_ptr = std::make_shared<std::thread>(std::bind(&JointStatePub::JointStatePublishThread, this));
  
  // marker_publish_thread_ptr=std::make_shared<std::thread>(std::bind(&JointStatePub::JointStatePublishThread,this));
}



JointStatePub::~JointStatePub() {
  stop_flag = true;
  joint_state_publish_thread_ptr->join();
  // marker_publish_thread_ptr->join();
}

 void JointStatePub::processFeedback(
  const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr & feedback
  )
{
 
  // RCLCPP_INFO(logger, "%s", oss.str().c_str());
   geometry_msgs::msg::Point point;
  point.x = feedback->pose.position.x;
  point.y = feedback->pose.position.y;
  point.z = feedback->pose.position.z;

 
   markers.pose.position=feedback->pose.position;

  trajectory_points.push_back(point);
   // 将轨迹点添加到 Marker 的 points 数组
    trajectory_marker.points=trajectory_points;

    // 发布轨迹 Marker
    trajectory_publisher->publish(trajectory_marker);

    // 这里可以根据需要进行调整，例如设置发布的频率
    rclcpp::sleep_for(std::chrono::milliseconds(100));
  marker_publisher->publish(markers);
  server_->applyChanges();
 
}


void JointStatePub ::publish_joint_command() {
  std::unique_lock<std::mutex> lock(joint_command_mutex_);
  using namespace std::placeholders; 
  command_joint_state_msg_.name = {
      "1-joint-left-hip-roll",  "1-joint-left-hip-yaw",  "1-joint-left-hip-pitch",  "1-joint-left-tarsus",  "1-joint-left-foot",
      "2-joint-right-hip-roll", "2-joint-right-hip-yaw", "2-joint-right-hip-pitch", "2-joint-right-tarsus", "2-joint-right-foot",
      "3-left-arm-joint01",     "3-left-arm-joint02",    "3-left-arm-joint03",      "3-left-arm-joint04",   "3-left-arm-joint05",
      "3-left-arm-joint06",     "3-left-arm-joint07",    "4-right-arm-joint01",     "4-right-arm-joint02",  "4-right-arm-joint03",
      "4-right-arm-joint04",    "4-right-arm-joint05",   "4-right-arm-joint06",     "4-right-arm-joint07",
  };

  command_joint_state_msg_.header.stamp = this->now();
  for (int ii = 0; ii < 24; ii++) {
    command_joint_state_msg_.position[ii] += 0.00001;  // run()  command
    command_joint_state_msg_.velocity[ii] = 1.1;
    command_joint_state_msg_.effort[ii] = 1.1;
  }
 
  if (count%100==0){
  // visualization_msgs::msg::Marker line_strip;
  // visualization_msgs::msg::Marker line_list;
  // visualization_msgs::msg::Marker markers;
  markers.header.frame_id="world";
  markers.header.stamp=now();
  markers.ns="markers";
  markers.id=0;
  markers.type=visualization_msgs::msg::Marker::SPHERE;
  markers.action=visualization_msgs::msg::Marker::ADD;
  markers.pose.orientation.x=0;
  markers.pose.orientation.y=0;
  markers.pose.orientation.z=0;
  markers.pose.orientation.w=1;
  markers.scale.x=0.1;
  markers.scale.y=0.1;
  markers.scale.z=0.1;
  markers.color.r=1;
  markers.color.g=0;
  markers.color.b=0;
  markers.color.a=1;
 
 


  
line_strip.type=visualization_msgs::msg::Marker::LINE_STRIP;
// line_list.type=visualization_msgs::msg::Marker::LINE_LIST;
line_strip.header.frame_id="world";
// line_list.header.frame_id="world";
line_strip.id=1;
// line_list.id=2;
line_strip.scale.x=0.001;
// line_list.scale.x=0.001;
line_strip.color.r=1;
line_strip.color.a=1;
line_strip.color.b=0;
line_strip.color.g=0;

// line_list.color.r=0;
// line_list.color.a=1;
// line_list.color.g=1;
// line_list.color.b=0;
geometry_msgs::msg::Point P;
P.x=markers.pose.position.x;
P.y=markers.pose.position.y;
P.z=markers.pose.position.z;
 
   line_strip.points.push_back(P);
   
   
   
   
  marker_publisher->publish(markers);
  marker_publisher->publish(line_strip);


  
  // interactive_marker_publisher->publish(interactive_markers);
  // interactive_marker_publisher->publish(interactive_markers);
  // server_->applyChanges();
  
  }

  joint_states_publisher->publish(command_joint_state_msg_);
 

  count++;
  }


void JointStatePub::joint_state_subscription_cb(sensor_msgs::msg::JointState::UniquePtr msg) {
  std::unique_lock<std::mutex> lock(joint_state_mutex_);

  cur_joint_state_msg_.name = {
      "1-joint-left-hip-roll",  "1-joint-left-hip-yaw",  "1-joint-left-hip-pitch",  "1-joint-left-tarsus",  "1-joint-left-foot",
      "2-joint-right-hip-roll", "2-joint-right-hip-yaw", "2-joint-right-hip-pitch", "2-joint-right-tarsus", "2-joint-right-foot",
      "3-left-arm-joint01",     "3-left-arm-joint02",    "3-left-arm-joint03",      "3-left-arm-joint04",   "3-left-arm-joint05",
      "3-left-arm-joint06",     "3-left-arm-joint07",    "4-right-arm-joint01",     "4-right-arm-joint02",  "4-right-arm-joint03",
      "4-right-arm-joint04",    "4-right-arm-joint05",   "4-right-arm-joint06",     "4-right-arm-joint07",
  };
  cur_joint_state_msg_.header.stamp = this->now();
  for (int ii = 0; ii < 24; ii++) {
    cur_joint_state_msg_.position[ii] = msg->position[ii];
    cur_joint_state_msg_.velocity[ii] = 0;
    cur_joint_state_msg_.effort[ii] = 0;
  }
  joint_states_publisher->publish(cur_joint_state_msg_);
  publish_joint_command();
}


void JointStatePub::JointStatePublishThread() {
  RCLCPP_INFO(this->get_logger(), "JointStatePublishThread. start joint state publish.");
  while (!stop_flag) {
    publish_joint_command();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}



// void JointStatePub::processFeedback(
//   const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr & feedback,
//   rclcpp::Logger logger)
// {
//   std::ostringstream oss;
//   oss << feedback->marker_name << " is now at " << feedback->pose.position.x << ", " <<
//     feedback->pose.position.y << ", " << feedback->pose.position.z;
//   RCLCPP_INFO(logger, "%s", oss.str().c_str());
// }




int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<JointStatePub>("joint_state_pub");
  
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);








  RCLCPP_INFO(node->get_logger(), "Ready");
  executor.spin();
 rclcpp::shutdown();
  return 0;
}
