/*
 * 文件: udp-echo-client-new.h
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的UDP客户端应用程序。头文件
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

#ifndef UDP_ECHO_CLIENT_NEW_H
#define UDP_ECHO_CLIENT_NEW_H

#include "ns3/udp-echo-client.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/tag.h"
#include "ns3/tag-buffer.h"
#include "ns3/application-container.h"
#include <queue>

namespace ns3 {
// 在ns3的命名空间内定义结构体IpIdcontent，使得能够在udp-echo-client-new.cc和tcp-client.cc
// 中定义该结构体数组
struct IpIdcontent
{
Ipv4Address m_ipTo;// 这次发送事件将要发送的目标节点的IP
EventId m_eid;// 这次计划的发送事件的事件id
std::string m_packetcontent;// 这次发送事件的数据包内容
int m_mutipac;
int m_statenum;
};

// ns3::UdpEchoClientNew的定义 公有继承自UdpEchoClient类
class UdpEchoClientNew : public UdpEchoClient
{
// 声明和定义公有成员
public:
// 将ExternallyDrivenSim类设为UdpEchoClientNew类的友元类，这样就可以在
// ExternallyDrivenSim类中让UdpEchoClientNew类使用其私有函数ScheduleTransmit
friend class ExternallyDrivenSim;
friend class UdpEchoServerNew;
/**
 * \brief Get the type ID.
 * \return the object TypeId
 */
static TypeId GetTypeId (void);
// 声明UdpEchoClientNew类的构造函数
UdpEchoClientNew ();
// 声明UdpEchoClientNew类的析构函数
virtual ~UdpEchoClientNew ();
static void GetSerContainerp(ApplicationContainer* SerContainer_p);
static void Getdelayswitch(uint16_t delayswitch);
static void Getdelay(double delay);
static void Getlossrate(double lossrate);

private:
// 声明启动UdpEchoClientNew应用的函数 该函数将会在该应用程序启动事件处开始运行，应用程序启动时间
// 在仿真脚本中通过ApplicationContainer::Start (Time start)调用
virtual void StartApplication (void);
// 声明停止UdpEchoClientNew应用的函数 该函数将会在该应用程序停止事件处开始运行，应用程序停止时间
// 在仿真脚本中通过ApplicationContainer::Stop (Time stop)调用
virtual void StopApplication (void);
// 声明计划发送事件的函数（没有设定发送目标节点的IP地址），该函数在ExternallyDrivenSim类中调用
void ScheduleTransmit (Time dt);
// 声明计划发送事件的函数（指定发送目标节点，并设定发送目标节点的IP地址以及此次发送数据包的内容），
// 该函数在ExternallyDrivenSim类中调用
void ScheduleTransmit(Time dt, Ipv4Address IpTo,std::string m_PayLoad,int mutipac,int statenum);
// 声明节点发送事件函数（根据目标节点IP地址发送）
void SendTo(void);
// 声明节点发送事件函数
void Send (void);
/**
 * \ 声明处理数据包的接收并读取数据包内容功能的函数
 */
void HandleRead (Ptr<Socket> socket);
// 属性定义 为当前安装有客户端的节点的ID，即在节点容器中的索引
void SetDelaysend(uint32_t n,Ptr<Packet> pac,Ptr<Application> appcli,Address Address_cli);
int64_t cli_id;
std::queue<Ptr<Packet>> set_pacqueue;
static ApplicationContainer* m_sercontainerp;
static uint16_t m_delayswitch;
static double m_delay;
static double m_lossrate;

};

} // namespace ns3

#endif /* UDP_ECHO_CLIENT_NEW_H */
