/*
 * 文件: udp-echo-server-new.cc
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的UDP服务器应用程序。源文件
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

#include "ns3/log.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/simulator.h"
#include "src/applications/model/udp-echo-server-new.h"
#include "src/core/model/externally-driven-sim.h"
#include "ns3/send-time-tag.h"
#include "src/network/model/packet-num-tag.h"
#include "src/network/model/packet-port-tag.h"
#include "src/network/model/statenum-tag.h"
#include "ns3/ipv4-l3-protocol.h"
#include "math.h"

namespace ns3
{
// 定义日志组件UdpEchoServerNewApplication并使用TypeId系统注册一个Object子类UdpEchoServerNew
NS_LOG_COMPONENT_DEFINE ("UdpEchoServerNewApplication");
NS_OBJECT_ENSURE_REGISTERED ( UdpEchoServerNew);
ApplicationContainer* UdpEchoServerNew::m_clicontainerp;

// 定义一个GetTypeId函数，来返回该类型TypeId类型，其中记录着该类的的一些基本信息
TypeId UdpEchoServerNew::GetTypeId(void)
{
static TypeId tid = TypeId("ns3::UdpEchoServerNew").SetParent<UdpEchoServer> ().
AddConstructor<UdpEchoServerNew> ();
return tid;
}

// 定义处理数据包的接收并读取数据包内容功能的函数 并在该函数中计划一个异步事件
void UdpEchoServerNew::HandleRead(Ptr<Socket> socket)
{
NS_LOG_INFO ("carry UdpEchoServerNew::HandleRead");
NS_LOG_FUNCTION (this << socket);
Ptr<Packet> packet;// 定义packet指针
Address from;// 定义from地址，作为RecvFrom的引用传递参数
Address localAddress;
for(int k = 0;k<100;k++)
{
des[k] = 0;// 每次接收到数据包后要把des数组置0
}
/*
 * 从套接字读取单个数据包并检索发件人地址。调用RecvFrom（maxSize，flags，fromAddress）
 * 隐式设置maxSize为最大接收数据包的大小，并将标志设置为零。其中fromAddress为输出参数，
 * 它将返回接收数据包的发送方地址（如果有）。 如果没有收到数据包，则保持不变。
 */
