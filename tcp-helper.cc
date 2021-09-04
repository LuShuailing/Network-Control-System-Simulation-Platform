/*
 * 文件: tcp-helper.cc
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的TCP应用程序的安装助手。源文件
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

#include "src/applications/helper/tcp-helper.h"
#include "src/applications/model/tcp-client.h"
#include "src/applications/model/tcp-server.h"

namespace ns3
{
// 定义TcpServerHelper类的含参数构造函数，参数 port为服务器将等待传入数据包的端口
TcpServerHelper::TcpServerHelper(uint16_t port)
{
m_factory.SetTypeId (TcpServer::GetTypeId ());
SetAttribute ("Port", UintegerValue(port));
}

// 函数定义 通过该函数传递使用构造函数的输入参数给TcpServer类的相应属性
void
TcpServerHelper::SetAttribute (
std::string name,
const AttributeValue &value)
{
m_factory.Set (name, value);
}

// 函数定义 该函数用来在节点上安装TcpServer应用，并返回一个应用容器指向TcpServer类应用程序对象
ApplicationContainer
TcpServerHelper::Install (Ptr<Node> node) const
{
return ApplicationContainer (InstallPriv (node));
}

// 在节点上安装ns3::TcpServer，应用程序通过使用SetAttribute设置的所有属性
Ptr<Application>
TcpServerHelper::InstallPriv (Ptr<Node> node) const
{
Ptr<Application> app = m_factory.Create<TcpServer> ();
node->AddApplication (app);
return app;
}

// 定义TcpClientHelper类的含参数构造函数，参数 port为服务器将等待传入数据包的端口
TcpClientHelper::TcpClientHelper (Address address,uint16_t port)
{
m_factory.SetTypeId (TcpClient::GetTypeId ());
SetAttribute ("RemoteAddress", AddressValue (address));
SetAttribute ("RemotePort", UintegerValue (port));
}

// 函数定义 通过该函数传递使用构造函数的输入参数给TcpClient类的相应属性
void
TcpClientHelper::SetAttribute (
std::string name,
const AttributeValue &value)
{
m_factory.Set (name, value);
}

// 函数定义 给定指向TcpClient应用程序的指针，将数据包的数据填充（作为数据发送到服务器的内容）
// 设置为填充字符串的内容（包括尾随零终止符）
void
TcpClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
app->GetObject<TcpClient>()->SetFill (fill);
}

// 函数定义 该函数用来在节点上安装TcpClient应用，并返回一个应用容器指向TcpClient类应用程序对象
ApplicationContainer
TcpClientHelper::Install (Ptr<Node> node) const
{
return ApplicationContainer (InstallPriv (node));
}

// 在节点上安装ns3::TcpClient，应用程序通过使用SetAttribute设置的所有属性
Ptr<Application>
TcpClientHelper::InstallPriv (Ptr<Node> node) const
{
Ptr<Application> app = m_factory.Create<TcpClient> ();
node->AddApplication (app);
return app;
}
}
