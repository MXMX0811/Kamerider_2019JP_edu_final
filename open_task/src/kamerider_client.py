#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
    Date: 2019/08/22
    Author: Zhang Mingxin
    Abstract: Code for open project(client)
"""
import socket
import rospy
from std_msgs.msg import String
from geometry_msgs.msg import Pose, Point, Quaternion

class kamerider_client():
	def __init__(self):
		self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		print(self.client)
		
		while True:
			try:
				self.client.connect(('172.20.10.12', 40001))
				break
			except:
				continue
		
		self.pub_tar = rospy.Publisher('/image/target', String, queue_size=15)
		self.pub_pos = rospy.Publisher('/kamerider_3/navi/input', Pose, queue_size=15)
		
		self.tar = self.client.recv(1024)
		print('target',self.tar)
		self.pub_tar.publish(self.tar)
		rospy.sleep(1)
		self.recv_pub()
		rospy.Subscriber('/kamerider_3/image/result', String, self.image_result_callback)
	
	def image_result_callback(self, msg):
		print('image result:', msg.data)
		self.client.send(msg.data)
		self.recv_pub()
		
	def recv_pub(self):
		self.data = self.client.recv(1024)
		print(str(self.data))
		self.temp = self.data.split()
		self.Pos = Pose(Point(float(self.temp[2]),float(self.temp[4]),float(self.temp[6])), Quaternion(float(self.temp[9]),float(self.temp[11]),float(self.temp[13]),float(self.temp[15])))
		self.pub_pos.publish(self.Pos)
		
		
if __name__ == '__main__':
	rospy.init_node('kamerider_client')
	kamerider_client()
	rospy.spin()

