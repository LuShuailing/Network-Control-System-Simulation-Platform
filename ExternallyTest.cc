/*
 * 文件: ExternallyTest.cc
 *
 * 版本: 3.0
 *
 * 描述：外部驱动的仿真器应用的ns3用户脚本。源文件
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

#include "src/core/model/externally-driven-sim.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "src/applications/helper/udp-echo-new-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/vector.h"
#include "ns3/dsr-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/lte-module.h"
#include <ns3/lr-wpan-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/propagation-module.h>
#include "ns3/sixlowpan-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

// 仿真脚本使用命名空间ns3与dsr
using namespace ns3;
using namespace dsr;

// 定义日志组件ExternallyTestScript
NS_LOG_COMPONENT_DEFINE ("ExternallyTestScript");

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p)
{
  NS_LOG_UNCOND ("Received packet of size " << p->GetSize ());
  uint8_t *buffer = new uint8_t[p->GetSize()];
  p->CopyData(buffer,p->GetSize());// 将数据包的内容拷贝到缓存对象
  std::string RCdata = std::string((char*)buffer);// 从缓存对象中提取数据包内容
  NS_LOG_UNCOND ("Receive packet data is "<<RCdata);
}
void
UePacketTrace (Ptr<OutputStreamWrapper> stream, const Ipv4Address &localAddrs, std::string contex, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << contex << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (srcAddrs))
    {
      oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ();
      if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
        {
          *stream->GetStream () << localAddrs << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << localAddrs << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}

// 定义WifiPhyStats类，用来获得wifi物理层状态
class WifiPhyStats : public Object
{
public:
/**
 * \brief Gets the class TypeId
 * \return the class TypeId
 */
static TypeId GetTypeId (void);

/**
 * \brief Constructor
 * \return none
 */
WifiPhyStats ();

/**
 * \brief Destructor
 * \return none
 */
virtual ~WifiPhyStats ();

/**
 * \brief Returns the number of bytes that have been transmitted
 * (this includes MAC/PHY overhead)
 * \return the number of bytes transmitted
 */
uint32_t GetTxBytes ();

/**
 * \brief Callback signiture for Phy/Tx trace
 * \param context this object
 * \param packet packet transmitted
 * \param mode wifi mode
 * \param preamble wifi preamble
 * \param txPower transmission power
 * \return none
 */
void PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);

/**
 * \brief Callback signiture for Phy/TxDrop
 * \param context this object
 * \param packet the tx packet being dropped
 * \return none
 */
static void PhyTxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet);

/**
 * \brief Callback signiture for Phy/RxDrop
 * \param context this object
 * \param packet the rx packet being dropped
 * \return none
 */
static void PhyRxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet);

private:
static uint32_t m_phyTxPkts; ///< phy transmit packets
static uint32_t m_phyTxBytes; ///< phy transmit bytes
};
uint32_t WifiPhyStats::m_phyTxPkts(0);
uint32_t WifiPhyStats::m_phyTxBytes(0);
NS_OBJECT_ENSURE_REGISTERED (WifiPhyStats);

TypeId
WifiPhyStats::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::WifiPhyStats")
.SetParent<Object> ()
.AddConstructor<WifiPhyStats> ();
return tid;
}

WifiPhyStats::WifiPhyStats ()
{
}

WifiPhyStats::~WifiPhyStats ()
{
}

void
WifiPhyStats::PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t power)
{
NS_LOG_FUNCTION (this << context << packet << "PHYTX mode=" << mode );
++m_phyTxPkts;
//NS_LOG_UNCOND ("m_phyTxPkts is "<<m_phyTxPkts);
uint32_t pktSize = packet->GetSize ();
m_phyTxBytes += pktSize;
//NS_LOG_UNCOND ("m_phyTxBytes is "<<m_phyTxBytes);

//NS_LOG_UNCOND ("Received PHY size=" << pktSize);
}

// 物理层发送丢包发生时，触发该回调函数，将丢包的相关信息添加到输出流
void
WifiPhyStats::PhyTxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet)
{
NS_LOG_UNCOND ("PHY Tx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ());
Ipv4Header h;
packet->PeekHeader(h);
Ipv4Address addr_Destination = h.GetDestination();
Ipv4Address addr_Source = h.GetSource();
NS_LOG_UNCOND ("PHY Tx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source);

PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
uint32_t TxDropPacketNum = PacketNum.GetPacketNum();
NS_LOG_UNCOND ("the Number of PHY Tx Drop Packet of Source node sending is "<<TxDropPacketNum);

uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());
std::string TxDropdata = std::string((char*)buffer);
NS_LOG_UNCOND ("PHY TxDrop data is "<<TxDropdata);
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time TxDropPacSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("PHY TxDrop Pac SendTime is (ns3 simulation time) "
<<TxDropPacSendTime.GetSeconds ());

double droptime_afterTX = Simulator::Now ().GetSeconds () - TxDropPacSendTime.GetSeconds ();
NS_LOG_UNCOND ("the PHY TxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX);
*os<<"PHY Tx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ()<<std::endl;
*os<<"PHY Tx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source<<std::endl;
*os<<"the Number of PHY Tx Drop Packet of Source node sending is "<<TxDropPacketNum<<std::endl;
*os<<"PHY TxDrop data is "<<TxDropdata<<std::endl;
*os<<"PHY TxDrop Pac SendTime is (ns3 simulation time) "<<TxDropPacSendTime.GetSeconds ()<<std::endl;
*os<<"the PHY TxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX<<std::endl;
delete [] buffer;
}

// 物理层接收丢包发生时，触发该回调函数，将丢包的相关信息添加到输出流
void
WifiPhyStats::PhyRxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet)
{
NS_LOG_UNCOND ("PHY Rx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ());
Ipv4Header h;
packet->PeekHeader(h);
Ipv4Address addr_Destination = h.GetDestination();
Ipv4Address addr_Source = h.GetSource();
NS_LOG_UNCOND ("PHY Rx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source);

PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
uint32_t RxDropPacketNum = PacketNum.GetPacketNum();
NS_LOG_UNCOND ("the Number of PHY Rx Drop Packet of Source node sending is "<<RxDropPacketNum);

uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());
std::string RxDropdata = std::string((char*)buffer);
NS_LOG_UNCOND ("PHY RxDrop data is "<<RxDropdata);
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time RxDropPacSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("PHY RxDrop Pac SendTime is (ns3 simulation time) "
<<RxDropPacSendTime.GetSeconds ());

double droptime_afterTX = Simulator::Now ().GetSeconds () - RxDropPacSendTime.GetSeconds ();
NS_LOG_UNCOND ("the PHY RXDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX);
*os<<"PHY Rx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ()<<std::endl;
*os<<"PHY Rx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source<<std::endl;
*os<<"the Number of PHY Rx Drop Packet of Source node sending is "<<RxDropPacketNum<<std::endl;
*os<<"PHY RxDrop data is "<<RxDropdata<<std::endl;
*os<<"PHY RxDrop Pac SendTime is (ns3 simulation time) "<<RxDropPacSendTime.GetSeconds ()<<std::endl;
*os<<"the PHY RxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX<<std::endl;
delete [] buffer;
}

uint32_t
WifiPhyStats::GetTxBytes ()
{
return m_phyTxBytes;
}

// 定义WifiMacStats类，用来获得wifi mac层状态
class WifiMacStats : public Object
{
public:
/**
 * \brief Gets the class TypeId
 * \return the class TypeId
 */
static TypeId GetTypeId (void);

/**
 * \brief Constructor
 * \return none
 */
WifiMacStats ();

/**
 * \brief Destructor
 * \return none
 */
virtual ~WifiMacStats ();

void MacTxTrace (std::string context,Ptr< const Packet > packet);

/**
 * \brief Callback signiture for Phy/TxDrop
 * \param context this object
 * \param packet the tx packet being dropped
 * \return none
 */
static void MacTxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet);

/**
 * \brief Callback signiture for Phy/RxDrop
 * \param context this object
 * \param packet the rx packet being dropped
 * \return none
 */
static void MacRxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet);

private:
static uint32_t m_macTxPkts; ///< mac transmit packets
static uint32_t m_macTxBytes; ///< mac transmit bytes
};
uint32_t WifiMacStats::m_macTxPkts(0);
uint32_t WifiMacStats::m_macTxBytes(0);
NS_OBJECT_ENSURE_REGISTERED (WifiMacStats);

TypeId
WifiMacStats::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::WifiMacStats")
.SetParent<Object> ()
.AddConstructor<WifiMacStats> ();
return tid;
}

WifiMacStats::WifiMacStats ()
{
}

WifiMacStats::~WifiMacStats ()
{
}

void
WifiMacStats::MacTxTrace (std::string context,Ptr< const Packet > packet)
{
NS_LOG_FUNCTION (this << context << packet);
++m_macTxPkts;
//NS_LOG_UNCOND ("m_macTxPkts is "<<m_macTxPkts);
uint32_t pktSize = packet->GetSize ();
m_macTxBytes += pktSize;
//NS_LOG_UNCOND ("m_macTxBytes is "<<m_macTxBytes);

//NS_LOG_UNCOND ("Received MAC size=" << pktSize);
}

