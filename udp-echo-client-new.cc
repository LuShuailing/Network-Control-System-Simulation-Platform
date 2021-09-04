/*
 * 文件: udp-echo-client-new.cc
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的UDP客户端应用程序。源文件
 *
 * 作者: 卢帅领
 *
 * E-mail: lushuailingfight@163.com
 *
 * Date:2020.11.1
 * 该软件被开发用于NCS仿真
 * NCS: network control system（网络控制系统）
 *
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-echo-client.h"
#include "src/core/model/externally-driven-sim.h"
#include "src/applications/model/udp-echo-client-new.h"
#include "ns3/send-time-tag.h"
#include "src/network/model/packet-num-tag.h"
#include "src/network/model/packet-port-tag.h"
#include "src/network/model/statenum-tag.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/double.h"

namespace ns3
{
// 定义日志组件UdpEchoClientNewApplication并使用TypeId系统注册一个Object子类UdpEchoClientNew
NS_LOG_COMPONENT_DEFINE ("UdpEchoClientNewApplication");
NS_OBJECT_ENSURE_REGISTERED ( UdpEchoClientNew);
// 在UdpEchoClientNew类外定义点静态ipidcontent结构体数组，用来当做节点发送事件ip、id和
// packetcontent的缓存，数组长度为50表示最多可以同时容纳50个待发送事件的ip、id和
// packetcontent信息对
ApplicationContainer* UdpEchoClientNew::m_sercontainerp;
uint16_t UdpEchoClientNew::m_delayswitch;
double UdpEchoClientNew::m_delay;
double UdpEchoClientNew::m_lossrate;
static struct IpIdcontent ipidcontent[50];
// 类外定义静态bool类型test[50]，用来表示该缓存的状态，如果该缓存中的某个分量已经被某待发送事件
// 的ip、id和packetcontent信息对占据，那么该test标志就为false，反之如果没有被待发送事件
// ip、id和packetcontent信息对占据，或者该发送事件已经发送，那么该test标志就为true，
// 表示可以保存下一个待发送事件的ip、id和packetcontent信息对
static bool test[50];

// 定义一个GetTypeId函数，来返回该类型TypeId类型，其中记录着该类的的一些基本信息
TypeId UdpEchoClientNew::GetTypeId(void)
{
static TypeId tid = TypeId("ns3::UdpEchoClientNew") .SetParent<UdpEchoClient> ()
.AddConstructor<UdpEchoClientNew> ();
return tid;
}

// 定义启动UdpEchoClientNew应用的函数 该函数将会在该应用程序启动事件处开始运行
// 开始该应用时，通过创建节点的套接字并对某远程节点建立连接
void UdpEchoClientNew::StartApplication(void)
{
Ptr<Application> Applic = (Application*) this;// 获得当前应用的指针
Ptr<Node> node = Applic->GetNode();// 获得当前应用的节点指针
cli_id = node->GetId();// 得到安装客户端的节点的ID
NS_LOG_UNCOND("node # "<<cli_id<<" carry UdpEchoClientNew::StartApplication");
NS_LOG_FUNCTION_NOARGS();// 该宏表示调用该函数产生通知：即输出函数的名称。
// 如果没有创建套接字，那么通过获取:UdpSocketFactory类的TypeId，调用Socket::CreateSocket
// 来创建UdpSocket
if (m_socket == 0)
{
TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
m_socket = Socket::CreateSocket (GetNode (), tid);
// 这段程序适用于使用ScheduleTransmit函数时，如果在外部脚本中安装客户端时
// 使用UdpEchoClientNewHelper类设定了目标节点的IP地址以及目标节点的端口号，
// 且在ExternallyDrivenSim::Listen()内调用的运行安排发送函数为RunScheduleTransmit，
// 而不是通过从matlab中获得数据包发送目标节点，并调用RunScheduleTransmitTo进行安排发送，
// 那么请取消注释以下代码
//NS_LOG_INFO ("m_peerAddress is"<<m_peerAddress);
//NS_LOG_INFO ("m_peerPort is"<<m_peerPort);
//    if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
//      {
//        if (m_socket->Bind () == -1)
//          {
//            NS_FATAL_ERROR ("Failed to bind socket");
//          }
//        m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
//      }
//    else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
//      {
//        if (m_socket->Bind6 () == -1)
//          {
//            NS_FATAL_ERROR ("Failed to bind socket");
//          }
//        m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
//      }
//    else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
//      {
//        if (m_socket->Bind () == -1)
//          {
//            NS_FATAL_ERROR ("Failed to bind socket");
//          }
//        m_socket->Connect (m_peerAddress);
//      }
//    else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
//      {
//        if (m_socket->Bind6 () == -1)
//          {
//            NS_FATAL_ERROR ("Failed to bind socket");
//          }
//        m_socket->Connect (m_peerAddress);
//      }
//    else
//      {
//        NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
//      }
}
// 启动该m_socket的广播功能
m_socket->SetAllowBroadcast(true);
// 由于客户端节点只是发送数据包而不需要再接收节点服务器因接收到数据包而发送的回复包，该函数调用被注释
// m_socket->SetRecvCallback(MakeCallback
// (&UdpEchoClientNew::HandleRead, this));
}

// 定义计划发送事件的函数（没有设定发送目标节点的IP地址），该函数在ExternallyDrivenSim类中调用
void UdpEchoClientNew::ScheduleTransmit(Time dt)// 节点的客户端应用计划发送事件
{
NS_LOG_UNCOND("node # "<<cli_id<<" carry UdpEchoClientNew::ScheduleTransmit");
NS_LOG_FUNCTION_NOARGS();
// 当前客户端节点计划一个相对时间dt后的发送事件，并从UdpEchoClientNew::Send得到该事件的eventid并返回
// 给m_sendEvent,之后send函数会使用SetEventId()函数重新赋值m_sendEvent变量为最新的事件，
// 判断当前的发送事件已经运行（运行之前会调用函数m_events->RemoveNext()将该事件从事件列表中移除）
m_sendEvent = Simulator::Schedule(dt, &UdpEchoClientNew::Send, this);
}

// 定义计划发送事件的函数（指定发送目标节点，并设定发送目标节点的IP地址以及此次发送数据包的内容），
// 该函数在ExternallyDrivenSim类中调用 这是UdpEchoClientNew::ScheduleTransmit(Time dt)
// 的重载函数
void UdpEchoClientNew::ScheduleTransmit(Time dt, Ipv4Address IpTo,std::string m_PayLoad,int mutipac,int statenum)
{
NS_LOG_UNCOND("node # "<<cli_id<<
" carry UdpEchoClientNew::ScheduleTransmit with designated node、IP and PayLoad");
NS_LOG_FUNCTION_NOARGS();
// 当前客户端节点计划一个相对时间dt后的发送事件，并从UdpEchoClientNew::SendTo得到该事件的eventid并返回
// 给m_sendEvent,之后sendTo函数会使用SetEventId()函数重新赋值m_sendEvent变量为最新的事件，
// 该m_sendEvent变量不仅仅用来判断该发送事件已经运行，同时也通过将m_sendEvent的eventId与Ip和
// PayLoad绑定，用来获得该次发送事件的指定目标节点的IP和packetcontent
m_sendEvent = Simulator::Schedule(dt, &UdpEchoClientNew::SendTo, this);
// 定义变量n，是某个待发送事件的ip、id和packetcontent信息对在ipidcontent中的索引
uint32_t n = 0;
// 通过索引循环判断每个test数组的分量，判断其是否保存有待发送事件的ip、id和packetcontent信息对，
// 如果发现某个test分量为true，表示对应的ipidcontent数组分量可以保存该次发送事件的ip、id和
// packetcontent信息对，那么就获得该索引，并跳出循环
for (int i = 0; i < 50; i++)
{
if (test[i])
{
n = i;
break;
}
}
// 在该索引对应的ipidcontent分量中保存此次发送事件的ip、id和packetcontent信息对
ipidcontent[n].m_ipTo = IpTo;// 把参数IpTo赋值给结构体ipid的变量m_ipTo
ipidcontent[n].m_packetcontent = m_PayLoad;// 把参数m_PayLoad赋值给结构体ipid的变量m_packetcontent
ipidcontent[n].m_eid = m_sendEvent;// 参数m_sendEvent的id赋值给结构体ipid的变量m_eid
ipidcontent[n].m_mutipac = mutipac;
ipidcontent[n].m_statenum = statenum;
// 该ipid分量已经保存有待发送事件的ip、id和packetcontent信息对，如果该发送事件不运行，
// 则该缓存分量不可使用,该缓存的状态被置为false
test[n] = false;
}

// 函数定义 节点的客户端应用执行数据包发送功能
void UdpEchoClientNew::Send(void)
{
NS_LOG_FUNCTION_NOARGS();
// 从ExternallyDrivenSim类中获得该次发送事件的EventId，这里为什么要对m_sendEvent进行重新赋值，
// 因为我们在运行该发送事件前，该节点的客户端可能会被重新计划一个新的发送事件，那么此时m_sendEvent
// 的值就指向之新的发送事件（也就是m_sendEvent重新刷新了，之前m_sendEvent保存的EventId不复存在），
// 那么当要运行send函数时，我们会执行以下的语句NS_ASSERT(m_sendEvent.IsExpired());该语句用来
// 判断当前时间是否已经运行，因为重新计划发送事件，显然我们现在的m_sendEvent指向的是最新的一次计划
// 的事件，那么我们就无法判断当前的发送事件是否过期（运行），那么我们就需要从推进事件仿真的函数中或的
// 当前发送事件的EventId，所以我们把当前的m_sendEvent重新赋值回当前的发送事件。
m_sendEvent = ExternallyDrivenSim::SetEventId();
// Simulator::IsExpired:检查事件是否已经运行或已被取消。该方法具有O（1）复杂度。
// 请注意，无法测试为“销毁”时间安排的事件的到期时间。 这样做会导致程序错误（崩溃）。
// 一个事件在开始被调度时被称为“过期”，这意味着如果事件执行的代码调用此函数，它将变为真。
// 判断是否该事件为空（即该事件已经运行，在事件列表中不复存在）如果不为空那么进行断言
NS_ASSERT(m_sendEvent.IsExpired());
Ptr<Packet> p;// 定义一个数据包的智能指针p，指向即将要发送的数据包对象
/*
 * 如果调用某个填充函数,则会设置m_dataSize的状态。那么m_dataSize不为零，
 * 我们将会有一个与我们期望复制和发送的数据包的大小（m_dataSize）相同的数据缓冲区（大小m_size）。
 * 在这种情况下，必须将m_size设置为与m_dataSize一致
 */