while ((packet = socket->RecvFrom(from)))
{
socket->GetSockName (localAddress);
m_rxTrace (packet);
m_rxTraceWithAddresses (packet, from, localAddress);
// 判断发送节点Address是否拥有匹配的internet套接字地址
if (InetSocketAddress::IsMatchingType(from))
{
Ptr<Application> Applic=(Application*) this;// 当前节点的指针
Ptr<Node> node=Applic->GetNode();// 得到该应用的节点指针
ser_id = node->GetId();// 得到安装服务器的节点的ID
// 返回对应于输入地址from的InetSocketAddress(internet套接字地址)
InetSocketAddress address=InetSocketAddress::ConvertFrom(from);
// 从数据包的PacketNumTag中获得该数据包的编号
PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
RCPacketNum = PacketNum.GetPacketNum();
// 终端输出通知：在仿真时间XXX 节点XXX 获得编号XXX的数据包 从XXX地址
NS_LOG_UNCOND("At time (ns3 simulation time) " << Simulator::Now ().GetSeconds ()
<< " node # " << ser_id <<" IPaddress: "<<node->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ()
<<" Port: "<<InetSocketAddress::ConvertFrom (localAddress).GetPort ()<<" Packet # "<<RCPacketNum
<<": Received "<< packet->GetSize()<< " bytes from IPaddress: "<< address.GetIpv4()
<<" Port: "<<address.GetPort ());
// 定义一个buffer数组大小为数据包大小的对象，通过new在堆上分配内存供buffer对象使用，
// 用来缓存数据包的内容
uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());// 将数据包的内容拷贝到缓存对象
RCdata = std::string((char*)buffer);// 从缓存对象中提取数据包内容
NS_LOG_UNCOND ("Receive packet data is "<<RCdata);
// 从数据包的SendTimeTag中获得该数据包的发送时间
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time RCSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("Receive packet SendTime is "<<RCSendTime.GetSeconds ());
PacketPortTag PacketPort;
packet->FindFirstMatchingByteTag (PacketPort);
int RCPacketPort = PacketPort.GetPacketPort();
if(RCPacketPort != -1)
{
NS_LOG_UNCOND ("Receive muti packet port is "<<RCPacketPort);
StateNumTag StateNum;
packet->FindFirstMatchingByteTag (StateNum);
int RCStateNum = StateNum.GetStateNum();
std::string temp = "";
for(int i = 0;i<RCStateNum;i++)
{
	if(i == RCPacketPort)
	{
		temp += RCdata;
	}
	else
	{
		temp += "   999";
	}
}
RCdata = temp;
NS_LOG_UNCOND ("mutipac simulink model all port input "<<RCdata);
}
// 接下来根据仿真的驱动模式，获得需要发送给matlab的数据包延迟和数据包的接收时间
if(ExternallyDrivenSim::g_qudong_mode == 1)
{
// 当仿真驱动模式为时间驱动时，我们获得需要发送给matlab的数据包延迟为当前仿真时间也就是数据包的接收时间
// 减去从SendTimeTag获得的数据包的发送时间
packetdelay = Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ();
NS_LOG_UNCOND ("packetdelay is "<<packetdelay);
send_time = RCSendTime.GetSeconds ();
if(ser_id != 0)
RCdata =std::to_string(RCSendTime.GetSeconds ())+" "+ RCdata + "   " + std::to_string(packetdelay);// wangluoTmpc.mdl wangluoTbuchang.mdl使用
else
RCdata = RCdata + "   " + std::to_string(packetdelay) + "   " + std::to_string(RCSendTime.GetSeconds ());
sim_time = Simulator::Now ().GetSeconds ();// 数据包接收时间为现在的仿真时间

}
if(ExternallyDrivenSim::g_qudong_mode == 2)
{
// 当仿真驱动模式为事件驱动模式时，情况会有一些不同，这里我们使用一种“名义上的”数据包延迟，我们把这种
// 数据包延迟定义为数据包的接收时间减去该数据包发送事件的前一个同步事件时间点作为该数据包的延迟，这是为
// 了在matlab中能够更简单的推算出该数据包的接收时间，进而推进matlab的仿真。计算出这种“名义上的”数据包
// 延迟的方式为数据包的发送事件的时间点减去该发送事件的前一个同步事件时间点（也有可能为0，即在该同步事件
// 时间点后立即发送数据包），在加上数据包真正的延迟。所以我们先把该数据包的发送时间对同步周期步长取余数，
// 来获得第一段的时间，然后在加上真正的延迟，这样我们计算出了这个“名义上的”数据包延迟。

// 这里由于ns3仿真默认的时间分辨率为纳秒，为了避免取余数时出现精度上的问题，所以我们使用GetNanoSeconds()
// 而不使用GetSeconds ()来计算，因为当使用GetSeconds ()时，对于计算同步事件时间点的发送事件，
// 很有可能出现余数十分接近0，或者十分接近同步周期步长的情况，而不是我们认为的0，这会导致两种软件
// 仿真时间无法同步的问题。
double Fmod = fmod(RCSendTime.GetNanoSeconds (),ExternallyDrivenSim::g_SimulationStep
.GetNanoSeconds())/1000000000;
//double Fmod = fmod(RCSendTime.GetSeconds (),ExternallyDrivenSim::g_SimulationStep
//.GetSeconds());
//if(fabs(Fmod-ExternallyDrivenSim::g_SimulationStep.GetSeconds())<0.000001)
//{
//Fmod = 0;
//}
// “名义上的”延迟为余数（第一段的时间），加上真正的数据包延迟
packetdelay = Fmod + (Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ());
NS_LOG_UNCOND ("packetdelay is "<<packetdelay);
// 赋值属性packetdelayReal，计算并输出真正的数据包延迟
packetdelayReal = Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ();
NS_LOG_UNCOND ("packetdelayReal is "<<packetdelayReal);
sim_time = Simulator::Now ().GetSeconds ();// 数据包接收时间为现在的仿真时间
send_time = RCSendTime.GetSeconds ();
}
packet->RemoveAllPacketTags();// 删除所有数据包标签。
packet->RemoveAllByteTags();// 删除存储在这个数据包中的所有字节中的标签。
ExternallyDrivenSim::GetNotices(this);// 该节点用该函数形成了外部应用程序的通知。
// 对于仿真驱动模式事件驱动，我们需要根据通过服务器接收到数据包的节点是否为控制系统的目标节点来计划
// 仿真的异步事件来完成一次与matlab的交互
if(ExternallyDrivenSim::g_qudong_mode == 2)
{
// UdpEchoServerNew应用获得外部应用程序发送过来的所有要发送的控制系统目标节点ID以及控制系统目标节点的数量
ExternallyDrivenSim::ser_get_des(this);
bool det_ser_id = false;// 定义变量det_ser_id用于判断该接收节点是否为控制系统目标节点，并置false
// 通过循环判断当前节点是否为控制系统目标节点，如果是那么置det_ser_id为true，并跳出循环
for(int i = 0;i<sys_ser_des;i++)
{
if(ser_id == des[i])
{
det_ser_id = true;
break;
}
}
// 如果该节点是控制系统目标节点，则利用Schedule_Asyn_event()函数计划一个异步事件
if(det_ser_id)
{
Schedule_Asyn_event();
}
}
// NS_LOG_LOGIC ("Echoing packet");
// socket->SendTo (packet, 0, from);// 服务器不在通过套接字发送回包
delete [] buffer;// 回收对象指针buffer指向的对象在堆上分配的内存，并释放该对象
}
}
}

