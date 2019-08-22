#! /usr/bin/env python
# -*- coding: utf-8 -*-
'''
    Date: 2019/07/06
    Author: Xu Yucheng
    Abstract: Object Search Code
'''
import os
import cv2
import numpy as np 
import base64
import rospy
from std_msgs.msg import Int8
from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
from darknet_ros_msgs.msg import BoundingBoxes
from geometry_msgs.msg import Twist, Point, Quaternion
import tf
from rbx1_nav.transform_utils import quat_to_angle, normalize_angle
from math import radians, copysign, sqrt, pow, pi
from sound_play.libsoundplay import SoundClient


IMAGE_DIR = '../result'
UNSTART = 0
PROCESS = 1
FINISH  = 2
FAILED  = 3

class ObjectSearch(object):
    def __init__(self):
        # State Flags
        self._turn = UNSTART
        self._search = UNSTART
        self._adjust = UNSTART
        self._reach  = UNSTART
        self._target = ''

        # ROS Parameters
        self.sub_nav = None
        self.sub_bbox = None
        self.sub_odom = None
        self.sub_target = None
        
        self.pub_odom = None
        self.pub_result = None

        self.sub_master_target_topic_name = None
        self.sub_bbox_topic_name = None
        self.sub_navigation_result_topic_name = None

        self.pub_search_result_topic_name = None

        self._center_x = 320
        self._object_center_x = 0
        
        # Initialize sound client
        self.sh = SoundClient(blocking=True)
        rospy.sleep(1)
        self.sh.stopAll()
        rospy.sleep(1)

        # Odom parameters
        rate = 20
        # Set the equivalent ROS rate variable
        self.r = rospy.Rate(rate)
        self._target = None
        # Set the forward linear speed to 0.15 meters per second 
        self.linear_speed = 0.15

        ############### Set the travel distance in meters
        self.goal_distance = 0
        # Set the rotation speed in radians per second
        self.angular_speed = 0.5
        # Set the angular tolerance in degrees converted to radians
        self.angular_tolerance = radians(1.0)

        # Initialize the tf listener
        self.tf_listener = tf.TransformListener()
        # Give tf some time to fill its buffer
        rospy.sleep(2)
        self.odom_frame = '/odom'
        rospy.loginfo("Ready for adjust robot pose by using odom")
        self.cmd_vel = rospy.Publisher('/cmd_vel_mux/input/navi', Twist, queue_size=5)

        # Set Ros parameters
        self.set_params()
        self.main_loop()



    def set_params(self):
        self.sub_bbox_topic_name              = rospy.get_param("sub_bbox_topic_name", "/darknet_ros/bounding_boxes")
        self.sub_navigation_result_topic_name = rospy.get_param("sub_navigation_result_topic_name", "/kamerider_3/navi/output")
        self.sub_master_target_topic_name     = rospy.get_param("sub_master_target_topic_name", "/image/target")
        self.pub_search_result_topic_name     = rospy.get_param("pub_search_result_topic_name", "/kamerider_3/image/result")

        self.sub_bbox = rospy.Subscriber(self.sub_bbox_topic_name, BoundingBoxes, self.darknet_callback)
        self.sub_nav  = rospy.Subscriber(self.sub_navigation_result_topic_name, String, self.navi_callback)       
        self.sub_target = rospy.Subscriber(self.sub_master_target_topic_name, String, self.master_callback)
        self.sub_control = rospy.Subscriber("/kamerider/control", String, self.control_callback)
        self.pub_result = rospy.Publisher(self.pub_search_result_topic_name, String, queue_size=1)


        try:
            self.tf_listener.waitForTransform(self.odom_frame, '/base_footprint', rospy.Time(), rospy.Duration(1.0))
            self.base_frame = '/base_footprint'
        except (tf.Exception, tf.ConnectivityException, tf.LookupException):
            try:
                self.tf_listener.waitForTransform(self.odom_frame, '/base_link', rospy.Time(), rospy.Duration(1.0))
                self.base_frame = '/base_link'
            except (tf.Exception, tf.ConnectivityException, tf.LookupException):
                rospy.loginfo("Cannot find transform between /odom and /base_link or /base_footprint")
                rospy.signal_shutdown("tf Exception") 

    
    def control_callback(self, msg):
        if msg.data == "stop":
            self._search = FINISH
            os.system("rosnode kill /darknet_ros")
            current_position_found = False
    
    
    def master_callback(self, msg):
        rospy.loginfo("Receive Target Message From Master: {}".format(msg))
        print("target",self._target)
        #if self._target==None:
        self._target = msg.data
        self._target = self._target.replace('\r','')
        text = "I will find "+self._target
       	
        

    
    def navi_callback(self, msg):
        rospy.loginfo("Receive Navigation Message From Navigation Node: {}".format(msg))
        if msg.data == "success":
            rospy.loginfo("Now Start Find Target Object")
            self._search = PROCESS
            print("search",self._search)
            
    
    def darknet_callback(self, msg):
        if len(msg.bounding_boxes) >= 0:
            for bbox in msg.bounding_boxes:
                if bbox.Class == self._target:
                    if self._search != FINISH:
                        print ("Find Target Object {}".format(self._target))
                    self._search = FINISH
                    self._area = (bbox.xmax - bbox.xmin) * (bbox.ymax - bbox.ymin)
                    self._object_center_x = int((bbox.xmax + bbox.xmin)/2)
                    
                    
    def get_odom(self):
        # Get the current transform between the odom and base frames
        try:
            (trans, rot)  = self.tf_listener.lookupTransform(self.odom_frame, self.base_frame, rospy.Time(0))
        except (tf.Exception, tf.ConnectivityException, tf.LookupException):
            rospy.loginfo("TF Exception")
            return

        return (Point(*trans), quat_to_angle(Quaternion(*rot)))
    
    def move_around(self,  start_rotation, goal_radius=pi/6):
        self._turn = PROCESS
        move_cmd = Twist()

        # Set the movement command to a rotation
        move_cmd.angular.z = self.angular_speed
        if goal_radius < 0:
            move_cmd.angular.z = -self.angular_speed
            goal_radius = -goal_radius

        # Track the last angle measured
        last_angle = start_rotation
        
        # Track how far we have turned
        turn_angle = 0
        
        while abs(turn_angle + self.angular_tolerance) < abs(goal_radius) and not rospy.is_shutdown():
            # Publish the Twist message and sleep 1 cycle         
            self.cmd_vel.publish(move_cmd)
            self.r.sleep()
            
            # Get the current rotation
            (position, rotation) = self.get_odom()
            
            # Compute the amount of rotation since the last loop
            delta_angle = normalize_angle(rotation - last_angle)
            
            # Add to the running total
            turn_angle += delta_angle
            last_angle = rotation
        # Stop the robot 
        move_cmd = Twist()
        self.cmd_vel.publish(move_cmd)
        rospy.sleep(1)
        print("Turn {} radius  end!".format(goal_radius))
        self._turn = FINISH
            
        

    def go_straight(self,start_pos, goal_distance=0):
        move_cmd = Twist()
                
        # Set the movement command to forward motion
        move_cmd.linear.x = self.linear_speed
        
        if goal_distance < 0:
            move_cmd.linear.x = -self.linear_speed
            goal_distance = -goal_distance
                    
        x_start = start_pos.x
        y_start = start_pos.y
        
        # Keep track of the distance traveled
        traveled_distance = 0
        
        # Enter the loop to move along a side
        while traveled_distance < goal_distance and not rospy.is_shutdown():
            # Publish the Twist message and sleep 1 cycle         
            self.cmd_vel.publish(move_cmd)
            
            self.r.sleep()

            # Get the current position
            (position, rotation) = self.get_odom()
            
            # Compute the Euclidean distance from the start
            traveled_distance = sqrt(pow((position.x - x_start), 2) + 
                            pow((position.y - y_start), 2))
        # Stop the robot 
        move_cmd = Twist()
        self.cmd_vel.publish(move_cmd)
        rospy.sleep(1)
        print("go {} meters long end!".format(goal_distance))
    
        
    def main_loop(self):

        rospy.loginfo("Start the main loop")
        while (True):
            # 等候来自导航节点和master的消息与目标
            while (True):
                #print(self._target)	
                if self._search == PROCESS:
                    if self._target:
                        rospy.loginfo("Start to find target object: {}".format(self._target))
                        os.system("gnome-terminal -x bash -c 'roslaunch darknet_ros yolo_robocup.launch'")
                        rospy.loginfo("Start Darknet Please Wait...")
                        rospy.sleep(15)
                        break
            
            angle_total = 0
            current_position_found = True
            # 转动机器人寻找物体
            rospy.loginfo("Start Turning the Robot")
            while (True):
                if angle_total >= 2*pi:
                    # 转满360 则表示在当前位置没能找到目标物体
                    # 则向master发布none消息，准备前往下一个点
                    current_position_found = False
                    break
                if self._search == FINISH:
                    break
                if self._turn != PROCESS and self._search != FINISH:
                    position = Point()
                    # Get the starting position values     
                    (position, rotation) = self.get_odom()
                    self.move_around(rotation, pi/6)
                    angle_total += pi/6
            
            if current_position_found:
                # 调整机器人正对物体
                rospy.loginfo("Start Adjust the orientation")
                while (True):
                    print ("Current Object X: {}, Center X: {}".format(self._object_center_x, self._center_x))
                    if abs(self._object_center_x - self._center_x) <= 10:
                        rospy.loginfo("Finish Adjusting Orientation")
                        break
                    else:
                        if self._object_center_x < self._center_x:
                            position = Point()
                            # Get the starting position values     
                            (position, rotation) = self.get_odom()
                            self.move_around(rotation, goal_radius=pi/120)
                        if self._object_center_x > self._center_x:
                            position = Point()
                            # Get the starting position values     
                            (position, rotation) = self.get_odom()
                            self.move_around(rotation, goal_radius=-pi/120)
                
                # 逼近物体
                while (True):
                    print ("Current Object Pixel Area： {}".format(self._area))
                    if self._area > 10000:
                        rospy.loginfo("Target Object Reached!")
                        result = "Do you want "+self._target
                        self.sh.say(result)
                        rospy.sleep(5)
                        self.sh.say("I found the target")
                        break
                    else:
                        position = Point()
                        # Get the starting position values     
                        (position, rotation) = self.get_odom()
                        self.go_straight(position, 0.3)
                self.go_straight(position, 0.25)
                msg = String()
                msg.data = "target_found"
                self.pub_result.publish(msg)
                break
                #self._target = None
            else:
                msg = String()
                msg.data = "none"
                self.pub_result.publish(msg)
                
            os.system('gnome-terminal -x bash -c "rosnode kill /darknet_ros"')
            rospy.sleep(10)


if __name__ == '__main__':
    rospy.init_node("ObjectSearch", anonymous=False)
    searcher = ObjectSearch()
    #searcher.main_loop()
    rospy.spin()
        