// 判断m_dataSize的大小，如果不为0，那么意味着调用了某个填充函数，这些填充函数在父类UdpEchoClient
// 中定义，用户可以使用UdpEchoClientNewHelper的setfill函数来调用并填充内容
if (m_dataSize)
{
/*
 * 大小不一致时断言输出UdpEchoClientNew::Send(): m_size and m_dataSize inconsistent
 * m_dataSize数据包的有效载荷大小，即发送数据包本身的大小，m_size为数据缓冲区大小
 */
NS_ASSERT_MSG(m_dataSize == m_size,
"UdpEchoClientNew::Send(): m_size and m_dataSize inconsistent");
// m_data：包有效载荷的数据.如果包有效载荷数据为0，也就是空包（不是数据包有效载荷大小），
// 断言输出m_dataSize（有数据大小） but no m_data（没数据内容）
NS_ASSERT_MSG(m_data,"UdpEchoClientNew::Send(): m_dataSize but no m_data");
// 给数据包Packet智能指针p赋值，使其指向一个具体定义的数据包，用2个变量m_data, m_dataSize
// 来具体定义数据包
p = Create<Packet> (m_data, m_dataSize);
// 这里定义一个SendTimeTag类变量SendTime，用于将该数据包的发送时间当做Tag添加到该数据包
SendTimeTag SendTime;
SendTime.SetSendTime (Simulator::Now ());// 该数据包的发送时间当做Tag
// AddByteTag使用新的字节标记标记此数据包中包含的每个字节。
p->AddByteTag (SendTime);// 把这个Tag类变量SendTime添加给即将发送的数据包
// 同理定义一个PacketNumTag类的变量PacketNum，将给数据包的编号当做Tag添加到该数据包
// 如果节点服务器没有接收到带有某个编号Tag的数据包，那么可以认为该数据包发生了丢包
PacketNumTag PacketNum;
PacketNum.SetPacketNum(m_sent+1);
p->AddByteTag (PacketNum);
}
/*
 * 如果m_dataSize为零，表示客户端不关心数据本身内容。客户端通过设置相应的属性PacketSize
 * 而不是通过调用SetFill函数来指定数据大小，在这种情况下，我们也不需要担心。
 * 但是我们确实允许m_size具有与m_dataSize（被设置为零）不同的值。
 */
