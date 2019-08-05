#include"ros/ros.h"
#include"std_msgs/String.h"
#include<string.h>
#include<strings.h>
#include<sstream>
#include<sound_play/SoundRequestActionGoal.h>

using namespace std;

void stringSplit(string s,char splitchar,vector<string>& vec)
{
    if(vec.size()>0)
        vec.clear();
    int length=s.length();
    int start=0;
    for(int i=0;i<length;i++){
        if(s[i]==splitchar && i==0)
            start+=1;
        else if(s[i]==splitchar){
            vec.push_back(s.substr(start,i-start));
            start=i+1;
        }
        else if(i==length-1){
            vec.push_back(s.substr(start,i+1-start));
        }
    }
}
void clear_str(string & datastr)
{
    for(int i=0;i<datastr.length();i++)
    {
         if(datastr[i]>='a'&&datastr[i]<='z'||datastr[i]>='A'&&datastr[i]<='Z'||datastr[i]==' ')
         {
             if(datastr[i]>='A'&&datastr[i]<='Z')
             {
                 datastr[i] = datastr[i] - 'A' + 'a';//小写化
             }
             continue;
         }
         else 
         {
             datastr.erase(i,1);
         }
    }
}
bool in_the_string(string target,string words)
{
    std::vector<string> res;
    clear_str(target);
    stringSplit(target,' ',res);

    std::vector<string>::iterator result = find(res.begin(),res.end(),words);
    if(result==res.end())
    {

        return false;
    }
    else
    {

        return true;
    }
}
void clear_string(string & datastr)
{
    for(int i=0;i<datastr.length();i++)
    {
         if(datastr[i]>='a'&&datastr[i]<='z'||datastr[i]>='A'&&datastr[i]<='Z')
         {
             if(datastr[i]>='A'&&datastr[i]<='Z')
             {
                 datastr[i] = datastr[i] - 'A' + 'a';//小写化
             }
             continue;
         }
         else 
         {
             datastr.erase(i,1);
         }
    }
}
bool is_equal(string a,string b)
{
    clear_string(a);
    clear_string(b);
    // cout<<a<<endl<<a.length()<<endl;
    // cout<<b<<endl<<b.length()<<endl;
    if(a.length()!=b.length())
    return false;
    for(int i=0;i<a.length();i++)
    {
        if(a[i]!=b[i])
        return false;
        
    }
    return true;
}

class open
{
    public:
    open();
    void send_msg(string topic_name,string msg);
    void wait_time(int time);
    void solve();
    void cmd_cb(const std_msgs::StringConstPtr & msg);
    void speak(std::string str);
    void wechat_cb(const std_msgs::StringConstPtr & msg);
    private:
    ros::Publisher pub_cmd;
    ros::Publisher pub_nd;
    ros::Publisher pub_voice;

    sound_play::SoundRequestActionGoal voice;
    ros::Subscriber sub_cmd;
    ros::Subscriber sub_wechat;
    ros::NodeHandle nh;
    ros::Rate loop_rate;
    std::string str_pub;
    std_msgs::String str_pub_msg;
    string wechat_str;
    int counter ;
    bool get_wechat;

};

open::open():loop_rate(1)
{
        counter = 1 ;
        wechat_str = "";
        pub_cmd =  nh.advertise<std_msgs::String>("control_command",1);     
        pub_nd = nh.advertise<std_msgs::String>("socket2topic_nd",1);
        pub_voice = nh.advertise<sound_play::SoundRequestActionGoal>("/sound_play/goal", 1);
        sub_cmd = nh.subscribe("/control_command", 1,&open::cmd_cb,this);
        get_wechat = false;
        sub_wechat = nh.subscribe("/wechat2total", 1,&open::wechat_cb,this);

}
void open::speak(std::string str)
{
    voice.goal.sound_request.arg = str.c_str();
    voice.goal.sound_request.arg2 = "voice_kal_diphone";
    voice.goal.sound_request.sound = -3;
    voice.goal.sound_request.volume = 1.0;
    voice.goal.sound_request.command = 1;
    pub_voice.publish(voice);
}

void open::wechat_cb(const std_msgs::StringConstPtr & msg)
{
    wechat_str = msg->data;
    get_wechat = true;

    speak("master i get a wechat :"+wechat_str);
    wait_time(10);
    speak("do you want to prepare something for your friend?");

    send_msg("control_command","start communication");//start open 主要是开启导航

}

void open::cmd_cb(const std_msgs::StringConstPtr & msg)
{

    string ss = msg->data;//这里是我们的反馈控制，可以加上对应的
    //开始导航,您有一个朋友想要来看你，然后你应该去交流问是否要为他准备一些东西，收到了微信
    //yes no no yes
    
    if(is_equal(ss,"continue ask"))//communication
    {
        wait_time(10);
        if(counter == 1)
        {
            send_msg("socket2topic_nd","b");    
        }    
        else if(counter == 2 ) 
        {
            send_msg("socket2topic_nd","c");
        }
        else if(counter==3)
        {
            send_msg("socket2topic_nd","c");    
        }
        else if(counter ==4)
        {
            send_msg("socket2topic_nd","b");    
        }

        counter ++;
    }


}
void open::wait_time(int time)
{
    int timer = 0;
    while(ros::ok())
    {
        if(timer>time)
            break;
        cout<<timer<<endl;
        timer = timer + 1;
        loop_rate.sleep();
        ros::spinOnce();
    }
}
void open::send_msg(string topic_name,string msg)
{

    str_pub_msg.data = msg;
    ROS_INFO("%s",str_pub_msg.data.c_str());
    if(is_equal(topic_name,"control_command"))
    {
        pub_cmd.publish(str_pub_msg);
    }
    else if(is_equal(topic_name,"socket2topic_nd"))
    {
        pub_nd.publish(str_pub_msg);
    }
}
int main(int argc, char *argv[])
{
    ros::init(argc, argv, "procedure_control");
    open temp;//通过类似全局变量的机制
    int timer = 0;
    ros::Rate loop_rate(1);
    while(ros::ok())
    {
        loop_rate.sleep();
        ros::spinOnce();
    }
    return 0;
}
