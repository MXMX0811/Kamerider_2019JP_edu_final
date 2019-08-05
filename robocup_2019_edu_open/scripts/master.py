#! /usr/bin/env python
# -*- encoding: UTF-8 -*-
import rospy
import numpy as np
import time
from std_msgs.msg import String
from geometry_msgs.msg import Pose


class path_pub():
    def __init__(self):
        rospy.init_node("path_pub")
        self.if_pub_next_point = False
        self.rest_pose = []
        self.index = [1,2,4,0,5,7,3,8,6]
        self.get_all_pose("/home/zmx/catkin_ws/src/robocup_2019_edu_open/scripts/points.txt")
        self.pose_init_1 = self.rest_pose[0]
        self.pose_init_3 = self.rest_pose[1]
        self.rest_pose.remove(self.rest_pose[0])
        self.rest_pose.remove(self.rest_pose[0])
        # print "------------"
        # print self.rest_pose
        # 发布器
        self.target_pub = rospy.Publisher("/image/target", String, queue_size=15)
        self.start_pub = rospy.Publisher("/control_command", String, queue_size=15)
        self.stop_pub = rospy.Publisher("/kamerider/control", String, queue_size=15)
        self.nav_pub_1 = rospy.Publisher("/kamerider_1/navi/input", Pose, queue_size=15)
        #self.nav_pub_3 = rospy.Publisher("/kamerider_3/navi/input", Pose, queue_size=15)
        self.socket_sub = rospy.Subscriber("/socket/target", String, self.socket_callback)
        self.nav_1_sub = rospy.Subscriber("/kamerider_1/image/result", String, self.image_result_1_callback)
        #self.nav_3_sub = rospy.Subscriber("/kamerider_3/image/result", String, self.image_result_3_callback)
        rospy.spin()

    def image_result_1_callback(self, msg):
        print msg.data
        if msg.data == "none":
            self.nav_pub_1.publish(self.rest_pose[self.index[0]])
            self.index.remove(self.index[0])
        else:
            temp = String()
            temp.data = "stop"
            self.stop_pub.publish(temp)
            time.sleep(4)
            self.nav_pub_1.publish(self.pose_init_1)
            #self.nav_pub_3.publish(self.pose_init_3)

    def image_result_3_callback(self, msg):
        print msg.data
        if msg.data == "none":
            self.nav_pub_3.publish(self.rest_pose[self.index[0]])
            self.index.remove(self.index[0])
        else:
            temp = String()
            temp.data = "stop"
            self.stop_pub.publish(temp)
            time.sleep(4)
            self.nav_pub_1.publish(self.pose_init_1)
            self.nav_pub_3.publish(self.pose_init_3)

    def socket_callback(self, msg):
        print msg.data
        self.target_pub.publish(msg.data)
        self.nav_pub_1.publish(self.rest_pose[self.index[0]])
        self.index.remove(self.index[0])
        #self.nav_pub_3.publish(self.rest_pose[self.index[0]])
        #self.index.remove(self.index[0])

    def get_all_pose(self, file_name):
        f = open(file_name, 'r')
        sourceInLines = f.readlines()
        for line in sourceInLines:
            curr_pos = Pose()
            temp1 = line.strip('\n')
            temp2 = temp1.split(',')
            curr_pos.position.x = float(temp2[0])
            curr_pos.position.y = float(temp2[1])
            curr_pos.position.z = float(temp2[2])
            curr_pos.orientation.x = float(temp2[3])
            curr_pos.orientation.y = float(temp2[4])
            curr_pos.orientation.z = float(temp2[5])
            curr_pos.orientation.w = float(temp2[6])
            self.rest_pose.insert(0, curr_pos)
        print ("↓↓↓↓↓↓↓↓↓↓↓↓point↓↓↓↓↓↓↓↓↓↓↓↓")
        print (self.rest_pose)
        print ("↑↑↑↑↑↑↑↑↑↑↑↑point↑↑↑↑↑↑↑↑↑↑↑↑")
        print ('\033[0;32m [Kamerider I] Points Loaded! \033[0m')



if __name__ == '__main__':
    path_pub()
