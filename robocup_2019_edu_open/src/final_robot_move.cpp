#include <ros/ros.h>
#include <std_msgs/String.h>
#include <vector>

#include <sys/stat.h>
#include <termios.h>
//#include <term.h>
#include <unistd.h>
  
//socket
#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

#include <sstream>
#include <stdlib.h>
#include <string>

#define SERVER_PORT    30000
#define LENGTH_OF_LISTEN_QUEUE 20
using namespace std;
ros::Publisher voice_pub;
std_msgs::String say;
void ssvepcb(const std_msgs::String::ConstPtr& msg);
int main(int argc, char** argv){
    ros::init(argc, argv, "robot_move");
    ros::NodeHandle nh_;
    voice_pub = nh_.advertise<std_msgs::String>("/socket", 1);
    ros::Subscriber move_sub = nh_.subscribe("socket2topic_nd", 1000, ssvepcb);
    ros::spin();
    return 0;
}
void ssvepcb(const std_msgs::String::ConstPtr& msg){
    char cmd = 0;
    string value = msg->data;
    cmd = value[0];
    say.data = value;
    voice_pub.publish(say);
}
