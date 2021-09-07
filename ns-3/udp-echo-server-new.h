/*
 * 文件: udp-echo-server-new.h
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的UDP服务器应用程序。头文件
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

#ifndef UDP_ECHO_SERVER_NEW_H
#define UDP_ECHO_SERVER_NEW_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/udp-echo-server.h"
#include <ns3/udp-socket.h>
#include <ns3/address-utils.h>
#include "ns3/application-container.h"

namespace ns3 {
class Socket;// 前置声明Socket这样可以声明定义Socket类函数参数和指针
// ns3::UdpEchoServerNew的定义 公有继承自UdpEchoServer类
class UdpEchoServerNew : public UdpEchoServer
{
public:
/**
 * \brief Get the type ID.
 * \return the object TypeId
 */
static TypeId GetTypeId (void);
// 声明UdpEchoServerNew类的构造函数
UdpEchoServerNew ();
// 声明UdpEchoServerNew类的析构函数
virtual ~UdpEchoServerNew ();
void SetDelayrec(Ptr<Packet> pac,InetSocketAddress Address_cli,InetSocketAddress Address_ser);
static void GetCliContainerp(ApplicationContainer* CliContainer_p);
// 定义UdpEchoServerNew类的数据包延迟属性
double packetdelay;
double packetdelayReal;
// 定义UdpEchoServerNew类的接收到的数据包内容属性
std::string RCdata;
// 定义UdpEchoServerNew类的接收到的数据包编号属性
uint32_t RCPacketNum;
// 属性定义 UdpEchoServerNew类用来保存所有要发送的控制系统目标节点ID的属性
int des[100];
// 属性定义 保存系统中被发送的控制系统目标节点的数量的属性
int sys_ser_des;
// 属性定义 保存控制系统目标节点接收到数据包的时间的属性
double sim_time;
double send_time;

private:
// 声明启动UdpEchoServerNew应用的函数 该函数将会在执行服务器启动事件时运行
virtual void StartApplication (void);
// 声明处理数据包的接收并读取数据包内容功能的函数 并在该函数中计划一个异步事件
void HandleRead (Ptr<Socket> socket);
// 函数声明 通过该函数计划一个异步事件来实现与matlab的交互
void Schedule_Asyn_event(void);
// 属性定义 当控制系统目标节点接收到数据包时，被计划的异步事件的eventid，在异步事件处会与matlab进行一次交互
EventId m_Asyn_event;
// 属性定义 为当前安装有服务器的节点的ID，即在节点容器中的索引
int64_t ser_id;
static ApplicationContainer* m_clicontainerp;

};

} // namespace ns3

#endif /* UDP_ECHO_SERVER_NEW_H */