// mac层发送丢包发生时，触发该回调函数，将丢包的相关信息添加到输出流
void
WifiMacStats::MacTxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet)
{
NS_LOG_UNCOND ("MAC Tx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ());
Ipv4Header h;
packet->PeekHeader(h);
Ipv4Address addr_Destination = h.GetDestination();
Ipv4Address addr_Source = h.GetSource();
NS_LOG_UNCOND ("MAC Tx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source);

PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
uint32_t TxDropPacketNum = PacketNum.GetPacketNum();
NS_LOG_UNCOND ("the Number of MAC Tx Drop Packet of Source node sending is "<<TxDropPacketNum);

uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());
std::string TxDropdata = std::string((char*)buffer);
NS_LOG_UNCOND ("MAC TxDrop data is "<<TxDropdata);
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time TxDropPacSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("MAC TxDrop Pac SendTime is (ns3 simulation time) "
<<TxDropPacSendTime.GetSeconds ());

double droptime_afterTX = Simulator::Now ().GetSeconds () - TxDropPacSendTime.GetSeconds ();
NS_LOG_UNCOND ("the MAC TxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX);
*os<<"MAC Tx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ()<<std::endl;
*os<<"MAC Tx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source<<std::endl;
*os<<"the Number of MAC Tx Drop Packet of Source node sending is "<<TxDropPacketNum<<std::endl;
*os<<"MAC TxDrop data is "<<TxDropdata<<std::endl;
*os<<"MAC TxDrop Pac SendTime is (ns3 simulation time) "<<TxDropPacSendTime.GetSeconds ()<<std::endl;
*os<<"the MAC TxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX<<std::endl;
delete [] buffer;
}

// mac层接收丢包发生时，触发该回调函数，将丢包的相关信息添加到输出流
void
WifiMacStats::MacRxDrop (std::ostream *os,std::string context, Ptr<const Packet> packet)
{
NS_LOG_UNCOND ("MAC Rx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ());
Ipv4Header h;
packet->PeekHeader(h);
Ipv4Address addr_Destination = h.GetDestination();
Ipv4Address addr_Source = h.GetSource();
NS_LOG_UNCOND ("MAC Rx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source);

PacketNumTag PacketNum;
packet->FindFirstMatchingByteTag (PacketNum);
uint32_t RxDropPacketNum = PacketNum.GetPacketNum();
NS_LOG_UNCOND ("the Number of MAC Rx Drop Packet of Source node sending is "<<RxDropPacketNum);

uint8_t *buffer = new uint8_t[packet->GetSize()];
packet->CopyData(buffer,packet->GetSize());
std::string RxDropdata = std::string((char*)buffer);
NS_LOG_UNCOND ("MAC RxDrop data is "<<RxDropdata);
SendTimeTag SendTime;
packet->FindFirstMatchingByteTag (SendTime);
Time RxDropPacSendTime = SendTime.GetSendTime ();
NS_LOG_UNCOND ("MAC RxDrop Pac SendTime is (ns3 simulation time) "
<<RxDropPacSendTime.GetSeconds ());

double droptime_afterTX = Simulator::Now ().GetSeconds () - RxDropPacSendTime.GetSeconds ();
NS_LOG_UNCOND ("the MAC RXDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX);
*os<<"MAC Rx Drop at time (ns3 simulation time) "<<Simulator::Now ().GetSeconds ()<<std::endl;
*os<<"MAC Rx Drop packet Destination "<<addr_Destination<<" and Source "<<addr_Source<<std::endl;
*os<<"the Number of MAC Rx Drop Packet of Source node sending is "<<RxDropPacketNum<<std::endl;
*os<<"MAC RxDrop data is "<<RxDropdata<<std::endl;
*os<<"MAC RxDrop Pac SendTime is (ns3 simulation time) "<<RxDropPacSendTime.GetSeconds ()<<std::endl;
*os<<"the MAC RxDrop time after the Packet has been sent is (ns3 simulation time) "
<<droptime_afterTX<<std::endl;
delete [] buffer;
}

// 定义外部驱动仿真测试脚本类
class ExternallyTest
{
public:
// 声明初始化ExternallyTest类的构造函数
ExternallyTest();
// 声明初始化ExternallyTest类的析构函数
virtual ~ExternallyTest();
// 声明利用命令行参数来配置ExternallyTest参数的函数
void Configure(int argc, char ** argv);
// 声明运行ExternallyTest函数
int Run();
private:
// 定义ExternallyTest类的节点个数属性
int m_nodes;
// 定义ExternallyTest类的运行总时间属性
double m_totalTime;
// 定义ExternallyTest类的是否产生pcap监视记录文件的属性
bool m_pcap;
// 定义ExternallyTest类的服务器应用个数属性
int m_server;
// 定义ExternallyTest类的服务器应用端口号属性
uint16_t m_servPort;
// 定义ExternallyTest类的网络节点列表属性（使用节点容器类定义）
NodeContainer nodes;//定义节点容器
// 定义ExternallyTest类的所有设备列表属性（使用设备容器类定义）
NetDeviceContainer devices;
// 定义ExternallyTest类的所有节点的接口地址列表属性（使用地址容器类定义）
Ipv4InterfaceContainer interfaces;
Ipv6InterfaceContainer interfaces6;
// 定义ExternallyTest类的wifi助手类属性
WifiHelper wifi;
// 定义ExternallyTest类的Wifi80211p助手类属性
Wifi80211pHelper wifi80211p;
// 定义ExternallyTest类的Wave助手类属性
WaveHelper waveHelper;
Ptr<LteHelper> lteHelper;
Ptr<PointToPointEpcHelper>  epcHelper;
Ptr<LteSidelinkHelper> proseHelper;
// 定义ExternallyTest类的ns3仿真每次同步事件间隔的仿真步长属性
uint16_t m_simulationStep;
Ptr<YansWifiChannel> channel;

private:
// 声明创建节点、物理层、mac层和节点移动性质函数
void CreateNodesphymacmobility();
// 声明安装internet协议栈函数
void InstallInternetStack();
// 声明安装应用层函数（包括节点的客户端和服务器应用）
void InstallApplication();
// 声明TakePositionFromFile函数 从文件中获得节点位置和移动模型
ListPositionAllocator* TakePositionFromFile(char file[]);
// 声明SetupRoutingProtocol函数 安装IP层路由协议
void SetupRoutingProtocol (NodeContainer & c);
// 声明AssignIpAddresses函数 给所有节点设备的接口分配IP地址
void AssignIpAddresses (NetDeviceContainer & d,
Ipv4InterfaceContainer & adhocTxInterfaces,Ipv6InterfaceContainer & adhocTxInterfaces6);
// 声明Configure_phyMode_Defaults函数 配置物理层传输模式和一些默认设置
void Configure_phyMode_Defaults ();
// 声明SeedManager函数 设置随机数种子
void SeedManager();
// 声明ConfigureNodes函数 根据节点个数属性创建相应个数的节点
void ConfigureNodes ();
// 声明Configurephymac函数 配置物理层和mac层并生成设备
void Configurephymac ();
// 声明Configurephymac函数 生成adhoc设备
void SetupAdhocDevices ();
// 声明ConfigureDevices函数 为设备们添加物理层和mac层的丢包跟踪
void ConfigureDevicestracing ();
// 声明ConfigureMobility函数 为节点们配置移动性质
void ConfigureMobility();
// 声明SetupAdhocMobilityNodes函数 为adhoc节点安装节点移动模型
void SetupAdhocMobilityNodes ();
// 声明CourseChange函数 向节点移动路径跟踪文件写入节点路径
static void CourseChange (std::ostream *os, std::string context, Ptr<const MobilityModel> mobility);
// 声明SetupLogFile函数 产生节点移动路径跟踪文件，物理层和mac层跟踪文件
void SetupLogFile ();
// 声明ConfigureLogTracing函数 一些跟踪文件是否产生的开关，以及终端日志是否产生的开关
void ConfigureLogTracing ();
// 声明Runsimulation函数 配置网络节点动画，流量文件生成，以及开始仿真运行
void Runsimulation ();
// 声明Set_ExternallyDrivenSim_config函数 一些ExternallyDrivenSim类运行需要的参数配置
void Set_ExternallyDrivenSim_config();
void SetupWaveMessages ();
void SetLtePacTracing( const ApplicationContainer &clientA ,const ApplicationContainer &serverA);
void Set_delayloss ();

// 定义ExternallyTest类的IP层路由选择属性
uint32_t m_protocol;
// 定义ExternallyTest类的IP层路由名称属性
std::string m_protocolName;
// 定义ExternallyTest类的m_routingTables属性 是否在特定时间生成路由表跟踪文件
int m_routingTables;
// 定义ExternallyTest类的m_80211mode属性 m_80211的模式
uint32_t m_80211_ltemode;
// 定义ExternallyTest类的ipv4RoutingHelper属性 使用ipv4静态路由助手类定义
Ipv4StaticRoutingHelper ipv4RoutingHelper;
// 定义ExternallyTest类的m_phyMode属性  802.11b协议物理层的传输模式的选择
uint32_t m_phyMode;
// 定义ExternallyTest类的m_phy_pMode属性 802.11p协议下物理层的传输模式的选择
uint32_t m_phy_pMode;
// 定义ExternallyTest类的m_phy_WAVEMode属性 WAVE协议下物理层的传输模式的选择
uint32_t m_phy_WAVEMode;
// 定义ExternallyTest类的m_phyModeName属性 802.11b协议下物理层的传输模式的名称
std::string m_phyModeName;
// 定义ExternallyTest类的m_phyMode_pName属性 802.11p协议下物理层的传输模式的名称
std::string m_phyMode_pName;
// 定义ExternallyTest类的m_phyMode_WAVEName属性 WAVE协议下物理层的传输模式的名称
std::string m_phyMode_WAVEName;
uint32_t m_lr_wpanMode;
// 定义ExternallyTest类的m_seed属性 传递仿真使用的随机数种子
uint32_t m_seed;
// 定义ExternallyTest类的m_run属性 传递仿真使用的随机数种子的片段序列号
uint32_t m_run;
// 定义ExternallyTest类的m_lossModel属性 物理层的传输损失模型的选择
static uint32_t m_lossModel;
// 定义ExternallyTest类的m_lossModelName属性 物理层的信道传输损失模型的名称
std::string m_lossModelName;
// 定义ExternallyTest类的m_fading属性 物理层的噪声模型（衰减损耗模型）的选择
uint32_t m_fading;
uint32_t m_ltefading;
// 定义ExternallyTest类的m_verbose属性 wifi与应用层的日志开关
int m_verbose;
// 定义ExternallyTest类的m_RemoteStationManager属性 RemoteStationManager模式的选择
int m_RemoteStationManager;
// 定义ExternallyTest类的m_EnergyDetectionThreshold属性 接收器信号功率探测阈值(单位dbm)
double m_EnergyDetectionThreshold;
// 定义ExternallyTest类的m_CcaMode1Threshold属性 CcaMode1信号强度探测阈值(单位dbm)
double m_CcaMode1Threshold;
// 定义ExternallyTest类的m_txp属性 发射器信号发送功率（单位db）
double m_txp;
// 定义ExternallyTest类的m_rxg属性 接收器信号放大增益（单位db）
double m_rxg;
// 定义ExternallyTest类的m_rxnf属性 接收器信噪比损耗（单位db）
double m_rxnf;
// 定义ExternallyTest类的m_Antenna属性 每个节点天线数
uint32_t m_Antenna;
// 定义ExternallyTest类的m_asciiTrace属性 是否产生网络ascii监视记录文件
int m_asciiTrace;
// 定义ExternallyTest类的m_trName属性 网络和节点移动ascii监视记录文件的前缀名
std::string m_trName;
// 定义ExternallyTest类的m_wifiPhyStats属性
Ptr<WifiPhyStats> m_wifiPhyStats;
Ptr<WifiMacStats> m_WifiMacStats;
// 定义ExternallyTest类的m_wifiPhyStats属性 RandomWaypointMobilityModel的节点移动速度参数
int m_nodeSpeed; ///< in m/s
// 定义ExternallyTest类的m_nodePause属性 RandomWaypointMobilityModel的节点暂停时间参数
int m_nodePause; ///< in s
// 定义ExternallyTest类的taPositionAlloc属性 指向节点位置列表的指针
Ptr<ListPositionAllocator> taPositionAlloc;
// 定义ExternallyTest类的m_streamIndex属性 安装节点移动模型使用的随机数流的数量
int64_t m_streamIndex;
// 定义ExternallyTest类的m_os属性 节点的CourseChange输出流
std::ofstream m_os;
// 定义ExternallyTest类的m_phy_drop_os属性 节点的物理层丢包输出流
std::ofstream m_phy_drop_os;
// 定义ExternallyTest类的m_mac_drop_os属性 节点的mac层丢包输出流
std::ofstream m_mac_drop_os;
// 定义ExternallyTest类的m_logFile属性 CourseChange的日志文件名
std::string m_logFile;
// 定义ExternallyTest类的m_logFile属性 节点移动模型的选择
uint16_t m_MobilityModel;
// 定义ExternallyTest类的m_rtt属性 生成路由表跟踪的时间
double m_rtt;
// 定义ExternallyTest类的m_qudong_mode属性 2中仿真驱动模式的选择
uint16_t m_qudong_mode;
// 定义ExternallyTest类的m_antennaHeightAboveZ属性 当选择TwoRayGroundPropagationLossModel是保存天线高度
static double m_antennaHeightAboveZ;
// 定义ExternallyTest类的m_trans_protocol_mode属性 2种传输层传输协议模式的选择
uint16_t m_trans_protocol_mode;
// 定义ExternallyTest类的m_otherlogTrace属性 其他一些日志跟踪文件产生的开关
int m_otherlogTrace;
WaveBsmHelper m_waveBsmHelper; /// waveBsmHelper
ApplicationContainer clientApps;
ApplicationContainer serverApps;
uint16_t m_delayswitch;
double m_Delay;
double m_Lossrate;
};

// 在类外重定义ExternallyTest的m_lossModel属性，并初始化为1
uint32_t ExternallyTest::m_lossModel(1);
// 在类外重定义ExternallyTest的m_antennaHeightAboveZ属性，并初始化为1.5
double ExternallyTest::m_antennaHeightAboveZ(1.5);
// 定义ExternallyTest类的构造函数 通过使用成员初始化列表的方式将一些ExternallyTest类的属性进行显式的初始化
ExternallyTest::ExternallyTest() :
m_totalTime(50),
m_pcap(true),
m_servPort(9),
m_simulationStep(30),
m_protocol(2),
m_routingTables(1),
m_80211_ltemode (1),
m_phyMode(4),
m_phy_pMode(11),
m_phy_WAVEMode(3),
m_lr_wpanMode(1),
m_seed(10),
m_run(1),
m_fading(0),
m_ltefading(0),
m_verbose (0),
m_RemoteStationManager(1),
m_EnergyDetectionThreshold(-110.0),
m_CcaMode1Threshold(-110.0),
m_txp(15.0),
m_rxg(0),
m_rxnf(10),
m_Antenna(1),
m_asciiTrace (1),
m_trName ("co-simulation-ascii"),
m_nodeSpeed (20),
m_nodePause (2),
m_streamIndex(0),
m_logFile ("node-CourseChange.adj.log"),
m_MobilityModel(2),
m_rtt(5.0),
m_qudong_mode(1),
m_trans_protocol_mode(1),
m_otherlogTrace (1),
m_delayswitch(1),
m_Delay(0.0),
m_Lossrate(0.0)
{
/*
 * 拥有所谓的“GlobalValue”。(即全局变量）
 * 	GlobalValue将从（按顺序）获得它的值：

		     初始值配置在定义的位置，
		     从NS_GLOBAL_VALUE环境变量中，
		     从命令行，
		     通过显式调用SetValue（）或Bind（）。
		期望将此类的实例分配为静态全局变量，该全局变量应用于存储可配置的全局状态。
 */

// 通过设置SimulatorImplementationType属性，构造函数设置仿真器(为全局变量）
// 实现类型为外部驱动模拟器,因而在整个仿真过程中将使用外部驱动模拟器。
GlobalValue::Bind("SimulatorImplementationType", StringValue(
"ns3::ExternallyDrivenSim"));
}

ExternallyTest::~ExternallyTest()// 析构函数
{
}

// 定义利用命令行参数来配置ExternallyTest参数的函数
void ExternallyTest::Configure(int argc, char *argv[])
{
// 通过CommandLine类定义一个名为cmd的对象
CommandLine cmd;
// cmd对象使用AddValue函数来添加仿真需要的可设置的参数，通过设置这些参数来改变ExternallyTest的相关属性
cmd.AddValue("pcap", "Enable PCAP traces on interfaces. [Default 1(true)]",
m_pcap);
cmd.AddValue("totalTime", "time of the simulation. [Default 50 sec]",
m_totalTime);
cmd.AddValue("simulationStep", "time of one simulation step. [Default 30 Millisec]",
m_simulationStep);
cmd.AddValue("serverPort", "the port of server. [Default 9]",
m_servPort);
cmd.AddValue("protocol",
"0=NONE;1=OLSR;2=AODV;3=DSDV;4=default Ipv4StaticRouting;5=DSR;6=AddHostRouteTo Ipv4StaticRouting;7=Ipv4GlobalRouting [Default 2]",
m_protocol);
cmd.AddValue("routingTables",
"Dump routing tables at t=rtt seconds 0:don't Dump routing tables;!=0:Dump routing tables [Default 1]",
m_routingTables);
cmd.AddValue ("rtt", "Dump routing tables at rtt seconds", m_rtt);
cmd.AddValue ("seed", "rng SeedManager Set the run seed of simulation.  [Default 10]", m_seed);
cmd.AddValue ("run", "rng SeedManager Set the run number of simulation.  [Default 1]", m_run);
cmd.AddValue ("lossModel", "1=Friis;2=ItuR1411Los;3=TwoRayGround;4=LogDistance;5=Cost231;6=Jakes;7=Kun2600Mhz;8=Range", m_lossModel);
cmd.AddValue ("fading", "0=None;1=Nakagami;2=NormalRandom;3=NormalRandom plus Nakagami", m_fading);
cmd.AddValue ("phyMode", "1=DsssRate1Mbps;2=DsssRate2Mbps;3=DsssRate5_5Mbps;4=DsssRate11Mbps", m_phyMode);
cmd.AddValue ("verbose", "0=quiet;1=verbose", m_verbose);
cmd.AddValue ("RemoteStationManager","1=AarfWifiManager;2=ConstantRateWifiManager", m_RemoteStationManager);
cmd.AddValue ("EnergyDetectionThreshold",
"The energy of a received signal should be higher than this threshold (dbm) to allow the PHY layer to detect the signal. ",
m_EnergyDetectionThreshold);
cmd.AddValue ("CcaMode1Threshold",
"The energy of a received signal should be higher than this threshold (dbm) to allow the PHY layer to declare CCA BUSY state. ",
m_CcaMode1Threshold);
cmd.AddValue ("TxPower", "available transmission level (dbm). ", m_txp);
cmd.AddValue ("RxGain", "Reception gain (dB). ", m_rxg);
cmd.AddValue ("RxNoiseFigure",
"Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver. ", m_rxnf);
cmd.AddValue ("Antennas", "The number of antennas on the device. ", m_Antenna);
cmd.AddValue ("asciiTrace", "Dump ASCII Trace data of wifi phy", m_asciiTrace);
cmd.AddValue ("speed", "RandomWaypointMobilityModel's max Node speed (m/s)", m_nodeSpeed);
cmd.AddValue ("pause", "RandomWaypointMobilityModel's Node pause (s)", m_nodePause);
cmd.AddValue ("logFile", "Log file", m_logFile);
cmd.AddValue ("MobilityModel",
"the model of node Mobility. 1=RandomWaypointMobilityModel;2=ConstantPositionMobilityModel;3=ConstantVelocityMobilityModel;4=ConstantAccelerationMobilityModel;5=GaussMarkovMobilityModel;6=RandomDirection2dMobilityModel;7=RandomWalk2dMobilityModel",
m_MobilityModel);
cmd.AddValue ("qudong_mode", "1=time_qudong;2=event_qudong", m_qudong_mode);
cmd.AddValue ("trans_protocol_mode", "1=UDP;2=TCP", m_trans_protocol_mode);
cmd.AddValue ("otherlogTrace", "Dump Trace data of other information", m_otherlogTrace);
cmd.AddValue ("80211_LTE-D2Dmode", "1=802.11b; 2=802.11p; 3=WAVE-PHY; 4 = LTE-D2D-COMM; 5 = lr-wpan", m_80211_ltemode);
cmd.AddValue ("phy_pMode", "1=OfdmRate6Mbps;2=OfdmRate9Mbps;3=OfdmRate12Mbps;4=OfdmRate18Mbps;5=OfdmRate24Mbps;6=OfdmRate36Mbps;7=OfdmRate48Mbps;8=OfdmRate54Mbps;9=OfdmRate3MbpsBW10MHz;10=OfdmRate4_5MbpsBW10MHz;11=OfdmRate6MbpsBW10MHz;12=OfdmRate9MbpsBW10MHz;13=OfdmRate12MbpsBW10MHz;14=OfdmRate18MbpsBW10MHz;15=OfdmRate24MbpsBW10MHz;16=OfdmRate27MbpsBW10MHz",
m_phy_pMode);
cmd.AddValue ("phy_WAVEMode", "1=OfdmRate3MbpsBW10MHz;2=OfdmRate4_5MbpsBW10MHz;3=OfdmRate6MbpsBW10MHz;4=OfdmRate9MbpsBW10MHz;5=OfdmRate12MbpsBW10MHz;6=OfdmRate18MbpsBW10MHz;7=OfdmRate24MbpsBW10MHz;8=OfdmRate27MbpsBW10MHz",
m_phy_WAVEMode);
cmd.AddValue ("lr_wpanMode", "1=SingleModelSpectrumChannel;2=MultiModelSpectrumChannel",
m_lr_wpanMode);
cmd.AddValue ("ltefading", "0=None;1=ConstantSpectrum;2=FriisSpectrum", m_ltefading);
cmd.AddValue ("setdelayswitch", "1=off:no use set delay;2=on:use set delay", m_delayswitch);
cmd.AddValue ("setdelay", "the packet delay user set", m_Delay);
cmd.AddValue ("lossrate", "the packet loss rate[0,1] user set", m_Lossrate);
// cmd类解析命令行参数
cmd.Parse(argc, argv);
}

// 定义Set_ExternallyDrivenSim_config函数 一些ExternallyDrivenSim类运行需要的参数配置
void ExternallyTest::Set_ExternallyDrivenSim_config (void)
{
ExternallyDrivenSim::SetSimulationStep(MilliSeconds(m_simulationStep));
ExternallyDrivenSim::Setqudongmode(m_qudong_mode);
ExternallyDrivenSim::Set_trans_protocolmode(m_trans_protocol_mode);
ExternallyDrivenSim::get_node_number(m_nodes);
}

// 定义TakePositionFromFile函数 从文件中获得节点位置和移动模型
ListPositionAllocator* ExternallyTest::TakePositionFromFile(char file[])
{
NS_LOG_INFO("Building node topology.");//运行函数通知
// 定义坐标分隔判断符','
const char u =
{
','
};
std::string line;// 定义string变量line
std::string ll;// 定义string变量ll
std::ifstream myfile(file);// 用file来创建一个文件输入流myfile
std::getline(myfile, line);// 使用getline得到文件输入流的第一行数据
// 将读取到的line字符串在','之前的部分读取到ll内
for (int c = 0; (line[c] != u); c++)
{
ll = ll + line[c];
}
// 定义tt字符串指针，将const指针返回给tt。这是对内部数据的处理。将stirng类型ll转换成字符串tt
const char* tt = ll.c_str();
m_nodes = atol(tt);// tt由字符串类型转换为长整型变量m_nodes，获得节点的个数
m_server = m_nodes - 1;// 定义m_server为m_nodes - 1
// 创建节点以及其物理层和mac层
ConfigureNodes ();
//wifi.SetStandard(WIFI_PHY_STANDARD_80211b);//设置wifi标准
//NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();//创建wifimac层
//YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();//创建wifiphy层
//wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11);
//YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();//定义wifi通道
//wifiPhy.SetChannel(wifiChannel.Create());//每个物理层安装wifi通道
//Ssid ssid = Ssid("wifi-default");//设置ssid
//wifi.SetRemoteStationManager("ns3::AarfWifiManager");//设置远程节点的wifi模式
//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("DsssRate11Mbps"), "ControlMode",
//                              StringValue ("DsssRate11Mbps"));
//wifiMac.SetType("ns3::AdhocWifiMac");//mac层设置wifi类型
//devices installing
//devices = wifi.Install(wifiPhy, wifiMac, nodes);//安装wifi设备
//if (m_pcap)
//wifiPhy.EnablePcapAll(std::string("gg-"));//创建信息监控
// 从保存拓扑的txt文件中获得节点拓扑，首先初始化一个值为0的double数组，用于之后保存某节点坐标
double g[3] =
{ 0
};
// 创建ListPositionAllocator指针listPosNod，指向使用ListPositionAllocator构造函数new出来的对象
ns3::ListPositionAllocator *listPosNod = new ns3::ListPositionAllocator();
if (myfile.is_open())//包装测试打开的文件。如果文件打开为真
{
/*
	* @brief 快速错误检查。
	* @return 如果eofbit已设置则返回true。eofbit表示输入操作到达输入序列的末尾。
	* 读到输入流末尾后的终止空值则为真，没有读到则为否
 */
while (!myfile.eof())
{
std::getline(myfile, line);// 将读取文件输入流myfile的第二行
int count = 0;
int k = 0;
while (k != 3)// 当k不等于3时
{
ll = "";
for (; (line[count] != u); count++)
{
ll = ll + line[count];// 读取line
}
const char* tt = ll.c_str();
g[k] = atof(tt);
if (k == 2)
{
listPosNod->Add(*new ns3::Vector3D(g[0], g[1], g[2]));
}
k++;
count++;
}// 以上为读取并解析坐标代码
}
myfile.close();// 文件输入流myfile关闭
}
else
{
NS_LOG_INFO("Unable to open "<< file<<"\n");// 打不开文件file
}
return (listPosNod);// 返回坐标指针
}

// 定义ConfigureNodes函数 根据节点个数属性创建相应个数的节点
void
ExternallyTest::ConfigureNodes ()
{
nodes.Create(m_nodes);
}

// 定义Configurephymac函数 配置物理层（信道模型）和mac层并生成设备
void
ExternallyTest::Configurephymac ()
{
SetupAdhocDevices ();
}

// 定义Configurephymac函数 生成adhoc设备
void
ExternallyTest::SetupAdhocDevices ()
{
if (m_lossModel == 1)
{
m_lossModelName = "ns3::FriisPropagationLossModel";
NS_LOG_UNCOND ("lossModelName is FriisPropagationLossModel");
}
else if (m_lossModel == 2)
{
m_lossModelName = "ns3::ItuR1411LosPropagationLossModel";
NS_LOG_UNCOND ("lossModelName is ItuR1411LosPropagationLossModel");
}
else if (m_lossModel == 3)
{
m_lossModelName = "ns3::TwoRayGroundPropagationLossModel";
NS_LOG_UNCOND ("lossModelName is TwoRayGroundPropagationLossModel");
}
else if (m_lossModel == 4)
{
m_lossModelName = "ns3::LogDistancePropagationLossModel";
NS_LOG_UNCOND ("lossModelName is LogDistancePropagationLossModel");
}
else if (m_lossModel == 5)
{
m_lossModelName = "ns3::Cost231PropagationLossModel";
NS_LOG_UNCOND ("lossModelName is Cost231PropagationLossModel");
}
else if (m_lossModel == 6)
{
m_lossModelName = "ns3::JakesPropagationLossModel";
NS_LOG_UNCOND ("lossModelName is JakesPropagationLossModel");
}
else if (m_lossModel == 7)
{
m_lossModelName = "ns3::Kun2600MhzPropagationLossModel";
NS_LOG_UNCOND ("lossModelName is Kun2600MhzPropagationLossModel");
}
else if (m_lossModel == 8)
{
m_lossModelName = "ns3::RangePropagationLossModel";
NS_LOG_UNCOND ("lossModelName is RangePropagationLossModel");
}
else
{
// 不支持的 propagation loss 模型.
// 被当做 ERROR，输出ERROR信息
NS_LOG_ERROR ("Invalid propagation loss model specified.  Values must be [1-8]");
}

LrWpanHelper lrWpanHelper;
switch (m_80211_ltemode)
{
case 1:
NS_LOG_UNCOND ("chose 80211mode is 802.11b");
break;
case 2:
NS_LOG_UNCOND ("chose 80211mode is 802.11p");
break;
case 3:
NS_LOG_UNCOND ("chose 80211mode is WAVE-PHY");
break;
case 4:
NS_LOG_UNCOND ("chose mode is LTE-D2D-COMM");
break;
case 5:
NS_LOG_UNCOND ("chose mode is lr-wpan");
break;
default:
NS_FATAL_ERROR ("No such PHY mode for mode index" << m_80211_ltemode);
break;
}
// 定义信号的频率变量
double freq = 0.0;
//Set the frequency
uint32_t ulEarfcn;
uint16_t ulBandwidth;
if (m_80211_ltemode == 1)
{
// 这里用的802.11b的频率为2.4 GHz
freq = 2.4120e9;
}
else if((m_80211_ltemode == 2)||(m_80211_ltemode == 3))
{
// 这里用的802.11p的频率为5.9 GHz
freq = 5.9e9;
}
else if(m_80211_ltemode == 4)
{
ulEarfcn = 18100;
ulBandwidth = 50;
}

if(m_80211_ltemode == 4)
{
//Create the helpers
lteHelper = CreateObject<LteHelper> ();

//Create and set the EPC helper
epcHelper = CreateObject<PointToPointEpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);

////Create Sidelink helper and set lteHelper
proseHelper = CreateObject<LteSidelinkHelper> ();
proseHelper->SetLteHelper (lteHelper);

//Enable Sidelink
lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));
}

if((m_80211_ltemode == 1)||(m_80211_ltemode == 2)||(m_80211_ltemode == 3))
{
// 安装 propagation 模型
YansWifiChannelHelper wifiChannel;
// 设置信号传播速度损耗模型，设置为常量速度损耗模型
wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
if (m_lossModel == 3)
{
// two-ray propagation 模型，要求天线高度
wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq),
"HeightAboveZ", DoubleValue (m_antennaHeightAboveZ));
}
else if(m_lossModel == 4)
{
// LogDistance propagation 模型，要求参考损耗参数，这里设为47.2
wifiChannel.AddPropagationLoss (m_lossModelName,"Exponent",DoubleValue (3.5),
"ReferenceLoss",DoubleValue (47.2));
}
else if(m_lossModel == 6||m_lossModel == 7||m_lossModel == 8)
{
wifiChannel.AddPropagationLoss (m_lossModelName);
}
else
{
wifiChannel.AddPropagationLoss (m_lossModelName,"Frequency",DoubleValue (freq));
}

// Propagation loss 模型是可以叠加的，这里在主要的传播损耗模型下叠加了相应的衰减损耗模型
// m_fading == 0,不叠加相应的衰减损耗模型
if (m_fading != 0)
{
if(m_fading == 1)
{
// 衰减损耗模型为NakagamiPropagationLossModel（一种快衰减模型）
wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
}
if(m_fading == 2)
{
// 衰减损耗模型为RandomPropagationLossModel（一种慢衰减模型）
wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel",
"Variable",StringValue ("ns3::NormalRandomVariable[Mean=0.0]"),
"Variable",StringValue ("ns3::NormalRandomVariable[Variance=49.0]"));
}
if(m_fading == 3)
{
// 衰减损耗模型为RandomPropagationLossModel加上NakagamiPropagationLossModel（慢衰减叠加快衰减）
wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel",
"Variable",StringValue ("ns3::NormalRandomVariable[Mean=0.0]"),
"Variable",StringValue ("ns3::NormalRandomVariable[Variance=49.0]"));
wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
}
}

