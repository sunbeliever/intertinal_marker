from ament_index_python.packages import get_package_share_path

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import Command, LaunchConfiguration

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


import os
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    robot_descriptions_path = get_package_share_path('wholebody_description')
    default_model_path = robot_descriptions_path / 'urdf/wholebody_description.urdf'
    default_rviz_config_path = robot_descriptions_path / 'rviz/moveit.rviz'

    frequency_arg = DeclareLaunchArgument(name='frequency', default_value='100.0',
                                    description='Refresh rate of robot_state_publisher')
    gui_arg = DeclareLaunchArgument(name='gui', default_value='true', choices=['true', 'false'],
                                    description='Flag to enable joint_state_publisher_gui')
    model_arg = DeclareLaunchArgument(name='model', default_value=str(default_model_path),
                                      description='Absolute path to robot urdf file')
    rviz_arg = DeclareLaunchArgument(name='rvizconfig', default_value=str(default_rviz_config_path),
                                     description='Absolute path to rviz config file')

    robot_description = ParameterValue(Command(['xacro ', LaunchConfiguration('model')]),
                                       value_type=str)



    package_name = 'wholebody_description'
    urdf_name = "wholebody_description.urdf"

    ld = LaunchDescription()
    pkg_share = FindPackageShare(package=package_name).find(package_name)
    urdf_model_path = os.path.join(pkg_share, f'urdf/{urdf_name}')
    print('-----------------------------')
    print('-------------model_arg----------------')
    print(default_model_path)
    print('-------------urdf_model_path----------------')
    print(urdf_model_path)

    # Static TF
    static_tf = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="static_transform_publisher",
        output="log",
        arguments=["0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "world", "3-left-arm-link01"],
    )

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        # parameters=[{'robot_description': robot_description},{'publish_frequency': LaunchConfiguration('frequency')}],
        arguments=[urdf_model_path]
    )



    # Depending on gui parameter, either launch joint_state_publisher or joint_state_publisher_gui
    joint_state_publisher_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        parameters=[{'rate': LaunchConfiguration('frequency')},
                    {
                    #  'zeros.1-joint-left-hip-roll': 0.0,
                    #  'zeros.1-joint-left-hip-yaw': 0.0,
                    #  'zeros.1-joint-left-hip-pitch': -0.51,
                    #  'zeros.1-joint-left-tarsus': 1.1,
                    #  'zeros.1-joint-left-foot': 0.62,
                    #  'zeros.2-joint-right-hip-roll': 0.0,
                    #  'zeros.2-joint-right-hip-yaw': 0.0,
                    #  'zeros.2-joint-right-hip-pitch': -0.51,
                    #  'zeros.2-joint-right-tarsus': 1.1,
                    #  'zeros.2-joint-right-foot': 0.62,
                     'zeros.3-left-arm-joint01': -2.355,
                     'zeros.3-left-arm-joint02': 1.57,
                     'zeros.3-left-arm-joint03': 0.0,
                     'zeros.3-left-arm-joint04': 1.57,
                     'zeros.3-left-arm-joint05': 0.0,
                     'zeros.3-left-arm-joint06': 0.0,
                     'zeros.3-left-arm-joint07': 0.0,
                     'zeros.4-right-arm-joint01': -2.355,
                     'zeros.4-right-arm-joint02': -1.57,
                     'zeros.4-right-arm-joint03': 0.0,
                     'zeros.4-right-arm-joint04': -1.57,
                     'zeros.4-right-arm-joint05': 0.0,
                     'zeros.4-right-arm-joint06': 0.0,
                     'zeros.4-right-arm-joint07': 0.0}],
        condition=UnlessCondition(LaunchConfiguration('gui'))
    )

    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        parameters=[{'rate': LaunchConfiguration('frequency')},
                    {
                    #  'zeros.1-joint-left-hip-roll': 0.0,
                    #  'zeros.1-joint-left-hip-yaw': 0.0,
                    #  'zeros.1-joint-left-hip-pitch': -0.51,
                    #  'zeros.1-joint-left-tarsus': 1.1,
                    #  'zeros.1-joint-left-foot': 0.62,
                    #  'zeros.2-joint-right-hip-roll': 0.0,
                    #  'zeros.2-joint-right-hip-yaw': 0.0,
                    #  'zeros.2-joint-right-hip-pitch': -0.51,
                    #  'zeros.2-joint-right-tarsus': 1.1,
                    #  'zeros.2-joint-right-foot': 0.62,
                    #  'zeros.3-left-arm-joint01': 0.0,
                    #  'zeros.3-left-arm-joint02': 0.0,
                    #  'zeros.3-left-arm-joint03': 0.0,
                    #  'zeros.3-left-arm-joint04': 0.0,
                    #  'zeros.3-left-arm-joint05': 0.0,
                    #  'zeros.3-left-arm-joint06': 0.0,
                    #  'zeros.3-left-arm-joint07': 0.0,
                    #  'zeros.4-right-arm-joint01': 0.0,
                    #  'zeros.4-right-arm-joint02': 0.0,
                    #  'zeros.4-right-arm-joint03': 0.0,
                    #  'zeros.4-right-arm-joint04': 0.0,
                    #  'zeros.4-right-arm-joint05': 0.0,
                    #  'zeros.4-right-arm-joint06': 0.0,
                    #  'zeros.4-right-arm-joint07': 0.0
                     }
                     ],
        condition=IfCondition(LaunchConfiguration('gui'))
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', LaunchConfiguration('rvizconfig')],
    )

    return LaunchDescription([
        gui_arg,
        static_tf,
        model_arg,
        frequency_arg,
        # joint_state_publisher_node,
        #joint_state_publisher_gui_node,
        robot_state_publisher_node,
        rviz_arg,
        rviz_node,

    ])