// 定义处理数据包的接收并读取数据包内容功能的函数 并在该函数中计划一个异步事件
void UdpEchoServerNew::SetDelayrec(Ptr<Packet> packet,InetSocketAddress Address_cli,InetSocketAddress Address_ser)
{
NS_LOG_INFO ("carry UdpEchoServerNew::SetDelayrec");
NS_LOG_FUNCTION (this << packet<<Address_cli<<Address_ser);
/*
 * 从套接字读取单个数据包并检索发件人地址。调用RecvFrom（maxSize，flags，fromAddress）
 * 隐式设置maxSize为最大接收数据包的大小，并将标志设置为零。其中fromAddress为输出参数，
 * 它将返回接收数据包的发送方地址（如果有）。 如果没有收到数据包，则保持不变。
 */
m_rxTrace (packet);
m_rxTraceWithAddresses (packet, Address_cli, Address_ser);
// 判断发送节点Address是否拥有匹配的internet套接字地址
Ptr<Application> Applic=(Application*) this;// 当前节点的指针
Ptr<Node> node=Applic->GetNode();// 得到该应用的节点指针
ser_id = node->GetId();// 得到安装服务器的节点的ID
// 从数据包的PacketNumTag中获得该数据包的编号
PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
RCPacketNum = PacketNum.GetPacketNum();
// 终端输出通知：在仿真时间XXX 节点XXX 获得编号XXX的数据包 从XXX地址
NS_LOG_UNCOND("At time (ns3 simulation time) " << Simulator::Now ().GetSeconds ()
<< " node # " << ser_id <<" IPaddress: "<<Address_ser.GetIpv4()
<<" Port: "<<Address_ser.GetPort ()<<" Packet # "<<RCPacketNum
<<": Received "<< packet->GetSize()<< " bytes from IPaddress: "<< Address_cli.GetIpv4()
<<" Port: "<<Address_cli.GetPort ());
// 定义一个buffer数组大小为数据包大小的对象，通过new在堆上分配内存供buffer对象使用，
// 用来缓存数据包的内容
uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());// 将数据包的内容拷贝到缓存对象
RCdata = std::string((char*)buffer);// 从缓存对象中提取数据包内容
NS_LOG_UNCOND ("Receive packet data is "<<RCdata);
// 从数据包的SendTimeTag中获得该数据包的发送时间
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time RCSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("Receive packet SendTime is "<<RCSendTime.GetSeconds ());
PacketPortTag PacketPort;
packet->FindFirstMatchingByteTag (PacketPort);
int RCPacketPort = PacketPort.GetPacketPort();
if(RCPacketPort != -1)
{
NS_LOG_UNCOND ("Receive muti packet port is "<<RCPacketPort);
StateNumTag StateNum;
packet->FindFirstMatchingByteTag (StateNum);
int RCStateNum = StateNum.GetStateNum();
std::string temp = "";
for(int i = 0;i<RCStateNum;i++)
{
	if(i == RCPacketPort)
	{
		temp += RCdata;
	}
	else
	{
		temp += "   999";
	}
}
RCdata = temp;
NS_LOG_UNCOND ("mutipac simulink model all port input "<<RCdata);
}
// 接下来根据仿真的驱动模式，获得需要发送给matlab的数据包延迟和数据包的接收时间
if(ExternallyDrivenSim::g_qudong_mode == 1)
{
// 当仿真驱动模式为时间驱动时，我们获得需要发送给matlab的数据包延迟为当前仿真时间也就是数据包的接收时间
// 减去从SendTimeTag获得的数据包的发送时间
packetdelay = Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ();
NS_LOG_UNCOND ("packetdelay is "<<packetdelay);
sim_time = Simulator::Now ().GetSeconds ();// 数据包接收时间为现在的仿真时间
send_time = RCSendTime.GetSeconds ();
}
if(ExternallyDrivenSim::g_qudong_mode == 2)
{
// 当仿真驱动模式为事件驱动模式时，情况会有一些不同，这里我们使用一种“名义上的”数据包延迟，我们把这种
// 数据包延迟定义为数据包的接收时间减去该数据包发送事件的前一个同步事件时间点作为该数据包的延迟，这是为
// 了在matlab中能够更简单的推算出该数据包的接收时间，进而推进matlab的仿真。计算出这种“名义上的”数据包
// 延迟的方式为数据包的发送事件的时间点减去该发送事件的前一个同步事件时间点（也有可能为0，即在该同步事件
// 时间点后立即发送数据包），在加上数据包真正的延迟。所以我们先把该数据包的发送时间对同步周期步长取余数，
// 来获得第一段的时间，然后在加上真正的延迟，这样我们计算出了这个“名义上的”数据包延迟。

// 这里由于ns3仿真默认的时间分辨率为纳秒，为了避免取余数时出现精度上的问题，所以我们使用GetNanoSeconds()
// 而不使用GetSeconds ()来计算，因为当使用GetSeconds ()时，对于计算同步事件时间点的发送事件，
// 很有可能出现余数十分接近0，或者十分接近同步周期步长的情况，而不是我们认为的0，这会导致两种软件
// 仿真时间无法同步的问题。
double Fmod = fmod(RCSendTime.GetNanoSeconds (),ExternallyDrivenSim::g_SimulationStep
.GetNanoSeconds())/1000000000;
//double Fmod = fmod(RCSendTime.GetSeconds (),ExternallyDrivenSim::g_SimulationStep
//.GetSeconds());
//if(fabs(Fmod-ExternallyDrivenSim::g_SimulationStep.GetSeconds())<0.000001)
//{
//Fmod = 0;
//}
// “名义上的”延迟为余数（第一段的时间），加上真正的数据包延迟
packetdelay = Fmod + (Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ());
NS_LOG_UNCOND ("packetdelay is "<<packetdelay);
// 赋值属性packetdelayReal，计算并输出真正的数据包延迟
packetdelayReal = Simulator::Now ().GetSeconds () - RCSendTime.GetSeconds ();
NS_LOG_UNCOND ("packetdelayReal is "<<packetdelayReal);
sim_time = Simulator::Now ().GetSeconds ();// 数据包接收时间为现在的仿真时间
send_time = RCSendTime.GetSeconds ();
}
packet->RemoveAllPacketTags();// 删除所有数据包标签。
packet->RemoveAllByteTags();// 删除存储在这个数据包中的所有字节中的标签。
ExternallyDrivenSim::GetNotices(this);// 该节点用该函数形成了外部应用程序的通知。
// 对于仿真驱动模式事件驱动，我们需要根据通过服务器接收到数据包的节点是否为控制系统的目标节点来计划
// 仿真的异步事件来完成一次与matlab的交互