// 创建the YansWifiChannel对象
channel = wifiChannel.Create ();
}
else if(m_80211_ltemode == 4)
{
//Set pathloss model
lteHelper->SetAttribute ("PathlossModel", StringValue (m_lossModelName));
if(m_ltefading == 0)
lteHelper->SetAttribute ("FadingModel", StringValue (""));
else if(m_ltefading == 1)
lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::ConstantSpectrumPropagationLossModel"));
else
lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
// channel model initialization
lteHelper->Initialize ();

// Since we are not installing eNB, we need to set the frequency attribute of pathloss model here
double ulFreq = LteSpectrumValueHelper::GetCarrierFrequency (ulEarfcn);
NS_LOG_LOGIC ("UL freq: " << ulFreq);
Ptr<Object> uplinkPathlossModel = lteHelper->GetUplinkPathlossModel ();
Ptr<PropagationLossModel> lossModel = uplinkPathlossModel->GetObject<PropagationLossModel> ();
NS_ABORT_MSG_IF (lossModel == NULL, "No PathLossModel");
if(m_lossModel == 1||m_lossModel == 2||m_lossModel == 3||m_lossModel == 5)
{
bool ulFreqOk = uplinkPathlossModel->SetAttributeFailSafe ("Frequency", DoubleValue (ulFreq));
if (!ulFreqOk)
{
NS_LOG_WARN ("UL propagation model does not have a Frequency attribute");
}
}
if(m_lossModel == 3)
{
uplinkPathlossModel->SetAttributeFailSafe ("HeightAboveZ", DoubleValue (m_antennaHeightAboveZ));
}
if(m_lossModel == 4)
{
uplinkPathlossModel->SetAttributeFailSafe ("Exponent",DoubleValue (3.5));
uplinkPathlossModel->SetAttributeFailSafe ("ReferenceLoss",DoubleValue (47.2));
}
}

