打开终端，运行 ssh zmx@本机ip
运行roscore

新开终端，运行 ssh zmx@本机ip
export ROS_MASTER_URI=http://本机ip:11311
rosrun robocup_2019_edu_open master.py

新开终端，运行 ssh zmx@本机ip
export ROS_MASTER_URI=http://本机ip:11311
rosrun robocup_2019_edu_open open_communication

还需运行soundplay_node.py，运行rostopic pub /control_command std_msgs/String "data: 'start counication'"发送开始信号

运行rosrun socket socket2topic_nd接受脑电信号