else
{
// 给数据包Packet智能指针p赋值，指向一个具体定义的数据包，用变量数据缓冲区大小m_size
// （通过设置相应的属性而不调用SetFill函数来指定数据大小）来具体定义数据包 其中数据包内容随机
p = Create<Packet> (m_size);
// 同上面一样给数据包打上2个Tag SendTime和PacketNum
SendTimeTag SendTime;
SendTime.SetSendTime (Simulator::Now ());
p->AddByteTag (SendTime);
PacketNumTag PacketNum;
PacketNum.SetPacketNum(m_sent+1);
p->AddByteTag (PacketNum);
}
Address localAddress;
m_socket->GetSockName (localAddress);
/*
 * 在数据包实际发送之前调用跟踪接收器，以便添加到数据包的标签也可以正常发送
 */
m_txTrace(p);// 设置有关跟踪该数据包Tx（发送）事件的回调函数。
m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
m_socket->Send(p);// 将数据（或虚拟数据）发送到远程主机。参数p：要发送的数据包对象的用指针
++m_sent;// m_sent发送的数据包计数器自加1，表示已经发送出去一个数据包
Ptr<Application> Applic = (Application*) this;// 获得当前应用的指针
Ptr<Node> node = Applic->GetNode();// 获得当前应用的节点指针
cli_id = node->GetId();// 得到安装客户端的节点的ID,这个唯一的ID恰好也是节点进入NodeList的索引。
// 日志通知：在XXX仿真时间 XXID索引节点发送编号XX数据包 该数据包XXX个字节
NS_LOG_UNCOND("At time (ns3 simulation time) " << Simulator::Now ().GetSeconds ()
<<" Node # "<< cli_id <<" IPaddress: "<<node->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ()
<<" Port: "<<InetSocketAddress::ConvertFrom (localAddress).GetPort ()<< " Packet # " << m_sent
<< ": sent " << m_size<< " bytes to IPaddress: "<<m_peerAddress<<" Port: "<<m_peerPort);
}