wifi80211p = Wifi80211pHelper::Default ();
waveHelper = WaveHelper::Default ();
// 下面的帮助程序将帮助我们整合我们想要的wifi网卡
YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
// 设置wifi网卡的物理层参数
wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (m_EnergyDetectionThreshold) );
wifiPhy.Set ("CcaMode1Threshold", DoubleValue (m_CcaMode1Threshold) );
wifiPhy.Set ("TxPowerStart", DoubleValue (m_txp) );
wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp) );
wifiPhy.Set ("RxGain", DoubleValue (m_rxg) );
wifiPhy.Set ("RxNoiseFigure", DoubleValue (m_rxnf) );
wifiPhy.Set ("Antennas", UintegerValue (m_Antenna) );
wifiPhy.SetChannel (channel);// 将信道模型整合到wifi网卡的物理层
// ns-3支持产生pcap跟踪文件
// 设置要使用的PCAP跟踪的数据链接类型为DLT_IEEE802_11
wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
wavePhy.Set ("EnergyDetectionThreshold", DoubleValue (m_EnergyDetectionThreshold) );
wavePhy.Set ("CcaMode1Threshold", DoubleValue (m_CcaMode1Threshold) );
wavePhy.Set ("TxPowerStart", DoubleValue (m_txp) );
wavePhy.Set ("TxPowerEnd", DoubleValue (m_txp) );
wavePhy.Set ("RxGain", DoubleValue (m_rxg) );
wavePhy.Set ("RxNoiseFigure", DoubleValue (m_rxnf) );
wavePhy.Set ("Antennas", UintegerValue (m_Antenna) );
wavePhy.SetChannel (channel);
wavePhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (m_txp));
Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (m_rxnf));