ApplicationContainer* p = m_clicontainerp;
uint32_t number = p->GetN();
for(uint32_t i = 0;i<number;i++)
{
Ptr<Application> itapp = p->Get(i);
Ipv4Address ipcli = itapp->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
if(ipcli == Address_cli.GetIpv4())
{
Ptr<UdpEchoClientNew> cliapp = (UdpEchoClientNew*)(PeekPointer(itapp));
cliapp->set_pacqueue.pop();
break;
}
}

if(ExternallyDrivenSim::g_qudong_mode == 2)
{
// UdpEchoServerNew应用获得外部应用程序发送过来的所有要发送的控制系统目标节点ID以及控制系统目标节点的数量
ExternallyDrivenSim::ser_get_des(this);
bool det_ser_id = false;// 定义变量det_ser_id用于判断该接收节点是否为控制系统目标节点，并置false
// 通过循环判断当前节点是否为控制系统目标节点，如果是那么置det_ser_id为true，并跳出循环
for(int i = 0;i<sys_ser_des;i++)
{
if(ser_id == des[i])
{
det_ser_id = true;
break;
}
}
// 如果该节点是控制系统目标节点，则利用Schedule_Asyn_event()函数计划一个异步事件
if(det_ser_id)
{
Schedule_Asyn_event();
}
}
// NS_LOG_LOGIC ("Echoing packet");
// socket->SendTo (packet, 0, from);// 服务器不在通过套接字发送回包
delete [] buffer;// 回收对象指针buffer指向的对象在堆上分配的内存，并释放该对象
}

