/*
 * 文件: externally-driven-sim.h
 *
 * 版本: 2.0
 *
 * 描述：通过tcp套接字外部驱动的仿真器应用。头文件
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

#ifndef EXTERNALLY_DRIVEN_SIM_H
#define EXTERNALLY_DRIVEN_SIM_H

#include "ns3/simulator-impl.h"
#include "ns3/scheduler.h"
#include "ns3/event-impl.h"
#include "ns3/system-thread.h"
#include "ns3/system-mutex.h"
#include "ns3/default-simulator-impl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <ns3/ipv4-address.h>
#include "src/applications/model/udp-echo-client-new.h"
#include "src/applications/model/udp-echo-server-new.h"
#include "src/applications/model/tcp-server.h"
#include "src/applications/model/tcp-client.h"
#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include <list>

/**
 * \file
 * \ingroup simulator
 * ns3::ExternallyDrivenSim的声明.
 */

namespace ns3 {
/**
 * \ingroup simulator
 *
 * 定义外部驱动仿真器的实现类 公有继承自DefaultSimulatorImpl类
 */
class ExternallyDrivenSim : public DefaultSimulatorImpl
{
public:
// 声明两个类UdpEchoServerNew和TcpServer为ExternallyDrivenSim的友元类，
// 这样可以在这两个类中使用ExternallyDrivenSim的私有方法和属性g_SimulationStep与
// g_qudong_mode
friend class UdpEchoServerNew;
friend class TcpServer;
/**
 *  Register this type.（在ns3中注册该类型）
 *  \return The object TypeId.（并返回该对象的TypeId）
 */
static TypeId GetTypeId (void);

/** 构造函数 */
ExternallyDrivenSim ();
// 声明ExternallyDrivenSim析构函数
virtual ~ExternallyDrivenSim ();

// 继承并重定义了Stop、Run、DoDispose、ProcessOneEvent方法
// 声明调度仿真停止事件即其时间的函数
virtual void Stop (Time const  &time);
// 声明调度整个仿真运行过程，安排发送事件与运行仿真事件的函数
virtual void Run (void);
// 函数声明 此函数从获得的事件中返回当前Event（事件）的EventId来设置某个保存EventId的变量
static EventId SetEventId(void);
// 函数声明 这个函数形成了发送给外部应用程序的通知即仿真结果（分别对应着UdpEchoServerNew类和TcpServer类两种应用程序）
static void GetNotices(UdpEchoServerNew* p);
static void GetNotices(TcpServer* p);
// 函数声明 此函数允许设置仿真的同步周期步长（仿真每个同步事件时间点的间隔）
static void SetSimulationStep(Time);
// 函数声明 此函数启动一个客户端类方法来安排新传送事件（不从外部应获得的节点索引来设定目标地址）
virtual void RunScheduleTransmit(void);
// 函数声明 此函数启动一个客户端类方法来安排新传送事件（从外部应获得的节点索引来设定目标地址，
// 并把此次节点发送的数据包内容当做参数）
virtual void RunScheduleTransmitTo(std::string PayLoad,int mutipac = -1,int statenum = -1,uint64_t delaysend = 0);
// 函数声明 此函数收集当前EventID的信息，即当前事件的信息
virtual void GetEventId(Ptr<EventImpl>m_eventImpl,uint64_t m_ts,uint32_t m_context,uint32_t m_uid);
// 函数声明 此函数将通知（一个仿真步长的仿真结果）传送给外部应用程序
virtual void TransmitNotices(void);
// 函数声明 此函数用于在一个仿真同步周期步长内部，当服务器接收到数据包时触发异步仿真事件标志（适用于事件驱动模式）
static void trans_noti_insam(void);
// 函数声明 此函数使用仿真脚本获得的节点数量变量来设置ExternallyDrivenSim的私有属性m_nodenumber
static void get_node_number(int nodenumber);
// 函数声明 此函数让UdpEchoServerNew应用获得外部应用程序发送过来的所有的控制系统目标节点ID以及
// 控制系统目标节点的数量
// 分别对应着UdpEchoServerNew类和TcpServer类两种应用程序
static void ser_get_des (UdpEchoServerNew* pp);
static void ser_get_des (TcpServer* pp);
// 函数声明 此函数使用仿真脚本获得的仿真驱动模式变量来设置ExternallyDrivenSim的私有属性g_qudong_mode
static void Setqudongmode(uint16_t qudong_mode);
// 函数声明 此函数使用仿真脚本获得的传输层模式变量来设置ExternallyDrivenSim的私有属性g_trans_protocol_mode
static void Set_trans_protocolmode(uint16_t trans_protocolmode);

private:
// 定义私有函数
// 声明 ExternallyDrivenSim析构函数实现 每个子类的析构函数应为空，其内容应移动到关联的DoDispose
virtual void DoDispose (void);
// 声明 此函数接收外部应用程序的通知并解析和显示
void Listen ();
// 声明 此函数利用节点的智能指针返回该节点上客户端应用的普通指针
// 分别对应着UdpEchoServerNew类和TcpServer类两种应用程序获取其对应的普通指针
UdpEchoClientNew* GetClient(Ptr<Node>);
TcpClient* GetClient_tcp(Ptr<Node> n);
// 声明 此函数利用节点的智能指针返回该节点的设备接口的IP地址
Ipv4Address GetDestIp(Ptr<Node>);
/* 声明推进下一个仿真事件的函数 */
void ProcessOneEvent (void);

// 定义私有属性
// 定义 同步事件时间点，仿真运行一个同步周期步长的时间上限
// 同步周期步长：每次同步事件时间点的间隔
static uint64_t m_TimeLimit;
// 定义tcp套接字listener
int listener;
// 定义网络接口套接字的地址
struct sockaddr_in addr;
// 定义接受连接后的新的子套接字
int m_sock;
// 定义仿真结束的时间，结束时间从Simulator::Stop(Seconds(m_totalTime))来获取
Time m_TimeOfEnd;
// 定义仿真停止的标志，当仿真结束或者仿真需要人为停止时需要置为true
bool m_quit;
// 定义套接字从外部应用客户端读取到的通讯字节流的字节数
int bytes_read;
// 定义发送数据包的节点的ID
uint32_t m_nodId;
// 属性定义 如果有人想让客户端发送数据包以某一相同时间推迟一段时间，那么可以给该变量赋值（默认为0，不推迟）
uint64_t m_timeOfStep;
// 定义保存客户端应用的智能指针（分别对应着UdpEchoServerNew类和TcpServer类两种应用程序）
Ptr<UdpEchoClientNew> m_pucl;
Ptr<TcpClient> m_pucl_tcp;
// 定义被发送（接收）数据包的节点的ID
uint32_t m_nodeIdTo;
// 定义保存获取的节点客户端应用程序的属性
Ptr<Application> m_applic;
// 定义根据节点ID返回的对应节点的智能指针
Ptr<Node> m_nod;
// 定义从节点客户端应用程序获得的需要发送的内容
std::string m_PayLoad;
// 定义异步仿真事件发生的标志
static bool in_sam_trans;
// 定义从仿真脚本获得的同步周期步长(每个同步事件时间点的间隔)
static Time g_SimulationStep;
// 定义从仿真脚本获得的仿真驱动模式
static uint16_t g_qudong_mode;
// 定义从仿真脚本获得的节点个数
static int m_nodenumber;
// 定义从仿真脚本获得的传输层协议模式
static uint16_t g_trans_protocol_mode;
NodeContainer m_nodes;

};

} // namespace ns3

#endif /* EXTERNALLY_DRIVEN_SIM_H */
