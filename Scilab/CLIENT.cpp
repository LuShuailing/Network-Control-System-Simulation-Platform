
 //作者：卢帅领
//日期：2020.11

#include "mex.h"
#include </home/ling/Scilab/scilab-6.1.0/cosimulator/struct.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <resolv.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <string.h>
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
	//定义从scilab工作区间获取的变量指针
	mxArray *nodenum = NULL;
	mxArray *packetct = NULL;
	mxArray *packetcte = NULL;
	mxArray *order_sys = NULL;
	//int ns3_send=0;
	order_sys = mexGetVariable("base","order");
	double* Order_sys = mxGetPr(order_sys);
    	int Order = *Order_sys;
	//从scilab空间获取需要发送的节点数
	nodenum = mexGetVariable("base","nodenumber");
    	double* Nodenum = mxGetPr(nodenum);
    	int Nodenumber = *Nodenum;
    	std::cout<<"Nodenumber is "<<Nodenumber<<std::endl;
    	
    	//从scilab空间获取需要发送的内容C_A_data
    	std::string Packetcontent11=" ";
    	for(int i=0;i<Order;i++)
    	{
	packetct = mexGetVariable("base","C_A_data");
    	double* Packetct = mxGetPr(packetct);
    	std::string Packetcontent1=std::to_string(*(Packetct+i));
    	if (Packetcontent11==" ")
    	{
    		Packetcontent11=Packetcontent1;
    		
    	}
    	else
    	{
    		Packetcontent11=Packetcontent11+" "+Packetcontent1;
    	
    	}
    	}
    	char Packetcontent12[900];
	strcpy(Packetcontent12,Packetcontent11.c_str());
    	std::cout<<"Packetcontent12 is "<<Packetcontent12<<std::endl;
    	
    	//从scilab空间获取需要发送的内容S_C_data，设计可扩展接口
    	std::string Packetcontent13=" ";
    	for (int i=0;i<Order;i++)
    	{
    	packetcte = mexGetVariable("base","S_C_data");
    	double* Packetcte = mxGetPr(packetcte);
    	std::string Packetcontent=std::to_string(*(Packetcte+i));
    	//std::string Packetcontent12=std::to_string(*(Packetcte+1));	
    	//std::cout<<"packetcte(getvariable) is "<<Packetcontent11+" "+Packetcontent12<<std::endl;
    	if (Packetcontent13==" ")
    	{
    		Packetcontent13=Packetcontent;
    		//std::cout<<Packetcontent<<" ";
    	}
    	else
    	{
    		Packetcontent13=Packetcontent13+" "+Packetcontent;
    		//std::cout<<Packetcontent<<" ";
    	}
    	}
    	std::cout<<std::endl;
	char Packetcontente[900];
	strcpy(Packetcontente,Packetcontent13.c_str());
    	std::cout<<"Packetcontente is "<<Packetcontent13<<std::endl;
	
	
	
	
	//创建socket
	int sock;
	struct sockaddr_in addr;
	struct Data_MATLAB trans[Nodenumber];
	struct Data_NS3 rec[Nodenumber];
	//std::string lastTime = "0";
	bool send_pac = false;


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
	perror("socket");
	exit(1);
	}
	addr.sin_family = AF_INET;//设置协议族
	addr.sin_port = htons(3425);//绑定端口
	addr.sin_addr.s_addr = htons(INADDR_ANY);//绑定本机地址
	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
	perror("connect");
	exit(2);
	}
	for (; ;)
	{
		/*std::string line;
		char file[] = "/home/ling/Scilab/scilab-6.1.0/client/nodeers.txt";
		std::ifstream myfile(file);
		std::getline(myfile, line);
		std::string nextTime = line;
		if(nextTime != lastTime)
		{
		*/
		
		
		/*std::getline(myfile, line);
		std::string str_node1send = line;
		int size_node1send = strlen(str_node1send.c_str())+1;
		char node1send[size_node1send];
		strcpy(node1send,str_node1send.c_str());
          std::getline(myfile, line);
		std::string str_node2send = line;
		uint16_t size_node2send = strlen(str_node2send.c_str())+1;
		char node2send[size_node2send];
		strcpy(node2send,str_node2send.c_str());
		*/
	// instruction to filling trans structure.
	
		//定义发送的结构体
		int size_sbuf =0;
		size_sbuf = sizeof(trans);
		char snd_buf[size_sbuf];
		memset(&trans,0,sizeof(trans));
		//设置节点是否发送的标志，sig=1为发送节点
		trans[0].sig = 1;
		trans[1].sig = 0;
		trans[2].sig = 0;
		trans[3].sig = 1;
		//trans[4].sig = 1;
		//trans[5].sig = 1;
		//trans[6].sig = 0;
		//trans[7].sig = 0;
		
		//设置目标节点
		trans[0].des = 2;
		//trans[1].des = 0;
		//trans[2].des = 3;
		trans[3].des = 1;
		//trans[4].des = 3;
		//trans[5].des = 2;
		//trans[6].des = 1;
		//trans[7].des = 0;
		//定义发送包的内容
		strcpy(trans[0].packetcontent,Packetcontent12);
		strcpy(trans[3].packetcontent,Packetcontente);
		//strcpy(trans[2].packetcontent, "");
		//strcpy(trans[3].packetcontent, "");
		//strcpy(trans[4].packetcontent, "day!  day!  day!!");
		//strcpy(trans[5].packetcontent, "I love sleep");
		//strcpy(trans[6].packetcontent, "it's too difficult");
		//strcpy(trans[7].packetcontent, "yayale you yinyang ya");
		trans[0].sys_send_node = 1;
		trans[1].sys_send_node = 1;
		trans[2].sys_send_node = 1;
		trans[3].sys_send_node = 1;
		//trans[4].sys_send_node = 1;
		//trans[5].sys_send_node = 1;
		//trans[6].sys_send_node = 1;
		//trans[7].sys_send_node = 1;
		
		
		//清空snd_buf，准备socket载体
		memset(snd_buf,0,sizeof(snd_buf));
		memcpy(snd_buf,trans,sizeof(snd_buf));       
		std::cout<<"send"<<snd_buf<<std::endl;
	        send(sock, &snd_buf, sizeof(snd_buf), 0);//给NS3发送通知
		//lastTime = nextTime;
		send_pac = true;
		//}
		//else
		{
		if(send_pac == true)
		{
		    int size_rbuf;
		    size_rbuf = sizeof(rec);
		    char recv_buf[size_rbuf];
		    memset(recv_buf,0,sizeof(recv_buf));
		    recv(sock, &recv_buf, sizeof(recv_buf), 0);
		
		
		
		//std::cout<<"ns3_send:"<<ns3_send<<"sock:"<<sock<<std::endl;
		    memset(&rec,0,sizeof(rec));
		    memcpy(&rec,recv_buf,sizeof(recv_buf));
		    //if((rec.sig[0]||rec.sig[1]) == 0)
		    	//{
		    	//break;
		    	//}
		//instructions to analyzing
		
		//打印接受到的数据包的时刻
                std::cout<<"每个节点接收的数据包的发送时刻  单位（s） "<<"\n" <<std::endl;
                for(int i = 0;i<Nodenumber-1;i++)
                {
                    std::cout<<"节点"<<i<<"接受包的发送时刻："<<rec[i].Send_time<<"\n" <<std::endl;
                }
                std::cout<<"节点"<<Nodenumber-1<<"接受包的发送时刻："<<rec[Nodenumber-1].Send_time<<"\n" <<std::endl;
                
                
                
                
		//打印接受到的数据包内容
		 std::cout<<"接收到NS信息，每个节点接收数据包内容 "<<"\n" <<std::endl;
                for(int i = 0;i<Nodenumber-1;i++)
                {
                std::cout<<"节点"<<i<<"内容:"<<rec[i].packetcontent<<"\n" <<std::endl;
                }
                std::cout<<"节点"<<Nodenumber-1<<"内容:"<<rec[Nodenumber-1].packetcontent<<"\n" <<std::endl;
                
                //打印接受到的数据包延迟
                std::cout<<"每个节点接收的数据包的延迟  单位（s） "<<"\n" <<std::endl;
                for(int i = 0;i<Nodenumber-1;i++)
                {
                    std::cout<<"节点"<<i<<"时间延迟："<<rec[i].Timedelay<<"\n" <<std::endl;
                }
                std::cout<<"节点"<<Nodenumber-1<<"时间延迟："<<rec[Nodenumber-1].Timedelay<<"\n" <<std::endl;
                
                //打印接受到的数据包真实延迟
		/*std::cout<<"每个节点接收的数据包的真实延迟  单位（s） "<<"\n" <<std::endl;
                for(int i = 0;i<Nodenumber-1;i++)
                {
                    std::cout<<"节点"<<i<<"真实延迟："<<rec[i].TimedelayReal<<"\n" <<std::endl;
                }
                std::cout<<"节点"<<Nodenumber-1<<"真实延迟："<<rec[Nodenumber-1].TimedelayReal<<"\n" <<std::endl;
                */
			/*if(rec.sig[0]==0&&rec.sig[1]==0&&rec.sig[2]==0&&rec.sig[3]==0&&rec.sig[4]==0)
			{
				std::cout << " don't receive any packet " << std::endl;
			}
			else
			{
		std::cout<<rec.packetcontent[0] << " and " << rec.packetcontent[1] << " and " <<rec.packetcontent[2] << " and " << rec.packetcontent[3] <<" and " <<rec.packetcontent[4] <<"\n" <<std::endl;
		std::cout<<"timedelay is"<<rec.Timedelay[0]<< " and " <<rec.Timedelay[1]<<" and "<<rec.Timedelay[2]<<" and "<<rec.Timedelay[3]<<" and "<<rec.Timedelay[4]<<"\n"<<std::endl;*/
		
		/*if(ns3_send<45000)
		{
		
		 //strcpy(rec[1].packetcontent,"0");
                strcpy(rec[1].packetcontent,"0");
                strcpy(rec[1].Timedelay,"0");
                //strcpy(rec[1].Timedelay,"0");
                    
                
              
                    
                
		
		
		rec[Nodenumber]={0};
		std::cout<<"ns3_send<45000  内容初始为"<<rec[1].packetcontent<<"\n" <<std::endl;
		}
		*/
		std::string node2recv;
		std::string q2q;
		std::string node2recv_delay;
		
		//在文件node2recv中分别获取延迟和数据
		if(strcmp(rec[2].packetcontent,"")==0)
		{
		std::cout<<"节点2接受数据为空"<<std::endl;
		node2recv="9999";
		std::ofstream node2receive;//创建输出流对象node2receive
		std::ofstream node2receive_delay;//创建输出流对象node2receive_delay
		node2receive.open("node2receive.txt", std::ios::ate|std::ios::out);//创建并打开txt文件
		node2receive_delay.open("node2receive_delay.txt", std::ios::ate|std::ios::out);
		if (!node2receive.is_open())
				{
				std::cout << " node2receive don't exist " << std::endl;
				}
				else
				{
					node2receive << node2recv << std::endl;//文件与变量建立连接
					node2receive.close();
				}
		
		
		
		}
		else
		{
		
		//在文件node2recv中分别获取延迟和数据
		
		q2q =std::string((char*)rec[2].packetcontent);
		}
		
		/*char res[];
		getParaFromStrInstruction(q2q,3,res);
		std::cout<<"res"<<res<<std::endl;*/
		
		
		
		int pose=0;//定义位置变量并初始化
		pose=q2q.find("   ");//获取” ”出现的位置
		if(-1 == pose) //pos=-1说明没有找到 “ ”
		{
		return;
		}
		node2recv=q2q.substr(0,pose);//获得字符串q2q中从第0位开始的长度为pose的字符串
		node2recv_delay=q2q.substr(pose+3);
		
		std::cout<<node2recv<<std::endl;
		
		
		
		//AnsiString s ="123 456";
		//if(s.Pos(" "))s=s.SubString(1,s.Pos(" ")-1);
		
		//node2recv=std::string((char*)rec[2].packetcontent);
		//node2recv_delay = std::string((char*)rec[2].Timedelay);
		
		std::ofstream node2receive;//创建输出流对象node2receive
		std::ofstream node2receive_delay;//创建输出流对象node2receive_delay
		node2receive.open("node2receive.txt", std::ios::ate|std::ios::out);//创建并打开txt文件
		node2receive_delay.open("node2receive_delay.txt", std::ios::ate|std::ios::out);
		if (!node2receive.is_open())
				{
				std::cout << " node2receive don't exist " << std::endl;
				}
				else
				{
					node2receive << node2recv << std::endl;//文件与变量建立连接
					node2receive.close();
				}
		if (!node2receive_delay.is_open())
		{
			std::cout << " node2receive_delay don't exist " << std::endl;
						}
		else
						{
			node2receive_delay << node2recv_delay << std::endl;
			node2receive_delay.close();
						}
						
						
						
						
						
						
						
						
						
		//将节点2的接受节点0发送数据包的时间存入文件 node0send_time.txt中
		//创建变量储存接受时间send_time
		std::string node0sed_time;
		node0sed_time = std::string((char*)rec[2].Send_time);
		//构造输出流对象node0send_time
		std::ofstream node0send_time;
		node0send_time.open("node0send_time.txt",std::ios_base::out|std::ios_base::ate);//打开文件node0send_time.txt,使流对象与文件建立联系
		
		//向文件node0send_time.txt输出
		node0send_time<<node0sed_time<<std::endl;
		
		node0send_time.close();//关闭文件
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		//在文件node1recv中分别获取延迟和数据
        	std::string node1recv;
        	std::string q1q;
		std::string node1recv_delay;
		if(strcmp(rec[1].packetcontent,"")==0)
		{
		std::cout<<"节点1接受为空"<<std::endl;
		node1recv="9999";
		
		std::ofstream node1receive;
		std::ofstream node1receive_delay;
		node1receive.open("node1receive.txt", std::ios::ate|std::ios::out);
		node1receive_delay.open("node1receive_delay.txt", std::ios::ate|std::ios::out);
				if (!node1receive.is_open())
				{
				std::cout << " node1receive don't exist " << std::endl;
				}
				else
				{
				node1receive << node1recv << std::endl;
				node1receive.close();
				}
		
		
		}
		else
		{
		
		
		q1q = node1recv=std::string((char*)rec[1].packetcontent);
		}
		
		int pos=0;
		pos=q1q.find("   ");//获取” ”出现的位置
		if(-1 == pos) //pos=-1说明没有找到 “ ”
		{
		return;
		}
		node1recv=q1q.substr(0,pos);
		node1recv_delay=q1q.substr(pos+3);
		
		
		
		
		//node1recv=std::string((char*)rec[1].packetcontent);
		//node1recv_delay = std::string((char*)rec[1].Timedelay);
		std::ofstream node1receive;
		std::ofstream node1receive_delay;
		node1receive.open("node1receive.txt", std::ios::ate|std::ios::out);
		node1receive_delay.open("node1receive_delay.txt", std::ios::ate|std::ios::out);
				if (!node1receive.is_open())
				{
				std::cout << " node1receive don't exist " << std::endl;
				}
				else
				{
				node1receive << node1recv << std::endl;
				node1receive.close();
				}
				if (!node1receive_delay.is_open())
				{
			std::cout << " node1receive_delay don't exist " << std::endl;
				}
				else
				{
				node1receive_delay << node1recv_delay << std::endl;
				node1receive_delay.close();
				}
				
				
		//将节点1的接受节点3发送数据包的时间存入文件 node1receive_time.txt中
		//创建变量储存接受时间send_time
		
		//将节点1的接受节点3发送数据包的时间存入文件 node3send_time.txt中
		std::string node3sed_time;
		node3sed_time = std::string((char*)rec[1].Send_time);
		//构造输出流对象node3send_time
		std::ofstream node3send_time;
		node3send_time.open("node3send_time.txt",std::ios_base::out|std::ios_base::ate);//打开文件node3send_time.txt,使流对象与文件建立联系
		
		//向文件node3send_time.txt输出
		node3send_time<<node3sed_time<<std::endl;
		
		node3send_time.close();//关闭文件
		
				
				
				
				
				
			send_pac = false;
            break;
			}
			
            else
			{
		std::cout << " don't send and receive any packet " << std::endl;
			}
		}
	}
	{
	close(sock);
	}
  mxDestroyArray(nodenum);
  mxDestroyArray(packetct);
  mxDestroyArray(packetcte);
	return ;
}

