/*
 * 文件: tcp-helper.h
 *
 * 版本: 2.0
 *
 * 描述：用于与外部仿真器配套的TCP应用程序的安装助手。头文件
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

#ifndef TCP_HELPER_H
#define TCP_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/uinteger.h"

namespace ns3 {
/**
 * 定义类TcpServerHelper，该类用于创建一个等待输入TCP数据包的服务器应用程序。
 */
class TcpServerHelper
{
public:
// 声明TcpServerHelper类的含参数构造函数，该类用于设定将要创建的TcpServer应用程序
// 对象所应包含的基本信息，参数port为服务器将等待传入数据包的端口
TcpServerHelper (uint16_t port);
// 函数声明 通过该函数传递使用构造函数的输入参数给TcpServer类的相应属性
void SetAttribute (std::string name, const AttributeValue &value);
// 函数声明 该函数用来在节点上安装TcpServer应用，并返回一个应用容器指向TcpServer类应用程序对象
ApplicationContainer Install (Ptr<Node> node) const;

private:
/**
 * 在节点上安装ns3::TcpServer，应用程序通过使用SetAttribute设置的所有属性
 *
 * \参数 node 将安装TcpServer的节点。
 * \返回 指向被安装应用程序的智能指针
 */
Ptr<Application> InstallPriv (Ptr<Node> node) const;
// 对象工厂，用于在构造函数中通过包含各种属性来构造TcpServer对象
ObjectFactory m_factory;
};

/**
 * 定义类TcpClientHelper，该类用于创建一个发送TCP数据包的客户端应用程序。
 */
class TcpClientHelper
{
public:
// 声明TcpClientHelper类的含参数构造函数，该类用于设定将要创建的TcpClient应用程序
// 对象所应包含的基本信息，参数 port为服务器将等待传入数据包的端口
TcpClientHelper (Address address,uint16_t port);
// 函数声明 通过该函数传递使用构造函数的输入参数给TcpClient类的相应属性
void SetAttribute (std::string name, const AttributeValue &value);
// 函数声明 给定指向TcpClient应用程序的指针，将数据包的数据填充（作为数据发送到服务器的内容）
// 设置为填充字符串的内容（包括尾随零终止符）。这意味着PacketSize属性（指的是m_dataSize）
// 可能会因此调用而更改。
void SetFill (Ptr<Application> app, std::string fill);
// 函数声明 该函数用来在节点上安装TcpClient应用，并返回一个应用容器指向TcpClient类应用程序对象
ApplicationContainer Install (Ptr<Node> node) const;

private:
/**
 * 在节点上安装ns3::TcpClient，应用程序通过使用SetAttribute设置的所有属性
 *
 * \参数 node 将安装TcpClient的节点。
 * \返回 指向被安装应用程序的智能指针
 */
Ptr<Application> InstallPriv (Ptr<Node> node) const;
// 对象工厂，用于在构造函数中通过包含各种属性来构造TcpClient对象
ObjectFactory m_factory;
};

} // namespace ns3

#endif /* TCP_HELPER_H */
