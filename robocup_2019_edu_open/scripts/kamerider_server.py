#!/usr/bin/python
# -*- coding: UTF-8 -*-
import socket               # 导入 socket 模块
import rospy
from std_msgs.msg import String
from geometry_msgs.msg import Pose
 
class kamerider_server():
    def __init__(self):
        self.kamerider_1_ip = '172.20.10.12'
        self.kamerider_1_port = 40000
        self.kamerider_3_ip = '172.20.10.12'
        self.kamerider_3_port = 40001
        self.socket_1 = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.socket_1.bind((self.kamerider_1_ip, self.kamerider_1_port))
        self.socket_3 = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.socket_3.bind((self.kamerider_3_ip, self.kamerider_3_port))
        self.socket_1.listen(5)  
        self.c1, self.addr1 = self.socket_1.accept()
        self.socket_3.listen(5)  
        self.c3, self.addr3 = self.socket_3.accept()

        rospy.Subscriber('/image/target', String, self.target_callback)        
        rospy.Subscriber('/kamerider_1/navi/input', Pose, self.kamerider_1_callback)
        rospy.Subscriber('/kamerider_3/navi/input', Pose, self.kamerider_3_callback)
        
        self.img_res_3=rospy.Publisher("/kamerider_3/image/result",String,queue_size=15)
        self.img_res_1=rospy.Publisher("/kamerider_1/image/result",String,queue_size=15)
        
        
    def target_callback(self, msg):
        print(msg.data)
        print('socket1:', self.addr1)
        self.c1.send(str(msg.data))
        
        print('socket3:', self.addr3)
        self.c3.send(msg.data)
        
    def kamerider_1_callback(self, msg):
        print(str(msg))
        print('socket1:', self.addr1)
        self.c1.send(str(msg))
        self.img_res_1_pub()

    def kamerider_3_callback(self, msg):
        print(str(msg))
        print('socket3:', self.addr3)
        self.c3.send(str(msg))
        print(14235235)
        self.img_res_3_pub()
        
    def img_res_1_pub(self):
        data = self.c1.recv(1024)
        self.img_res_1.publish(data)
        print(data)     
            
    def img_res_3_pub(self):
        data = self.c3.recv(1024)
        self.img_res_3.publish(data)
        print(data)     
  


if __name__ == '__main__':
    rospy.init_node('kamerider_server')
    kamerider_server()
    rospy.spin()