if (m_verbose)
{
wifi.EnableLogComponents ();// 打开wifi日志
wifi80211p.EnableLogComponents ();
waveHelper.EnableLogComponents ();
lrWpanHelper.EnableLogComponents ();

LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);

LogComponentEnable ("LteUeRrc", logLevel);
LogComponentEnable ("LteUeMac", logLevel);
LogComponentEnable ("LteSpectrumPhy", logLevel);
LogComponentEnable ("LteUePhy", logLevel);
}

if(m_80211_ltemode == 1)
{
// 安装 802.11b 物理层传输标准
wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
}
else if(m_80211_ltemode == 2)
{
if(m_phy_pMode<=8)
{
wifi80211p.SetStandard (WIFI_PHY_STANDARD_80211a);
}
else
{
wifi80211p.SetStandard (WIFI_PHY_STANDARD_80211_10MHZ);
}
}

if(m_80211_ltemode == 1)
{
Ssid ssid = Ssid("wifi-80211b");// 设置ssid
}
else if(m_80211_ltemode == 2)
{
Ssid ssid = Ssid("wifi-80211p");
}
else if(m_80211_ltemode == 3)
{
Ssid ssid = Ssid("wifi-WAVE");
}
// 设置远程节点的wifi物理层模式
if(m_RemoteStationManager == 1)
{
// AarfWifiManager模式：IEEE 802.11速率自适应模式
NS_LOG_UNCOND ("chose AarfWifiManager for WifiRemoteStationManager");
wifi.SetRemoteStationManager("ns3::AarfWifiManager");
wifi80211p.SetRemoteStationManager("ns3::AarfWifiManager");
waveHelper.SetRemoteStationManager("ns3::AarfWifiManager");
}
else if(m_RemoteStationManager == 2)
{
if(m_80211_ltemode == 1)
{
NS_LOG_UNCOND ("chose ConstantRateWifiManager for 802.11b,the phyMode Setup for 802.11b is "
<< m_phyModeName);
// ConstantRateWifiManager模式 速率固定模式
wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
"DataMode",StringValue (m_phyModeName),
"ControlMode",StringValue (m_phyModeName));
}

else if(m_80211_ltemode == 2)
{
NS_LOG_UNCOND ("chose ConstantRateWifiManager for 802.11p,the phyMode Setup for 802.11p is "
<< m_phyMode_pName);
wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
"DataMode",StringValue (m_phyMode_pName),
"ControlMode",StringValue (m_phyMode_pName));
}
else if(m_80211_ltemode == 3)
{
NS_LOG_UNCOND ("chose ConstantRateWifiManager for WAVE,the phyMode Setup for WAVE is "
<< m_phyMode_WAVEName);
waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
"DataMode",StringValue (m_phyMode_WAVEName),
"ControlMode",StringValue (m_phyMode_WAVEName));
}
}

// 添加AdhocWifiMac层
WifiMacHelper wifiMac;
wifiMac.SetType ("ns3::AdhocWifiMac");

NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();

QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();

// 将wifi物理层和mac层整合到节点上，使得该节点成为具有网卡的设备
if(m_80211_ltemode == 1)
{
devices = wifi.Install (wifiPhy, wifiMac, nodes);
}
else if(m_80211_ltemode == 2)
{
devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
}
else if(m_80211_ltemode == 3)
{
devices = waveHelper.Install (wavePhy, waveMac, nodes);
const SchInfo schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);
const TxProfile txProfile = TxProfile (SCH1);
Ptr<WaveNetDevice> WaveNetDev[m_nodes];
for(int i = 0;i < m_nodes;i++)
{
WaveNetDev[i] = DynamicCast<WaveNetDevice> (devices.Get (i));
Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, WaveNetDev[i], schInfo);
// An important point is that the receiver should also be assigned channel
// access for the same channel to receive packets.
Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, WaveNetDev[i], schInfo);
Simulator::Schedule (Seconds (0.0), &WaveNetDevice::RegisterTxProfile, WaveNetDev[i], txProfile);
}
}
else if(m_80211_ltemode == 4)
{
devices = lteHelper->InstallUeDevice (nodes);
//Sidelink pre-configuration for the UEs
Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
ueSidelinkConfiguration->SetSlEnabled (true);

LteRrcSap::SlPreconfiguration preconfiguration;

preconfiguration.preconfigGeneral.carrierFreq = ulEarfcn;
preconfiguration.preconfigGeneral.slBandwidth = ulBandwidth;
preconfiguration.preconfigComm.nbPools = 1;

LteSlPreconfigPoolFactory pfactory;

//Control
pfactory.SetControlPeriod ("sf40");
pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
pfactory.SetControlOffset (0);
pfactory.SetControlPrbNum (22);
pfactory.SetControlPrbStart (0);
pfactory.SetControlPrbEnd (49);

//Data
pfactory.SetDataBitmap (0xFFFFFFFFFF);
pfactory.SetDataOffset (8); //After 8 subframes of PSCCH
pfactory.SetDataPrbNum (25);
pfactory.SetDataPrbStart (0);
pfactory.SetDataPrbEnd (49);

preconfiguration.preconfigComm.pools[0] = pfactory.CreatePool ();

ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
lteHelper->InstallSidelinkConfiguration (devices, ueSidelinkConfiguration);
}
else if(m_80211_ltemode == 5)
{
	std::cout<<"ddddddddddddddddddd"<<std::endl;
	Ptr<SpectrumChannel> channel;
	if(m_lr_wpanMode == 1)
	{
		channel = CreateObject<SingleModelSpectrumChannel> ();
	}
	else
	{
		channel = CreateObject<MultiModelSpectrumChannel> ();
	}
	std::cout<<"aaaaaaaaaaaaaaaaaaa"<<std::endl;
	Ptr<PropagationLossModel> lossModel;
	Ptr<PropagationLossModel> fadeModel;
	switch(m_lossModel)
	{
	case 1:lossModel=CreateObject<FriisPropagationLossModel> ();break;
	case 2:lossModel=CreateObject<ItuR1411LosPropagationLossModel> ();break;
	case 3:lossModel=CreateObject<TwoRayGroundPropagationLossModel> ();break;
	case 4:lossModel=CreateObject<LogDistancePropagationLossModel> ();break;
	case 5:lossModel=CreateObject<Cost231PropagationLossModel> ();break;
	case 6:lossModel=CreateObject<JakesPropagationLossModel> ();break;
	case 7:lossModel=CreateObject<Kun2600MhzPropagationLossModel> ();break;
	case 8:lossModel=CreateObject<RangePropagationLossModel> ();break;
	}
	channel->AddPropagationLossModel (lossModel);
	std::cout<<"bbbbbbbbbbbbbbbbbbbbbb"<<std::endl;
	switch(m_fading)
	{
		case 0:break;
		case 1:fadeModel = CreateObject<NakagamiPropagationLossModel> ();
		channel->AddPropagationLossModel (fadeModel);break;
		case 2:fadeModel = CreateObject<RandomPropagationLossModel> ();
				channel->AddPropagationLossModel (fadeModel);break;
		case 3:fadeModel = CreateObject<RandomPropagationLossModel> ();
						channel->AddPropagationLossModel (fadeModel);
			   fadeModel = CreateObject<NakagamiPropagationLossModel> ();
			   channel->AddPropagationLossModel (fadeModel);break;
	}
	Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
	channel->SetPropagationDelayModel (delayModel);
	int mac16address = 1;
	for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); i++)
	    {
	      Ptr<Node> node = *i;

	      Ptr<LrWpanNetDevice> netDevice = CreateObject<LrWpanNetDevice> ();
	      std::string mac_16 = "00:0"+std::to_string(mac16address);
	      netDevice->SetAddress (Mac16Address (mac_16.c_str()));
	      netDevice->SetChannel (channel);
	      node->AddDevice (netDevice);
	      netDevice->GetMac ()->SetMcpsDataIndicationCallback (MakeCallback (&DataIndication));
	      netDevice->SetNode (node);
	      devices.Add (netDevice);
	      mac16address++;
	    }
	  SixLowPanHelper sixlowpan;
	  devices = sixlowpan.Install (devices);
}
// 相应网络跟踪文件生成，有ascii和pcap两种文件类型
if (m_asciiTrace != 0)
{
AsciiTraceHelper ascii;
Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (m_trName + ".tr").c_str ());
if((m_80211_ltemode == 1)||(m_80211_ltemode == 2))
{
wifiPhy.EnableAsciiAll (osw);
}
else if(m_80211_ltemode == 3)
{
wavePhy.EnableAsciiAll (osw);
}
else if(m_80211_ltemode == 5)
{
lrWpanHelper.EnableAsciiAll (osw);
}
}