// 节点的客户端应用执行数据包发送（指定发送的目标节点及其IP，并给相应数据包添加内容）
void UdpEchoClientNew::SendTo(void)
{
NS_LOG_FUNCTION_NOARGS();// 该宏表示调用该函数产生通知：即输出函数的名称。
// 从ExternallyDrivenSim类中获得该次发送事件的EventId
m_sendEvent = ExternallyDrivenSim::SetEventId();
NS_ASSERT(m_sendEvent.IsExpired());// 该发送事件已经运行
uint32_t n = 0;
// 这里将当前发送事件的eventid与保存在ipidcontent结构体数组中的分量进行比较来获得当前发送事件目标节点IP
// 和packetcontent，通过对ipidcontent索引循环，如果在ipidcontent结构体数组某分量中寻找到有该发送
// 事件的eid，获得该分量在ipidcontent结构体数组中的索引,并通过索引找到目标节点IP和packetcontent
for (int i = 0; i < 50; i++)
{
if (ipidcontent[i].m_eid == m_sendEvent)
{
n = i;
break;
}
}
// 该发送事件的ip、id和packetcontent信息对已经找到，该发送事件已经运行，对应的ipidcontent
// 结构体数组分量可以在之后写入新的事件的ip、id和packetcontent信息对
test[n] = true;
// 通过信息对找到该次节点发送事件的目标节点的IP，并与该节点建立套接字连接，m_peerPort
// （目标节点的端口号）为UdpEchoClientNewHelper类通过输入参数m_servPort来设定的
// UdpEchoClient属性RemotePort
m_socket->Connect(InetSocketAddress(ipidcontent[n].m_ipTo, m_peerPort));
Ptr<Packet> p;
// 输出该节点发送数据包的内容
NS_LOG_UNCOND("UdpEchoClientNew send packetcontent is "<<ipidcontent[n].m_packetcontent);
// 通过调用父类UdpEchoClient的SetFill函数,将数据包的数据填充（作为数据发送到服务器的内容）
// 设置填充字符串的内容是以零终止的字符串
SetFill(ipidcontent[n].m_packetcontent);
if (m_dataSize)
{
NS_ASSERT_MSG(m_dataSize == m_size,
"UdpEchoClientNew::Send(): m_size and m_dataSize inconsistent");
NS_ASSERT_MSG(m_data,"UdpEchoClientNew::Send(): m_dataSize but no m_data");
p = Create<Packet> (m_data, m_dataSize);
SendTimeTag SendTime;
SendTime.SetSendTime (Simulator::Now ());
p->AddByteTag (SendTime);
PacketNumTag PacketNum;
PacketNum.SetPacketNum(m_sent+1);
p->AddByteTag (PacketNum);
PacketPortTag PacketPort;
PacketPort.SetPacketPort(ipidcontent[n].m_mutipac);
p->AddByteTag (PacketPort);
StateNumTag StateNum;
StateNum.SetStateNum(ipidcontent[n].m_statenum);
p->AddByteTag (StateNum);
}
else
{
p = Create<Packet> (m_size);
SendTimeTag SendTime;
SendTime.SetSendTime (Simulator::Now ());
p->AddByteTag (SendTime);
PacketNumTag PacketNum;
PacketNum.SetPacketNum(m_sent+1);
p->AddByteTag (PacketNum);
}
Address localAddress;
m_socket->GetSockName (localAddress);
m_txTrace(p);
m_txTraceWithAddresses (p, localAddress, InetSocketAddress(ipidcontent[n].m_ipTo, m_peerPort));
if(m_delayswitch == 1)
{
m_socket->Send(p);
}
else
{
double min = 0.0;
double max = 1000000.0;
Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
x->SetAttribute ("Min", DoubleValue (min));
x->SetAttribute ("Max", DoubleValue (max));
double value = x->GetValue ();
if(value>=max*m_lossrate)
SetDelaysend(n,p,(Application*)this,localAddress);
}
++m_sent;
Ptr<Application> Applic = (Application*) this;
Ptr<Node> node = Applic->GetNode();
cli_id = node->GetId();// 得到安装客户端的节点的ID
// 日志通知：在XXX仿真时间 XXID索引节点发送编号XX数据包 该数据包XXX个字节发送目标节点IP为XXX
NS_LOG_UNCOND("At time (ns3 simulation time) " << Simulator::Now ().GetSeconds ()
<<" Node # " << cli_id <<" IPaddress: "<<node->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ()
<<" Port: "<<InetSocketAddress::ConvertFrom (localAddress).GetPort ()<< " Packet # " << m_sent
<< ": sent " << m_size << " bytes to IPaddress: "<<ipidcontent[n].m_ipTo<<" Port: "<<m_peerPort);
}