// 定义启动UdpEchoServerNew应用的函数 该函数将会在执行服务器启动事件时运行
void UdpEchoServerNew::StartApplication(void)
{
Ptr<Application> Applic=(Application*) this;// 当前节点的指针
Ptr<Node> node=Applic->GetNode();// 得到该应用的节点指针
ser_id = node->GetId();// 得到安装服务器的节点的ID
NS_LOG_UNCOND ("node # "<<ser_id<<" carry UdpEchoServerNew::StartApplication");
NS_LOG_FUNCTION_NOARGS();// 终端通知
if (m_socket == 0)// 套接字等于0，即没有套接字对象
{
/*成员函数：LookupByName
 * 参数
	  命名所请求的TypeId的名称

返回
	与请求名称关联的唯一ID。

	此方法不会失败：如果输入名称不是有效的TypeId名称，它将会崩溃。
 */
TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
m_socket = Socket::CreateSocket(GetNode(), tid);// 根据tid中保存UdpSocket的信息的创建Udp套接字
// 定义一个internet套接字地址local，该类与BSD套接字API中的inet_sockaddr类似。即该类保存
// Ipv4Address和端口号以形成ipv4传输端点。表示local端点可以使用端口号m_port监听任何Ipv4Address目标端口为m_port
// 发送过来的数据包
InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),m_port);
if (m_socket->Bind (local) == -1)// 将该端点分配到该套接字
{
NS_FATAL_ERROR ("Failed to bind socket");
}
// 接下来为开启该套接字的多播数据包接收功能
// 多播地址被定义为224.0.0.0 - 239.255.255.255，只有在此范围内时才为真
if (addressUtils::IsMulticast(m_local))
{
// 将该Socket类指针转变成UdpSocket类指针
Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
if (udpSocket)
{
// 相当于setsockopt（MCAST_JOIN_GROUP）对应于套接字选项MCAST_JOIN_GROUP。
/*
 * 在指定的接口号上启用接收此套接字的多播数据报。 如果零指定为接口，则单个本地接口由系统选择。
 * 将来，这个函数在实现IGMP时会根据需要生成触发器IGMP连接，但是现在，尚未为此接口/groupAddress
 * 组合启用，则此函数仅仅实现在系统中启用多播数据报接收功能。
 */
udpSocket->MulticastJoinGroup(0, m_local);
}
else
{
NS_FATAL_ERROR(
"Error: joining multicast on a non-UDP socket");// 终端显示致命错误
}
}
}
// 设置该套接字接收到数据包的回调函数，即套接字一旦接收到数据包则启用UdpEchoServerNew::HandleRead
// 函数实现对数据包内容的处理和读取
m_socket->SetRecvCallback(MakeCallback(&UdpEchoServerNew::HandleRead,this));
}

// 函数定义 通过该函数计划一个异步事件来实现与matlab的交互
void UdpEchoServerNew::Schedule_Asyn_event(void)
{
NS_LOG_UNCOND ("carry UdpEchoServerNew::Schedule_Asyn_event");
// 当判断为控制系统目标节点接收到数据包，立即在该时刻计划一个异步事件，通过运行该事件调用
// ExternallyDrivenSim::trans_noti_insam将ExternallyDrivenSim::in_sam_trans属性置true
// 来实现在该异步事件处与matlab的一次交互
typedef void (*p)(void);
p p_insam = NULL;
p_insam = ExternallyDrivenSim::trans_noti_insam;
m_Asyn_event = Simulator::Schedule(Seconds(0), p_insam);
}

void UdpEchoServerNew::GetCliContainerp(ApplicationContainer* CliContainer_p)
{
NS_LOG_INFO("Get the ExternallyTest client application container.");
m_clicontainerp = CliContainer_p;
}

// UdpEchoServerNew类的构造函数
UdpEchoServerNew::UdpEchoServerNew()
{
}

// UdpEchoServerNew类的析构函数
UdpEchoServerNew::~UdpEchoServerNew()
{
}
}
