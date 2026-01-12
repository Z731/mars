from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='mars_ros2',
            executable='mars_xlog_node',
            name='mars_xlog_node',
            output='screen',
            parameters=[{
                'log_dir': './mars_log',
                'name_prefix': 'mars_ros2',
                'console_log': True,
                'debug': False,
            }],
        )
    ])