// 定义私有函数StopApplication用于停止UDP客户端
void
UdpEchoClientNew::StopApplication ()
{
NS_LOG_FUNCTION (this);
// 关闭套接字及其一切功能
if (m_socket != 0)
{
m_socket->Close ();
m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
m_socket = 0;
}
// 通过循环判断所有的test[i]分量，取消ipidcontent缓存中保存的所有计划但仍未运行的发送事件
for (int i = 0; i < 50; i++)
{
if (test[i] == false)
{
Simulator::Cancel (ipidcontent[i].m_eid);
}
}
}

// 定义UdpEchoClientNew类的构造函数，bool类数组test初始化为全真值，所有缓存分量都可以使用
UdpEchoClientNew::UdpEchoClientNew()
{
cli_id = 0;
set_pacqueue = std::queue<Ptr<Packet>>();
for (int i = 0; i < 50; i++)
{
test[i] = true;
}
}

// 定义UdpEchoClientNew类的析构函数
UdpEchoClientNew::~UdpEchoClientNew()
{
}

// 函数定义 处理数据包的接收并读取数据包内容的函数
void UdpEchoClientNew::HandleRead(Ptr<Socket> socket)
{
NS_LOG_INFO("carry UdpEchoClientNew::HandleRead");
NS_LOG_FUNCTION(this << socket);// 输出this（当前对象的指针）和socket
Ptr<Packet> packet;// 定义接收数据包packet
Address from;// 定义from地址
while ((packet = socket->RecvFrom(from)))
/*
 * 从套接字读取单个数据包并检索发件人地址。使用maxSize调用RecvFrom（maxSize，flags，fromAddress）
 * 隐式设置为最大大小的整数，并将标志设置为零。
 */
{
// 判断InetSocketAddress与Address是否匹配
if (InetSocketAddress::IsMatchingType(from))
{
// 返回对应于输入地址的InetSocketAddress
InetSocketAddress address = InetSocketAddress::ConvertFrom(from);
NS_LOG_UNCOND("Received " << packet->GetSize() << " bytes from "
<< address.GetIpv4());// 通知回显
}
}
}