if (m_pcap)
{
if((m_80211_ltemode == 1)||(m_80211_ltemode == 2))
{
wifiPhy.EnablePcapAll (std::string("cosimulation-pcap"));
}
else if(m_80211_ltemode == 3)
{
wavePhy.EnablePcapAll (std::string("cosimulation-pcap"));
}
else if(m_80211_ltemode == 5)
{
lrWpanHelper.EnablePcapAll (std::string("cosimulation-pcap"));
}
}

}

// 定义ConfigureDevices函数 使用回调函数为设备们添加物理层和mac层的发送和丢包跟踪
void
ExternallyTest::ConfigureDevicestracing ()
{
NS_LOG_INFO ("trace of wifi Tx and drop");
if(m_otherlogTrace)
{
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/Tx",
MakeCallback (&WifiPhyStats::PhyTxTrace, m_wifiPhyStats));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/PhyEntities/*/State/Tx",
MakeCallback (&WifiPhyStats::PhyTxTrace, m_wifiPhyStats));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
MakeCallback (&WifiMacStats::MacTxTrace, m_WifiMacStats));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop",
MakeBoundCallback (&WifiPhyStats::PhyTxDrop, &m_phy_drop_os));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop",
MakeBoundCallback (&WifiPhyStats::PhyRxDrop, &m_phy_drop_os));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop",
MakeBoundCallback (&WifiMacStats::MacTxDrop, &m_mac_drop_os));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop",
MakeBoundCallback (&WifiMacStats::MacRxDrop, &m_mac_drop_os));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/PhyEntities/*/PhyTxDrop",
MakeBoundCallback (&WifiPhyStats::PhyTxDrop, &m_phy_drop_os));
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/PhyEntities/*/PhyRxDrop",
MakeBoundCallback (&WifiPhyStats::PhyRxDrop, &m_phy_drop_os));
}
}

// 定义ConfigureMobility函数 为节点们配置移动性质
void
ExternallyTest::ConfigureMobility ()
{
SetupAdhocMobilityNodes ();
}

// 定义SetupLogFile函数 产生节点移动路径跟踪文件，物理层和mac层跟踪文件
void
ExternallyTest::SetupLogFile ()
{
// 打开需要输入的日志文件
m_os.open (m_logFile.c_str (),std::ios::ate|std::ios::out);
m_phy_drop_os.open("Tx and Rx phy drop.log",std::ios::ate|std::ios::out);
m_mac_drop_os.open("Tx and Rx mac drop.log",std::ios::ate|std::ios::out);
}

// 定义SetupAdhocMobilityNodes函数 为adhoc节点安装节点移动模型
void
ExternallyTest::SetupAdhocMobilityNodes ()
{
MobilityHelper mobility;

// 各种节点移动模型的选择
if(m_MobilityModel == 1)
{
ObjectFactory pos;
pos.SetTypeId ("ns3::RandomBoxPositionAllocator");
pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=-500.0|Max=500.0]"));
pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=-500.0|Max=500.0]"));
pos.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=-500.0|Max=500.0]"));

Ptr<PositionAllocator> TaPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
std::stringstream ssSpeed;
ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
std::stringstream ssPause;
ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";
mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
"Speed", StringValue (ssSpeed.str ()),
"Pause", StringValue (ssPause.str ()),
"PositionAllocator", PointerValue (TaPositionAlloc));
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
else if(m_MobilityModel == 2)
{
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 0);
}
else if(m_MobilityModel == 3)
{
mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
else if(m_MobilityModel == 4)
{
mobility.SetMobilityModel("ns3::ConstantAccelerationMobilityModel");
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
else if(m_MobilityModel == 5)
{
mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
"Bounds", BoxValue (Box (0, 150000, 0, 150000, 0, 10000)),
"TimeStep", TimeValue (Seconds (0.5)),
"Alpha", DoubleValue (0.85),
"MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=800|Max=1200]"),
"MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
"MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
"NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
"NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
"NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
else if(m_MobilityModel == 6)
{
std::stringstream ssSpeed;
ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
std::stringstream ssPause;
ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";
mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
"Speed", StringValue (ssSpeed.str ()),
"Pause", StringValue (ssPause.str ()),
"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
else if(m_MobilityModel == 7)
{
std::stringstream ssSpeed;
ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
"Time", TimeValue (Seconds (0.5)),
"Distance", DoubleValue (1),
"Mode",EnumValue(ns3::RandomWalk2dMobilityModel::MODE_DISTANCE),
"Direction", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
"Speed",StringValue(ssSpeed.str ()),
"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
WaveBsmHelper::GetNodesMoving ().resize (m_nodes, 1);
}
mobility.SetPositionAllocator (taPositionAlloc);// 使用节点位置列表分配节点位置
mobility.Install (nodes);// 移动模型安装在节点上
if(m_MobilityModel == 3)
{
for(int i = 0;i<m_nodes;i++)
{
nodes.Get(i)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (1, 1, 1));
}
}
if(m_MobilityModel == 4)
{
for(int i = 0;i<m_nodes;i++)
{
nodes.Get(i)->GetObject<ConstantAccelerationMobilityModel> ()->SetVelocityAndAcceleration (Vector (1, 1, 1),Vector (1, 1, 1));
}
}
m_streamIndex += mobility.AssignStreams (nodes, m_streamIndex);

// 为日志文件配置回调函数CourseChange
Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
MakeBoundCallback (&ExternallyTest::CourseChange, &m_os));
}

// 定义CourseChange函数 向节点移动路径跟踪文件写入节点路径 当CourseChange事件发生时打印节点实际位置和速度

void
ExternallyTest::
CourseChange (std::ostream *os, std::string context, Ptr<const MobilityModel> mobility)
{
Vector pos = mobility->GetPosition (); // 获取节点位置
Vector vel = mobility->GetVelocity (); // 获取节点速度
// 当选择的信道传输模型为two-ray propagation 模型时，节点位置要加上天线的高度
if (m_lossModel == 3)
{
pos.z = pos.z+m_antennaHeightAboveZ;
}

int nodeId = mobility->GetObject<Node> ()->GetId ();
//NS_LOG_UNCOND ("Changing pos for node=" << nodeId << " at " << Simulator::Now () );

// 打印节点位置和速度信息到输出流
*os <<"Changing pos for node=" << nodeId << " at (ns3 simulation time) "
<< Simulator::Now ()<< " POS: x=" << pos.x << ", y=" << pos.y
<< ", z(if lossModel tworay with antennaHeight)=" << pos.z << "; VEL: x="
<< vel.x << ", y=" << vel.y<< ", z=" << vel.z << std::endl;
}

// 定义ConfigureLogTracing函数 一些跟踪文件是否产生的开关，以及终端日志是否产生的开关
void
ExternallyTest::ConfigureLogTracing ()
{
NS_LOG_INFO (
"Setup the trace of Mobility and lteHelper,log of ExternallyDrivenSim and Application");
if(m_otherlogTrace)
{
SetupLogFile ();

AsciiTraceHelper ascii;
MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (m_trName + ".mob"));
if(m_80211_ltemode == 4)
{
lteHelper->EnableSlPscchMacTraces ();
lteHelper->EnableSlPsschMacTraces ();
lteHelper->EnableSlRxPhyTraces ();
lteHelper->EnableSlPscchRxPhyTraces ();
}
}
if (m_verbose)
{
LogComponentEnable("ExternallyDrivenSim", LOG_LEVEL_INFO);
LogComponentEnable("UdpEchoClientNewApplication", LOG_LEVEL_INFO);
LogComponentEnable("UdpEchoServerNewApplication", LOG_LEVEL_INFO);
LogComponentEnable("TcpClientApplication", LOG_LEVEL_INFO);
LogComponentEnable("TcpServerApplication", LOG_LEVEL_INFO);
}
}

// 定义创建节点、物理层、mac层和节点移动性质函数
void ExternallyTest::CreateNodesphymacmobility()
{
NS_LOG_INFO ("setting the node default phy,channel and mac parameters");
// 这里file字符设置保存节点拓扑文件在系统中的路径
char file[] = "/home/ling/ns3/ns-3.27/scratch/top.txt";
ListPositionAllocator* listPosNod = TakePositionFromFile(file);
taPositionAlloc = listPosNod;
ConfigureDevicestracing ();
ConfigureMobility();
Configurephymac();
}

// 定义SetupRoutingProtocol函数 安装IP层路由协议
void ExternallyTest::SetupRoutingProtocol (NodeContainer & c)
{
AodvHelper aodv;
OlsrHelper olsr;
DsdvHelper dsdv;
Ipv4StaticRoutingHelper defaultstaticRouting;
DsrHelper dsr;
DsrMainHelper dsrMain;
Ipv4GlobalRoutingHelper GlobalRouting;
Ipv4ListRoutingHelper list;
InternetStackHelper internet;

// 配置路由表输出文件的参数
Time rtt = Seconds (m_rtt);
AsciiTraceHelper ascii;
Ptr<OutputStreamWrapper> rtw = ascii.CreateFileStream ("routing_table");

switch (m_protocol)
{
case 0:
m_protocolName = "NONE";
break;
case 1:
if (m_routingTables != 0)
{
olsr.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (olsr, 100);
m_protocolName = "OLSR";
break;
case 2:
if (m_routingTables != 0)
{
aodv.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (aodv, 100);
m_protocolName = "AODV";
break;
case 3:
if (m_routingTables != 0)
{
dsdv.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (dsdv, 100);
m_protocolName = "DSDV";
break;
case 4:
if (m_routingTables != 0)
{
defaultstaticRouting.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (defaultstaticRouting, 100);
m_protocolName = "default Ipv4StaticRouting";
break;
case 5:
// setup is diff
m_protocolName = "DSR";
break;
case 6:
if (m_routingTables != 0)// setup is diff
{
ipv4RoutingHelper.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (ipv4RoutingHelper, 100);
m_protocolName = "AddHostRouteTo Ipv4StaticRouting";
break;
case 7:
if (m_routingTables != 0)// setup is diff
{
GlobalRouting.PrintRoutingTableAllAt (rtt, rtw);
}
list.Add (GlobalRouting, 100);
m_protocolName = "Ipv4GlobalRouting";
break;
default:
NS_FATAL_ERROR ("No such protocol:" << m_protocol);
break;
}

// m_protocol为5、6、7的IP层安装方式有所不同
if (m_protocol < 5)
{
internet.SetRoutingHelper (list);
internet.Install (c);
}
else if (m_protocol == 5)
{
internet.Install (c);
dsrMain.Install (dsr, c);
}
else if (m_protocol == 6)
{
internet.Install (c);
}
else if (m_protocol == 7)
{
internet.Install (c);
GlobalRouting.PopulateRoutingTables ();
}
NS_LOG_UNCOND ("Routing Setup for " << m_protocolName<<"(if without using set delay transmission mode) ");
}

// 定义AssignIpAddresses函数 给所有节点设备的接口分配IP地址
void
ExternallyTest::AssignIpAddresses (NetDeviceContainer & d,
Ipv4InterfaceContainer & adhocTxInterfaces,Ipv6InterfaceContainer & adhocTxInterfaces6)
{
NS_LOG_INFO ("Assigning IP addresses");
if((m_80211_ltemode == 1)||(m_80211_ltemode == 2)||(m_80211_ltemode == 3))
{
Ipv4AddressHelper address;
// 我们可能有很多节点，并希望它们都在同一个子网中，以支持广播
address.SetBase ("6.6.6.0", "255.255.255.0");
adhocTxInterfaces = address.Assign (d);
}
else if(m_80211_ltemode == 4)
{
adhocTxInterfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (d));
}
else if(m_80211_ltemode == 5)
{
	  Ipv6AddressHelper ipv6;
	  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
	  adhocTxInterfaces6 = ipv6.Assign (devices);
}
}

// 定义安装internet协议栈函数
void ExternallyTest::InstallInternetStack()
{
NS_LOG_INFO(" Installing Internet stack on all nodes.");
SetupRoutingProtocol (nodes);
AssignIpAddresses (devices,interfaces,interfaces6);
// 这里m_protocol为6的路由（手动设置路由）安装比较特殊，要在分配完各个设备IP地址后进行
if(m_protocol == 6)
{
Ptr<Ipv4> ipv4Node;
Ptr<Ipv4StaticRouting> staticRoutingNode;
int nod = 0;
while (nod < m_nodes)
{
ipv4Node = nodes.Get(nod)->GetObject<Ipv4> ();
staticRoutingNode = ipv4RoutingHelper.GetStaticRouting(ipv4Node);
int leftNodes = nod - 1;
int leftExit = leftNodes;
while (leftNodes >= 0)
{
staticRoutingNode->AddHostRouteTo(interfaces.GetAddress(leftNodes),
interfaces.GetAddress(leftExit), 1);
leftNodes--;
}
int rightNodes = nod + 1;
int rightExit = rightNodes;
while (rightNodes <= m_nodes - 1)
{
staticRoutingNode->AddHostRouteTo(interfaces.GetAddress(rightNodes),
interfaces.GetAddress(rightExit), 1);
rightNodes++;
}
nod++;
}
}
}

// 定义SeedManager函数 设置随机数种子
void ExternallyTest::SeedManager()
{
NS_LOG_INFO(
" SeedManager using the seed and run.");
SeedManager::SetSeed (m_seed);
SeedManager::SetRun (m_run);
}

// 定义Configure_phyMode_Defaults函数 配置物理层传输模式和一些默认设置
void
ExternallyTest::Configure_phyMode_Defaults ()
{
NS_LOG_INFO("switch the phyMode and ConfigureDefaults of different wifi mode.");
if(m_80211_ltemode == 1)
{
switch (m_phyMode)
{
case 1:
m_phyModeName = "DsssRate1Mbps";
break;
case 2:
m_phyModeName = "DsssRate2Mbps";
break;
case 3:
m_phyModeName = "DsssRate5_5Mbps";
break;
case 4:
m_phyModeName = "DsssRate11Mbps";
break;
default:
NS_FATAL_ERROR ("No such protocol for 802.11b:" << m_phyMode);
break;
}
Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
StringValue (m_phyModeName));
}
else if(m_80211_ltemode == 2)
{
switch (m_phy_pMode)
{
case 1:
m_phyMode_pName = "OfdmRate6Mbps";
break;
case 2:
m_phyMode_pName = "OfdmRate9Mbps";
break;
case 3:
m_phyMode_pName = "OfdmRate12Mbps";
break;
case 4:
m_phyMode_pName = "OfdmRate18Mbps";
break;
case 5:
m_phyMode_pName = "OfdmRate24Mbps";
break;
case 6:
m_phyMode_pName = "OfdmRate36Mbps";
break;
case 7:
m_phyMode_pName = "OfdmRate48Mbps";
break;
case 8:
m_phyMode_pName = "OfdmRate54Mbps";
break;
case 9:
m_phyMode_pName = "OfdmRate3MbpsBW10MHz";
break;
case 10:
m_phyMode_pName = "OfdmRate4_5MbpsBW10MHz";
break;
case 11:
m_phyMode_pName = "OfdmRate6MbpsBW10MHz";
break;
case 12:
m_phyMode_pName = "OfdmRate9MbpsBW10MHz";
break;
case 13:
m_phyMode_pName = "OfdmRate12MbpsBW10MHz";
break;
case 14:
m_phyMode_pName = "OfdmRate18MbpsBW10MHz";
break;
case 15:
m_phyMode_pName = "OfdmRate24MbpsBW10MHz";
break;
case 16:
m_phyMode_pName = "OfdmRate27MbpsBW10MHz";
break;
default:
NS_FATAL_ERROR ("No such protocol for 802.11p:" << m_phy_pMode);
break;
}
Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
StringValue (m_phyMode_pName));
}
else if(m_80211_ltemode == 3)
{
switch (m_phy_WAVEMode)
{
case 1:
m_phyMode_WAVEName = "OfdmRate3MbpsBW10MHz";
break;
case 2:
m_phyMode_WAVEName = "OfdmRate4_5MbpsBW10MHz";
break;
case 3:
m_phyMode_WAVEName = "OfdmRate6MbpsBW10MHz";
break;
case 4:
m_phyMode_WAVEName = "OfdmRate9MbpsBW10MHz";
break;
case 5:
m_phyMode_WAVEName = "OfdmRate12MbpsBW10MHz";
break;
case 6:
m_phyMode_WAVEName = "OfdmRate18MbpsBW10MHz";
break;
case 7:
m_phyMode_WAVEName = "OfdmRate24MbpsBW10MHz";
break;
case 8:
m_phyMode_WAVEName = "OfdmRate27MbpsBW10MHz";
break;
default:
NS_FATAL_ERROR ("No such protocol for WAVE:" << m_phy_WAVEMode);
break;
}
Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
StringValue (m_phyMode_WAVEName));
}
Config::SetDefault("ns3::ArpCache::PendingQueueSize",UintegerValue(100));
Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
StringValue ("2200"));
// 2200字节一下禁用帧分段
Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
StringValue ("2200"));
if(m_80211_ltemode == 4)
{
Config::SetDefault ("ns3::LteUeMac::SlGrantMcs", UintegerValue (16));
Config::SetDefault ("ns3::LteUeMac::SlGrantSize", UintegerValue (5)); //The number of RBs allocated per UE for Sidelink
Config::SetDefault ("ns3::LteUeMac::Ktrp", UintegerValue (1));
Config::SetDefault ("ns3::LteUeMac::UseSetTrp", BooleanValue (true)); //use default Trp index of 0
// Set error models
Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));
}
}

void
ExternallyTest::SetupWaveMessages ()
{
// WAVE PHY mode
// 0=continuous channel; 1=channel-switching
int chAccessMode = 0;
if (m_80211_ltemode == 3)
{
chAccessMode = 1;


uint32_t m_wavePacketSize = 200;
double m_waveInterval = 0.1;
double m_gpsAccuracyNs = 40;
std::vector <double> m_txSafetyRanges;
m_txSafetyRanges.resize (10, 0);
m_txSafetyRanges[0] = 50.0;
m_txSafetyRanges[1] = 100.0;
m_txSafetyRanges[2] = 150.0;
m_txSafetyRanges[3] = 200.0;
m_txSafetyRanges[4] = 250.0;
m_txSafetyRanges[5] = 300.0;
m_txSafetyRanges[6] = 350.0;
m_txSafetyRanges[7] = 400.0;
m_txSafetyRanges[8] = 450.0;
m_txSafetyRanges[9] = 500.0;
double m_txMaxDelayMs = 10;
m_waveBsmHelper.Install (interfaces,
Seconds (m_totalTime),
m_wavePacketSize,
Seconds (m_waveInterval),
// GPS accuracy (i.e, clock drift), in number of ns
m_gpsAccuracyNs,
m_txSafetyRanges,
chAccessMode,
// tx max delay before transmit, in ms
MilliSeconds (m_txMaxDelayMs));

// fix random number streams
m_streamIndex += m_waveBsmHelper.AssignStreams (nodes, m_streamIndex);
}
}

void ExternallyTest::SetLtePacTracing( const ApplicationContainer &clientA ,const ApplicationContainer &serverA)
{
	if (m_asciiTrace != 0)
	{
	AsciiTraceHelper ascii;
	Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ( (m_trName + ".tr").c_str ());

	//Trace file table header
	*stream->GetStream () << "time(sec)\ttx/rx\tNodeID\tIMSI\tPktSize(bytes)\tIP[src]\tIP[dst]" << std::endl;

	std::ostringstream oss;

	// Set Tx traces
	for (uint16_t ac = 0; ac < clientA.GetN (); ac++)
	  {
		int32_t interface =  clientA.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetInterfaceForDevice (clientA.Get (ac)->GetNode ()->GetDevice (0));
	    Ipv4Address localAddrs =  clientA.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (interface,0).GetLocal ();
	    std::cout << "Tx address: " << localAddrs << std::endl;
	    oss << "tx\t" << clientA.Get (ac)->GetNode ()->GetId () << "\t" << clientA.Get (ac)->GetNode ()->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
	    clientA.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
	    oss.str ("");
	  }

	// Set Rx traces
	for (uint16_t as = 0; as < serverA.GetN (); as++)
	  {
		int32_t interface =  serverA.Get (as)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetInterfaceForDevice (clientA.Get (as)->GetNode ()->GetDevice (0));
	    Ipv4Address localAddrs =  serverA.Get (as)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (interface,0).GetLocal ();
	    std::cout << "Rx address: " << localAddrs << std::endl;
	    oss << "rx\t" << serverA.Get (as)->GetNode ()->GetId () << "\t" << serverA.Get (as)->GetNode ()->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
	    serverA.Get (as)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
	    oss.str ("");
	  }
	}
}

// 定义安装应用层函数（包括节点的客户端和服务器应用）
void ExternallyTest::InstallApplication()
{
NS_LOG_INFO("Create applications.");
// 一个全局变量开关，为所有协议启用所有校验和
GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

// 根据m_trans_protocol_mode判断使用的传输层协议，分别使用不同的助手类生成不同的2种客户端和服务器
if(m_trans_protocol_mode == 1)
{
//UdpEchoClientNewHelper echoClient(interfaces.GetAddress(m_server), m_servPort);
// Address ()设为空，不需要指定远程服务器的的地址，这在UdpEchoClientNew类内部设置
UdpEchoClientNewHelper echoClient(Address (), m_servPort);
// PacketSize也设为空，这在UdpEchoClientNew类内部判断并设置要传送数据包的大小
echoClient.SetAttribute("PacketSize", UintegerValue());
// 为每个节点安装客户端
clientApps = echoClient.Install(nodes.Get(0));

for (int i = 1; i < m_nodes; i++)
{
Ptr< Application > node_client_ptr = (echoClient.Install(nodes.Get(i))).Get(0);
clientApps.Add(node_client_ptr);
}
// 获得并输出客户端的数量
uint32_t clnum=clientApps.GetN();
NS_LOG_UNCOND(" Client number is "<<clnum);
// 设置客户端应用开启和停止的时间：为整个仿真时间
clientApps.Start(Seconds(0.));
clientApps.Stop(Seconds(m_totalTime));
// 服务器的安装过程类似与客户端
UdpEchoServerNewHelper echoServer(m_servPort);
serverApps = echoServer.Install(nodes.Get(0));

for (int i = 1; i < m_nodes; i++)
{
Ptr< Application > node_server_ptr = (echoServer.Install(nodes.Get(i))).Get(0);
serverApps.Add(node_server_ptr);
}

uint32_t senum=serverApps.GetN();
NS_LOG_UNCOND(" Server number is "<<senum);
serverApps.Start(Seconds(0.));
serverApps.Stop(Seconds(m_totalTime));
}
else if(m_trans_protocol_mode == 2)
{
TcpClientHelper echoClient(Address (),m_servPort);
clientApps = echoClient.Install(nodes.Get(0));

for (int i = 1; i < m_nodes; i++)
{
Ptr< Application > node_client_ptr = (echoClient.Install(nodes.Get(i))).Get(0);
clientApps.Add(node_client_ptr);
}

uint32_t clnum=clientApps.GetN();
NS_LOG_UNCOND(" Client number is "<<clnum);
clientApps.Start(Seconds(0.));
clientApps.Stop(Seconds(m_totalTime));

TcpServerHelper echoServer(m_servPort);
serverApps = echoServer.Install(nodes.Get(0));

for (int i = 1; i < m_nodes; i++)
{
Ptr< Application > node_server_ptr = (echoServer.Install(nodes.Get(i))).Get(0);
serverApps.Add(node_server_ptr);
}

uint32_t senum=serverApps.GetN();
NS_LOG_UNCOND(" Server number is "<<senum);
serverApps.Start(Seconds(0.));
serverApps.Stop(Seconds(m_totalTime));
}

if(m_80211_ltemode == 4)
{
//Set Sidelink bearers
uint32_t groupL2Address = 255;
Time slBearersActivationTime = Seconds (0);
Ipv4Address groupAddress[m_nodes];
Ptr<LteSlTft> tft[m_nodes];
for(int i = 0;i < m_nodes;i++)
{
std::string des_adress = "7.0.0.";
des_adress += std::to_string(m_nodes+1-i);
groupAddress[i].Set(des_adress.c_str());
tft[i] = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress[i], groupL2Address);
proseHelper->ActivateSidelinkBearer (slBearersActivationTime, NetDeviceContainer(devices.Get(i)), tft[i]);
}
SetLtePacTracing(clientApps,serverApps);
}
Set_delayloss();
}

void ExternallyTest::Set_delayloss ()
{
NS_LOG_INFO("run simulation with set delay and loss.");
	if(m_trans_protocol_mode == 1)
	{
	UdpEchoClientNew::GetSerContainerp(&serverApps);
	UdpEchoServerNew::GetCliContainerp(&clientApps);
	UdpEchoClientNew::Getdelayswitch(m_delayswitch);
	UdpEchoClientNew::Getdelay(m_Delay);
	UdpEchoClientNew::Getlossrate(m_Lossrate);
	}
	else if(m_trans_protocol_mode == 2)
	{
	TcpClient::GetSerContainerp(&serverApps);
	TcpServer::GetCliContainerp(&clientApps);
	TcpClient::Getdelayswitch(m_delayswitch);
	TcpClient::Getdelay(m_Delay);
	TcpClient::Getlossrate(m_Lossrate);
	}
}

// 定义Runsimulation函数 配置网络节点动画，流量文件生成，以及开始仿真运行
void
ExternallyTest::Runsimulation ()
{
NS_LOG_INFO ("Run simulation.");
// 设置网络节点动画，并开启一些动画的跟踪
std::string animFile = "co-simulation-animation.xml" ;
AnimationInterface anim (animFile);
anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (m_totalTime));
anim.EnableIpv4RouteTracking ("routingtable-wireless-co-simulation.xml",
Seconds (0), Seconds (m_totalTime), Seconds (0.25));
//anim.EnablePacketMetadata ();// 开启会导致运行出错，可能是ns3的问题
anim.EnableQueueCounters (Seconds (0), Seconds (m_totalTime));
anim.EnableWifiMacCounters (Seconds (0), Seconds (m_totalTime));
anim.EnableWifiPhyCounters (Seconds (0), Seconds (m_totalTime));
Time MobilityPollInterval = Seconds(0.20);
anim.SetMobilityPollInterval(MobilityPollInterval);
NS_LOG_UNCOND(
"Animation Trace file created: " << animFile.c_str ()
<<" and routingtable-wireless-co-simulation.xml");
// 安装流量监控
FlowMonitorHelper flowmonHelper;
Ptr<FlowMonitor> flowmon = flowmonHelper.InstallAll ();
std::string tr_name = "co-simulation-flowmon" ;
// Simulator::Stop运行GetImpl ()->Stop (delay)，这里GetImpl ()为ExternallyDrivenSim
// 所以运行的是ExternallyDrivenSim::Stop函数来计划仿真停止事件
Simulator::Stop(Seconds(m_totalTime));
Simulator::Run ();
// SerializeToXmlFile该函数必须在运行开启后调用，且流量监控文件必须在仿真停止后才会输入相应内容
// 杀死进程没有相应内容
flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), true, true);
NS_LOG_UNCOND("flowmon Trace file created: " << tr_name.c_str ());
Simulator::Destroy ();
}

// 定义运行ExternallyTest函数
int ExternallyTest::Run()
{
SeedManager();
Configure_phyMode_Defaults ();
CreateNodesphymacmobility();// 创建节点等
Set_ExternallyDrivenSim_config();
InstallInternetStack();// 创建internet栈
InstallApplication();// 安装应用层
SetupWaveMessages ();
ConfigureLogTracing ();
Runsimulation ();
return 0;
}

// 联合仿真ns3部分的主程序 ns3仿真程序的入口
int main(int argc, char *argv[])
{

ExternallyTest Et;
Et.Configure(argc, argv);
int Run = Et.Run();
return Run;

}
