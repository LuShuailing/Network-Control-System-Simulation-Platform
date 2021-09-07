/*
 * 文件: struct.h
 *
 * 版本: 2.0
 *
 * 描述：通过tcp套接字外部驱动的仿真器应用给matlab发送的结构体。头文件
 *
 * 作者: 卢帅领
 *
 * E-mail: lushuailingfight@163.com
 *
 * Date:2020.11.1
 *
 * 该软件被开发用于NCS仿真
 * NCS: network control system（网络控制系统）
 *
 */

#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>
#include<string>

// ns3仿真中，定义结构体Data，保存的是两次与matlab交互之间收集到的某节点的信息，
// 并在后一次交互时发送给matlab的客户端
struct Data_MATLAB
{
int sig;// 该节点在这个时间段内是否接收到过数据包
int des;
char packetcontent[900];// 该节点在这个时间段内接收到的数据包的内容
bool sys_send_node;
int pos_vel_send_sig;
double pos;
double vel;
};

struct Data_NS3
{
int sig;// 该节点在这个时间段内是否接收到过数据包
char packetcontent[8000];// 该节点在这个时间段内接收到的数据包的内容
char Timedelay[500];// 该节点在这个时间段内接收到的数据包的时延
char PacketNum[200];// 该节点在这个时间段内接收到的数据包的编号
char Sim_time[500];// 该节点在这个时间段内接收到的数据包的接收时间
char Send_time[500];
char TimedelayReal[500];
};

#endif /* STRUCT_H */