void UdpEchoClientNew::SetDelaysend(uint32_t n,Ptr<Packet> pac,Ptr<Application> appcli,Address Address_cli)
{
set_pacqueue.push(pac);
Ipv4Address ipcli = appcli->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
uint16_t sendport = InetSocketAddress::ConvertFrom (Address_cli).GetPort ();
ApplicationContainer* p = m_sercontainerp;
uint32_t number = p->GetN();
for(uint32_t i = 0;i<number;i++)
{
Ptr<Application> itapp = p->Get(i);
Ipv4Address ipser = itapp->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
if(ipser == ipidcontent[n].m_ipTo)
{
Ptr<UdpEchoServerNew> serapp = (UdpEchoServerNew*)(PeekPointer(itapp));
EventId SetDelay_event = Simulator::Schedule
(Seconds(m_delay),&UdpEchoServerNew::SetDelayrec,serapp,pac,InetSocketAddress(ipcli,sendport),InetSocketAddress(ipidcontent[n].m_ipTo, m_peerPort));
break;
}
}
}

void UdpEchoClientNew::GetSerContainerp(ApplicationContainer* SerContainer_p)
{
NS_LOG_INFO("Get the ExternallyTest server application container.");
m_sercontainerp = SerContainer_p;
}

void UdpEchoClientNew::Getdelayswitch(uint16_t delayswitch)
{
NS_LOG_INFO("Get the ExternallyTest set delay switch.");
m_delayswitch = delayswitch;
NS_LOG_INFO ("the delay switch is seted as :"<<m_delayswitch);
}

void UdpEchoClientNew::Getdelay(double delay)
{
NS_LOG_INFO("Get the ExternallyTest set delay.");
m_delay = delay;
NS_LOG_INFO ("the delay is seted as :"<<m_delay);
}

void UdpEchoClientNew::Getlossrate(double lossrate)
{
NS_LOG_INFO("Get the ExternallyTest set lossrate.");
m_lossrate = lossrate;
NS_LOG_INFO ("the lossrate is seted as :"<<m_lossrate);
}
}
