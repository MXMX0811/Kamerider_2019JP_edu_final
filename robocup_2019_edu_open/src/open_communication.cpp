/*用于进行人机交互，利用建立线段树来维护我们的查询库*/
#include<iostream>
#include<string>
#include<string.h>
#include<fstream>
#include<ctime>
#include<cstdlib>
#include<ros/ros.h>
#include<std_msgs/String.h>
#include<sound_play/SoundRequestActionGoal.h>
using namespace std;
#define ls(x) x<<1
#define rs(x) x<<1|1
const int maxn=200;//数据集，包括应该有的样本
/*-----实现语音中的字符串分割----------------------------*/
void stringSplit(string s,char splitchar,vector<string>& vec){
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

/*信号传输数据，可以考虑再度优化一下，目前的太弱了,线段树进行数据处理,程序鲁棒性出现阶段性问题，可以考虑测试新的程序，主要还是线段树的想法*/
struct signal_data
{
    bool data;
    bool vaild;
};
struct node//基本的信息
{
	string problem;
	string ans;
	int lson;
	int rson;
	int pa;
	int num;//请输入具体人数，建立树和查询树不需要一样，只需要知道大概长什么样就好了
	int height;//可以分级,我树根算1

}tree[(maxn<<2)+7];
int vis[(maxn<<2)+7];
string data[maxn+2];
int num;
void refresh(int root)//每次更新线段树
{

	if(tree[root].lson==0&&tree[root].rson==0)//叶子节点
	{
		num++;
		return;
	}
	else
	{
		refresh(ls(root));
		refresh(rs(root));
		tree[root].num = tree[ls(root)].num+tree[rs(root)].num;
	}

}
void dfs(int root)//最简单直接暴力存树就好了，因为树的大小有限所以可以直接暴力存,否则就要二叉树的重建了
{
	if(tree[root].lson==0&&tree[root].rson==0)
	{
		cout<<tree[root].ans<<endl;
		return ;//必须返回
	}
	else
	{
		dfs(ls(root));
		cout<<tree[root].problem<<endl;
		dfs(rs(root));
	}
	return ;
}
void record(int root)//文件的读写非常关键
{
	ofstream fout;
	fout.open("/home/zmx/catkin_ws/src/robocup_2019_edu_open/src/data.txt");
	for(int i=1;i<=(maxn<<2);i++)//涉及到字符串的读入操作。如何读入一个字符串还是个问题。。字符串如何完整读入解决这个就好了
	{
		if(tree[i].lson!=0&&tree[i].rson!=0)//叶子节点
			fout<<i<<" "<<tree[i].problem<<endl<<tree[i].lson<<" "<<tree[i].rson<<" "<<tree[i].num<<endl;
		else
		fout<<i<<" "<<tree[i].ans<<endl<<tree[i].lson<<" "<<tree[i].rson<<" "<<tree[i].num<<endl;//特殊标记
	}
	system("clear");
	cout<<"Record completed"<<endl;
	return ;
}
void build_tree(int root)
{
	ifstream fin;
	fin.open("/home/zmx/catkin_ws/src/robocup_2019_edu_open/src/new_data.txt");
	for(int i=1;i<=(maxn<<2);i++)//文件没有到末尾,不会到达末尾的。。
	{
		
		int id;
		string temp;
		int lson;
		int rson;
		int num;
		fin>>id;
		fin.get();//果然可以清空
		getline(fin,temp,'\n');//他会把\n吞掉
		fin>>lson;
		fin>>rson;
		fin>>num;
		if(lson==0&&rson==0)
		{
			tree[id].ans=temp;
			tree[id].lson=tree[id].rson=0;
		}
		else
		{
			tree[id].problem=temp;
			tree[id].lson=lson;
			tree[id].rson=rson;
		}
		tree[id].num=num;
	}
}
void build(int root)
{
	string ss;//这个记录太深了万一有一个错了就都完了呵呵呵,每次到达最下面都记录一次
	int num;
	if(tree[root].num==1)//叶子节点已经记录完了
	{	

		cout<<tree[root].ans<<endl;
		return ;
	}
	if(tree[root].num!=0)//已经确定了
	{
		cout<<"如果"<<tree[root].problem<<"是对的"<<endl;
		build(ls(root));
		cout<<"如果"<<tree[root].problem<<"是错的"<<endl;
		build(rs(root));
		return ;//很关键,及时return防止无效分支
	}
	cout<<"还有多少人？"<<endl;
	cin>>num;
	tree[root].num=num;
	cin.ignore();
	if(tree[root].num==1)
	{
		cout<<"输入答案"<<endl;
		getline(cin,ss);
		tree[root].ans=ss;
		record(1);
		return ;
	}	
	else
	{
		
		cout<<"输入一个新的分类问题"<<endl;
		getline(cin,ss);
		tree[root].problem=ss;
		tree[root].lson=ls(root);//也就是只有问题会到这里来.
		tree[root].rson=rs(root);
		tree[ls(root)].pa=root;
		tree[rs(root)].pa=root;
		cout<<"如果"<<tree[root].problem<<"是对的"<<endl;
		build(ls(root));
		cout<<"如果"<<tree[root].problem<<"是错的"<<endl;
		build(rs(root));
    }
}
/*用来实现机器人交互的过程*/
class open
{
public:
    open();
    void speak(std::string str);
    void signal_cb(const std_msgs::StringConstPtr & msg);
    void query(int root);
    void solve();
    bool get_signal();
    void cmd_cb(const std_msgs::StringConstPtr & msg);
    void wait_for_cmd();
    void send_cmd(string str_cmd);
private:
    ros::NodeHandle nh;
    ros::Rate loop_rate;
    ros::Publisher pub_voice;
    ros::Publisher pub_sig;
    ros::Publisher pub_answer;
    ros::Publisher pub_cmd;

    ros::Subscriber sub_signal;
    ros::Subscriber sub_cmd;
    signal_data signal;
    std_msgs::String answer;
    std_msgs::String str_pub;
    string cmd;
    int root;
    bool finish;
    bool start;
};
/*
signal 内部信号，是脑电信号转换之后的信号
answer 是答案传输之后的信号 一定要确定答案已经传输
*/
open::open():loop_rate(10)
{
    sub_signal = nh.subscribe("socket2topic_nd", 1, &open::signal_cb,this);
    sub_cmd = nh.subscribe("control_command",1,&open::cmd_cb,this);

    pub_voice = nh.advertise<sound_play::SoundRequestActionGoal>("/sound_play/goal", 1);
    pub_sig = nh.advertise<std_msgs::String>("signal",10);
    pub_answer = nh.advertise<std_msgs::String>("/socket/target",1000);
    pub_cmd = nh.advertise<std_msgs::String>("control_command",1000);
    cmd = "";
    speak("1111");
    speak(" ");
    root = 1;
    finish = false;
    signal.vaild = false;
    start = false;
    //准备去有效地点
    str_pub.data = "go to the serve place";
    pub_cmd.publish(str_pub);
    wait_for_cmd();
}
void open::send_cmd(string str_cmd)
{   
       str_pub.data = str_cmd;
       pub_cmd.publish(str_pub);

}
void open::wait_for_cmd()
{
    cout<<"now i will wait for your message!"<<endl;
    while(ros::ok())
    {
        if(is_equal(cmd,"start communication"))
        {
            start = true;
            break;
        }
        loop_rate.sleep();
        ros::spinOnce();
    }
    if(is_equal(cmd,"start communication"))
    {
        solve();
    }
    return ;

}
void open::cmd_cb(const std_msgs::StringConstPtr & msg)
{
    cmd = msg->data;
    
}
void open::speak(std::string str)
{
    sound_play::SoundRequestActionGoal voice;
    voice.goal.sound_request.arg = str.c_str();
    voice.goal.sound_request.arg2 = "voice_kal_diphone";
    voice.goal.sound_request.sound = -3;
    voice.goal.sound_request.volume = 1.0;
    voice.goal.sound_request.command = 1;
    pub_voice.publish(voice);
}
//每次都能深入一层
void open::signal_cb(const std_msgs::StringConstPtr & msg)
{
     if(finish)
     return; 
     if(!start)
     return;
     std::string signal_str = msg->data;
     if(is_equal(signal_str,"b"))
     {
        signal.vaild = true;
        signal.data = true;
     }
     else if(is_equal(signal_str,"c"))
     {
        signal.vaild = true;
        signal.data = false;
     }
     else
     {
        signal.vaild = false;
        signal.data = false;
     }
}
bool open::get_signal()
{
    
    while(1)//没有收到信号
        {
            //等待信息
            if(signal.vaild==true)
            {   
                signal.vaild = false;
                return signal.data;
            }          
            loop_rate.sleep();
            ros::spinOnce();
        }
}
//这里就是一个建立树的过程,基本上就一个查询就可以学习到,可以建立新的询问,核心节点
void open::query(int root)//这里就是一个建立树的过程,基本上就一个查询就可以学习到,可以建立新的询问
{
	bool temp;//cin 和getline不要混合使用
    //最终到达叶子节点
	if(tree[root].lson==0&&tree[root].rson==0)
	{
		cout<<"You want "<<tree[root].ans<<endl;//可以想想怎么记录下来,输出出来如何再度学习
        speak("i guess you want" + tree[root].ans + "am i right?");
        sleep(3);
        send_cmd("continue ask");
		//确认是否想要这个东西，可以回去一个阶段
        temp = get_signal();
		if(temp==true)
		{
			system("clear");
			cout<<"finish understanding what you want to say!"<<endl;
            finish = true;
            str_pub.data = "go to the shop";
            speak("now i will get water for you ");
            answer.data = "bottle";
            pub_cmd.publish(str_pub);
            pub_answer.publish(answer);
			return ;
		}
		else if(temp==false)
		{
			refresh(1);
			query(root/2);
			return ;
		}
	}
    send_cmd("continue ask");
	cout<<tree[root].problem<<endl;
    speak(tree[root].problem);
	temp = get_signal();
	if(temp==true)
	{
		cout<<"Already excluded "<<tree[rs(root)].num<<" Choice"<<endl;
		cout<<"There are still left "<<tree[ls(root)].num<<" Choice"<<endl;
		query(ls(root));//做好标准左边Y右边N
	}
	else if(temp==false)
	{
		cout<<"Already excluded "<<tree[ls(root)].num<<" Choice"<<endl;
		cout<<"There are still left "<<tree[rs(root)].num<<" Choice"<<endl;
		query(rs(root));
	}	
	return ;
}
//基本上到了之后直接刷新
void open::solve()
{
    build_tree(1);
    speak("now i will guess what you want!");
    query(1);
}
/*主函数，考虑多线程*/
int main(int argc, char *argv[])
{   
        cout<<"start communication"<<endl;
        ros::init(argc, argv, "open_communication");
		open test;
  
		return 0;
}
