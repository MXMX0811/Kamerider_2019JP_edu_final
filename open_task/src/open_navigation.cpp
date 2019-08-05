/*
    Date: 2019/07/06
    Author: Xu Yucheng
    Abstract: Navigation code for Team KameRider Open Challenge
*/
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ros/ros.h>
#include <std_msgs/String.h>
//navigation中需要使用的位姿信息头文件
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseWithCovariance.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Quaternion.h>
//move_base头文件
#include <move_base_msgs/MoveBaseGoal.h>
#include <move_base_msgs/MoveBaseAction.h>
//actionlib头文件
#include <actionlib/client/simple_action_client.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

class open_navigation
{
private:
    bool _start = false;
    std::string sub_master_navigation_topic_name;
    std::string pub_navigation_result_topic_name;

    ros::Subscriber sub_master;
    ros::Publisher pub_result;

    geometry_msgs::Pose goal_pose;

    void masterCallback(const geometry_msgs::Pose& msg)
    {
        ROS_INFO ("Receive a New Navigation Pose from MASTER");
        goal_pose.position.x = msg.position.x;
        goal_pose.position.y = msg.position.y;
        goal_pose.position.z = msg.position.z;
        goal_pose.orientation.w = msg.orientation.w;
        goal_pose.orientation.x = msg.orientation.x;
        goal_pose.orientation.y = msg.orientation.y;
        goal_pose.orientation.z = msg.orientation.z;
        open_navigation::_start = true;
    }

public:
    int run(int argc, char** argv)
    {
        ROS_INFO ("-------INIT--------");
        ros::init (argc, argv, "open_navigation");
        ros::NodeHandle nh;

        nh.param<std::string>("sub_master_navigation_topic_name", open_navigation::sub_master_navigation_topic_name, "/kamerider_1/navi/input");
        nh.param<std::string>("pub_navigation_result_topic_name", open_navigation::pub_navigation_result_topic_name, "/kamerider_1/navi/output");

        sub_master = nh.subscribe(sub_master_navigation_topic_name, 1, &open_navigation::masterCallback, this);
        pub_result = nh.advertise<std_msgs::String>(pub_navigation_result_topic_name, 1);

        // Set move_base server
        MoveBaseClient mc_("move_base", true);
        move_base_msgs::MoveBaseGoal nav_goal;

        while (ros::ok())
        {
            if (_start)
            {
                ROS_INFO("START Navigating to a New Position");
                nav_goal.target_pose.header.frame_id = "map";
                nav_goal.target_pose.header.stamp = ros::Time::now();
                nav_goal.target_pose.pose = geometry_msgs::Pose (goal_pose);
                while (!mc_.waitForServer (ros::Duration(5.0)))
                {
                    ROS_INFO("Waiting For Server...");
                }
                mc_.sendGoal (nav_goal);
                mc_.waitForResult (ros::Duration(40.0));
                if (mc_.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
                {
                    ROS_INFO ("Successfully arrived the current goal position");
                    _start = false;
                    std_msgs::String msg;
                    msg.data = "success";
                    pub_result.publish(msg);
                }
            }
            ros::spinOnce();
        }
    }
};

int main (int argc, char** argv)
{
    open_navigation agent;
    return agent.run(argc, argv);
}