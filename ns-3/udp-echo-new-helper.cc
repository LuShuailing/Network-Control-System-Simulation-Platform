/*
 * 文件: udp-echo-new-helper.cc
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的UDP应用程序的安装助手。源文件
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
#include "ns3/udp-echo-helper.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "src/applications/helper/udp-echo-new-helper.h"
#include "src/applications/model/udp-echo-server-new.h"
#include "src/applications/model/udp-echo-client-new.h"

namespace ns3
{
// 函数定义 定义UdpEchoServerNewHelper类的含参数构造函数，这里要注意子类（派生类）在继承特性中需要
// 自己的构造函数，而子类不能直接访问父类的私有成员，而必须通过子类的方法进行访问，具体地说，子类构造函数
// 必须使用基类构造函数，创建子类对象时，程序首先创建父类的对象，从概念上说，这意味着父类对象应当在程序进
// 入子类构造函数之前被创建，c++使用成员初始化列表语法来完成这种工作。反映到该构造函数，其中
// UdpEchoServerHelper(port)是成员初始化列表。他是可执行的代码调用UdpEchoServerNewHelper
// 构造函数之前将会首先调用UdpEchoServerHelper构造函数，假设程序调用该构造函数如下，
// UdpEchoServerNewHelper echoServer(m_servPort);那么UdpEchoServerNewHelper构造函数将会把
// 实参m_servPort赋给形参port，然后将m_servPort作为实参传递给UdpEchoServerHelper构造函数，
// 后者将创建一个嵌套的UdpEchoServerHelper对象，并将数据m_servPort储存在该对象中，然后程序进入
// UdpEchoServerNewHelper构造函数体，完成echoServer对象的创建。

// 如果省略成员初始化列表，构造函数直接为UdpEchoServerNewHelper::UdpEchoServerNewHelper(uint16_t port)
// 情况会如何呢？我们在创建子类对象时首先要创建父类对象，如果不调用父类构造函数，程序将会使用默认的父类构造函数，
// 这时上述代码等价与UdpEchoServerNewHelper::UdpEchoServerNewHelper(uint16_t port):UdpEchoServerHelper()，
// 然而对于类UdpEchoServerHelper我们定义了带参的构造函数，如果没有定义任何构造函数，编译器才会提供
// 默认的构造函数，如果已经定义了构造函数，那么就必须为该类提供显式的默认构造函数，我们知道
// UdpEchoServerHelper类是没有定义显式的默认构造函数的，那么由于找不到默认构造函数，那么该定义将
// 会出错： error: no matching function for call to ‘ns3::UdpEchoServerHelper::UdpEchoServerHelper()’
// 即找不到合适的默认构造函数，所以除非要使用默认构造函数，否则应显式调用正确的父类构造函数。
UdpEchoServerNewHelper::UdpEchoServerNewHelper(uint16_t port) : UdpEchoServerHelper(port)
{
// 通过m_factory调用SetTypeId函数包含各种属性来构造UdpEchoServerNew对象
m_factory.SetTypeId (UdpEchoServerNew::GetTypeId ());
// 传递使用构造函数的输入参数给UdpEchoServerNew类的相应属性Port
SetAttribute ("Port", UintegerValue(port));
}

// 定义UdpEchoServerNewHelper类的析构函数
UdpEchoServerNewHelper::~UdpEchoServerNewHelper()
{
}

// 函数定义 通过该函数传递使用构造函数的输入参数给UdpEchoServerNew类的相应属性
void
UdpEchoServerNewHelper::SetAttribute (
std::string name,
const AttributeValue &value)
{
m_factory.Set (name, value);
}

// 函数定义 该函数通过调用私有函数InstallPriv，用来在节点上安装UdpEchoServerNew应用，
// 并返回一个应用容器指向UdpEchoServerNew类应用程序对象
ApplicationContainer
UdpEchoServerNewHelper::Install (Ptr<Node> node) const
{
return ApplicationContainer (InstallPriv (node));
}

// 函数定义 在节点上安装ns3::UdpEchoServerNew，应用程序通过使用SetAttribute设置的所有属性
Ptr<Application> UdpEchoServerNewHelper::InstallPriv (Ptr<Node> node) const
{
Ptr<Application> app = m_factory.Create<UdpEchoServerNew> ();
node->AddApplication (app);
return app;
}

// 函数定义 定义UdpEchoClientNewHelper类的含参数构造函数
UdpEchoClientNewHelper::UdpEchoClientNewHelper (Address address, uint16_t port) : UdpEchoClientHelper( address,  port)
{
m_factory.SetTypeId (UdpEchoClientNew::GetTypeId ());
SetAttribute ("RemoteAddress", AddressValue (address));
SetAttribute ("RemotePort", UintegerValue (port));
}

// 函数定义 UdpEchoClientNewHelper的设置UdpEchoClientNew属性的函数
void
UdpEchoClientNewHelper::SetAttribute (
std::string name,
const AttributeValue &value)
{
m_factory.Set (name, value);
}

// 函数定义 给定指向UdpEchoClientNew应用程序的指针，将数据包的数据填充（作为数据发送到服务器的内容）
// 设置为填充字符串的内容（包括尾随零终止符）。这意味着PacketSize属性（指的是m_dataSize）
// 可能会因此调用而更改。
void
UdpEchoClientNewHelper::SetFill (Ptr<Application> app, std::string fill)
{
// 可以看出该填充函数的实现是通过调用UdpEchoClientNew应用程序对象继承自UdpEchoClient的公有函数
// SetFill来实现的
app->GetObject<UdpEchoClientNew>()->SetFill (fill);
}

// 定义SetFill的重构函数
void
UdpEchoClientNewHelper::SetFill (Ptr<Application> app,
uint8_t fill, uint32_t dataLength)
{
app->GetObject<UdpEchoClientNew>()->SetFill (fill, dataLength);
}

// 定义SetFill的另一种重构函数
void
UdpEchoClientNewHelper::SetFill (Ptr<Application> app, uint8_t *fill,
uint32_t fillLength, uint32_t dataLength)
{
app->GetObject<UdpEchoClientNew>()->SetFill (fill, fillLength, dataLength);
}

// 函数定义 该函数通过调用私有函数InstallPriv，用来在节点上安装UdpEchoClientNew应用，
// 并返回一个应用容器指向UdpEchoClientNew类应用程序对象
ApplicationContainer
UdpEchoClientNewHelper::Install (Ptr<Node> node) const
{
return ApplicationContainer (InstallPriv (node));
}

// 函数定义 在节点上安装ns3::UdpEchoClientNew，应用程序通过使用SetAttribute设置的所有属性
Ptr<Application>
UdpEchoClientNewHelper::InstallPriv (Ptr<Node> node) const
{
Ptr<Application> app = m_factory.Create<UdpEchoClientNew> ();
node->AddApplication (app);
return app;
}

// 定义UdpEchoClientNewHelper类的析构函数
UdpEchoClientNewHelper::~UdpEchoClientNewHelper()
{
}
}
