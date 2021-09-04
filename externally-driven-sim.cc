/*
 * 文件: externally-driven-sim.cc
 *
 * 版本: 2.0
 *
 * 描述：通过tcp套接字外部驱动的仿真器应用。源文件
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

#include "ns3/simulator.h"
#include "ns3/default-simulator-impl.h"
#include "ns3/scheduler.h"
#include "ns3/event-impl.h"
#include "ns3/global-value.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/nstime.h"
#include "ns3/ipv4.h"
#include "src/core/model/externally-driven-sim.h"
#include "src/applications/model/udp-echo-client-new.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/ptr.h"
#include "ns3/pointer.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>
#include <math.h>
#include <src/core/model/struct.h>
#include <unistd.h>

namespace ns3// 使用并把以下部分纳入ns3命名空间
{
// 定义日志组件ExternallyDrivenSim并使用TypeId系统注册一个Object子类ExternallyDrivenSim
NS_LOG_COMPONENT_DEFINE ("ExternallyDrivenSim");

NS_OBJECT_ENSURE_REGISTERED ( ExternallyDrivenSim);

// 一些需要在其他类使用的属性的重定义 都为静态成员数据(属性).静态成员数据在类里只是一个说明，
// 还需要一个定义（或叫初始化）。静态成员数据要在类定义之外被初始化（要用类名限定修饰），
// 而且程序里只能提供一次初始化，所以初始化不能放在头文件里。
uint64_t ExternallyDrivenSim::m_TimeLimit;
bool ExternallyDrivenSim::in_sam_trans;
Time ExternallyDrivenSim::g_SimulationStep;
int ExternallyDrivenSim::m_nodenumber;
uint16_t ExternallyDrivenSim::g_qudong_mode;
uint16_t ExternallyDrivenSim::g_trans_protocol_mode;
// 定义并初始化静态持续内部链接变量（静态全局（外部）变量）用于统计事件发生的次数（执行事件计数器），代码块不执行也留在内存中。
static int g_EventCount = 0;
// 定义（new）一个静态持续内部链接对象（该程序的静态全局变量）time_determine，用来缓存m_TimeLimit的值
// 以此来选择发送给外部应用程序的通知的写入方式
static uint64_t *time_determine = (uint64_t*)malloc(100*sizeof(uint64_t));
// 定义该程序的静态全局变量，当前时间变量,即当前仿真时间
static uint64_t g_currentTs;
// 定义该程序的静态全局变量，智能指针变量g_EventImpl（事件实现的智能指针）
static Ptr<EventImpl> g_EventImpl;
// 定义该程序的静态全局变量g_ts保存事件的时间
static uint64_t g_ts;
// 定义该程序的静态全局变量g_context 事件的上下文
static uint32_t g_context;
// 定义该程序的静态全局变量g_uid 事件的id
static uint32_t g_uid;
// 定义该程序的静态全局变量Data类发送结构体trans数组,可以包含100个节点的信息
static struct Data_NS3 trans[100];
// 定义该程序的静态全局变量k_des用于在该程序中保存要发送的控制系统目标节点数量
static int k_des = 0;
// 定义并初始化该程序的静态全局变量get_des为0，用于在该程序中保存要所有要发送的控制系统目标节点ID
static int get_des[100] = {0};

/*
 * 定义ExternallyDrivenSim::GetTypeId成员函数，返回TypeId类型。
 * 接口的唯一标识符。
 * 这个类记录了很多关于Object基类的子类的元信息：
 * - 子类的基类
 * - 子类中的一组可访问构造函数
 * - 子类中可访问的一组“属性”
 */
TypeId ExternallyDrivenSim::GetTypeId(void)
{
static TypeId tid = TypeId("ns3::ExternallyDrivenSim")
.SetParent<DefaultSimulatorImpl>().
AddConstructor<ExternallyDrivenSim>();
// 初始化TypeId静态类型tid（作用域该函数），
// 函数SetParent在此TypeId中记录其TypeId是子类的基类的TypeId。
// 在此TypeId中记录默认可访问的构造函数。
return tid;
}

/*
 * 定义ExternallyDrivenSim的构造函数 用于并在函数内部开启套接字
 */
ExternallyDrivenSim::ExternallyDrivenSim()
{
m_TimeLimit = 0;// 把m_TimeLimit赋值为0
// 创建tcp原始套接字listener,注意这里我们将使用套接字的默认行为阻塞模式：即一直阻塞到请求动作完成为止
// 例如至少接收到一条来自客户端的消息，recv函数才会返回，当然，与此同时具有被阻塞函数的进程将会被
// 操作系统挂起（另一种非阻塞模式这里不使用）TCP/IP socket编程P100
listener = socket(AF_INET, SOCK_STREAM, 0);
/*
 * 当返回值小于0，输出错误消息socket，退出函数告诉系统程序已完成，导致程序终止进程。
 * 状态参数(status=1)是程序的退出状态，它成为进程终止状态的一部分。 该功能不返回。
 */
if (listener < 0)
{
perror("socket");
exit(1);
}
addr.sin_family = AF_INET;// 确定tcpip地址族
// 该函数将整型从主机字节顺序转换为网络字节顺序（网络的端口号要转变为实际网络可识别的顺序）
// 以此来定义端口号3425
addr.sin_port = htons(3425);
/*
 * 该函数将uint32_t整数主机从主机字节顺序转换为网络字节顺序。这用于IPv4 Internet地址定义。
 */
addr.sin_addr.s_addr = htonl(INADDR_ANY);
/*
 * bind函数为套接字分配一个本地地址。 addr和length参数指定地址; 地址的详细格式取决于命名空间。
 * 地址的第一部分始终是指定名称空间的格式指示符，并指出该地址采用该名称空间的格式。返回值在成功时为0，
 * 在失败时为-1。
 */
if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) < 0)
{
perror("bind");// 返回错误信息bind，表示bind失败
exit(2);
}
printf("NS3服务器开启，等待SCILAB连接...\n");// 打印“服务器加载”的提示
/*
 * listen函数使套接字能够接受连接，从而使其成为服务器套接字。参数n为未决连接指定队列的长度。
 * 当队列填满时，尝试连接的新客户端将以ECONNREFUSED失败，直到服务器调用接受函数接受来自队列的连接。
 * listen函数在成功时返回0，在失败时返回-1。
 */
listen(listener, 1);// 原始套接字开始侦听
/*
 * 该函数用于接受服务器上的套接字连接请求。linux平台上accept（）函数返回的socket是阻塞套接字
 * 因而如果没有挂起的连接，accept函数将等待，除非套接字已设置非阻塞模式。
 * （当使用非阻塞套接字时，您可以使用select来等待挂起的连接。）
 * 因而当运行ns3脚本程序调用仿真器对象时，如果没有接收到外部应用的客户端的通知时，
 * 服务器将在运行到此条语句时阻塞，终端将显示 server loaded 直到客户端发出通知建立子进程连接
 * addr和length-ptr参数用于返回有关发起连接的客户端套接字的名称的信息。
 * 接受连接不会使连接成为套接字的一部分。相反，它会创建一个连接的新套接字。
 * accept的正常返回值是新套接字的文件描述符。接受后，原始套接字套接字保持打开状态并且未连接，
 * 并继续收听，直至关闭它。 通过再次调用accept，可以接受与套接字的更多连接。如果发生错误，
 * 则接受返回-1。
 */
m_sock = accept(listener, NULL, NULL);
if (m_sock < 0)
{
perror("accept");// 返回错误信息accept失败，子连接进程连接失败
exit(3);
}
}

// 当仿真停止时，要运行析构函数销毁代码，要关闭连接新旧两个套接字 用定义DoDispose方法来实现析构函数
void ExternallyDrivenSim::DoDispose(void)
{
NS_LOG_FUNCTION (__func__<<" carried ");
close(m_sock);
close(listener);
}

ExternallyDrivenSim::~ExternallyDrivenSim()// 析构函数
{
}

/*
 * 定义外部驱动仿真类的接收侦听函数来接收外部应用程序的数据包。
 * 这里的服务器逻辑为一个迭代服务器，迭代服务器会依次处理客户端轮询的短连接，
 * 只要当前连接的任务没有完成，服务器的进程就会一直被占用，直到任务完成后，
 * 服务器关闭这个子socket连接，释放连接。
 *  它的原型可以描述成：
while(1)
{
new_fd = 服务器accept客户端的连接(new_fd = accept(listenfd, XX, XX))
逻辑处理
在这个new_fd上给客户端发送消息
关闭new_fd
}
 */
void ExternallyDrivenSim::Listen()
	{
	
	
	
		// 连接建立后等待客户端发送过来的任何数据包，在这之前进行的回显提示服务器在等待客户端的数据包
		// 这里其实接收的是客户端用来断开连接的空包，由于之前服务器发送了通知，
		// 客户端接收到了通知后要断开连接因而要发送用于断开连接的空数据包(除了第一次的交互以外都是这种情况）
		// 第一次交互时这里接收的matlab客户端发送的具有实际意义的数据包
		printf("等待SCILAB发送数据...\n");
		printf("Client:");
		
		
		
		        close(m_sock);
			listen(listener, 1);// 开始新一轮的侦听
			m_sock = accept(listener, NULL, NULL);// 创建新的套接字子进程
			if (m_sock < 0)
			{
				perror("accept");// 返回错误信息accept失败，子套接字未连接
				exit(3);
			}
			
		// 定义Data结构体rec数组即接收数据包，数组的大小用m_nodenumber设置
		struct Data_MATLAB rec[m_nodenumber];
		// 定义size_rbuf，为套接字接收缓冲区的大小并赋值为结构体rec的大小(保存外部应用的数据包）
		// 加上四字节的int(保存发过来的quit用于手动停止仿真）变量
		int size_rbuf = 0;
		size_rbuf = sizeof(rec)+sizeof(int);
		// 用size_rbuf来定义套接字接收缓冲区
		char recv_buf[size_rbuf];
		// 缓冲区接收前清0
		memset(recv_buf, 0, sizeof(recv_buf));
		// 通过m_sock套接字接收客户端的数据包并读取解析并返回和显示接收的外部应用的数据包的大小
		/* information analyse functions解析数据包函数，recv函数就像read，但带有附加的标志。
		* 套接字数据选项中描述了可能的标志值。这里是0，即阻塞模式。如果套接字设置了非阻塞模式，
		* 并且没有数据可供读取，则recv立即失败，而不是等待。此函数返回接收到的字节数，如果失败则返回-1。
		*/
		bytes_read = recv(m_sock, &recv_buf, sizeof(recv_buf), 0);
		NS_LOG_UNCOND("MATLAB send packet bytes_read is " << bytes_read);
		// 由于客户端每次发送完通知要断开连接，而每次断开连接后客户端会发送两个空包（FIN和ACK包）来断开连接
		// 因此如果发送过来的为空包，那么判断应该新套接字子进程应该断开，这时候需要运行close(m_sock)来关闭
		// 上一次的子进程连接，并让原始套接字继续开始新一轮的侦听，为下一次客户端发起的连接创建一个的新套接字
		/*while (bytes_read == 0)
		{
			
			//这里需要运行close(m_sock);第一，因为可分配的socket描述符是有限的，如果分配了以后不释放，
			//也就是不能回收再利用，也就是总有描述符耗尽的一天。第二，本来把和客户端连接的任务交给子进程
			//以后父进程将会继续监听并accept下个连接了，但如果子进程不关闭自己跟客户的连接，
			// 意思就是这个连接居然永远存在！

			close(m_sock);
			listen(listener, 1);// 开始新一轮的侦听
			m_sock = accept(listener, NULL, NULL);// 创建新的套接字子进程
			if (m_sock < 0)
			{
				perror("accept");// 返回错误信息accept失败，子套接字未连接
				exit(3);
			}
			// 这里其实接收的是客户端用来通知的实际的包，因为上一次的连接因发送断开连接的空数据包而关闭
			// 这里接收的是下一次客户端发起的新连接接收的通知数据包（除了第一次的交互以外都是这种情况）

			// 第一次交互的情况为：仿真开始运行，首先调用仿真运行实例函数ExternallyDrivenSim::Run(void)
			// 之后几个节点的应用程序将会开始运行，在ns3的第一个同步周期内，由于应用程序刚启动，所以在第一个同步
			// 周期内没有任何事件发生，在第二个周期才开始真正意义上的的具体仿真过程，第一个周期运行时，
			// 仍会运行TransmitNotices(); 对外发送刚初始化的trans结构体内的信息，这其实是一个没有任何意义
			// 的空数据包，对于这个数据包，在外部应用客户端处，有逻辑判断接收到的这个数据包其实意味着ns3在初始化
			// 各个应用程序，会输出为ns3在进行初始化的信息，并进行循环，重新运行接收语句来接收ns3端接收到外部
			// 客户端发送的第一个数据包，ns3的服务器接收到这个包然后触发各个ns3应用的第一次发送，ns3运行仿真事件
			// 后将运行TransmitNotices();这触发这一轮套接字交互中第二次向matlab客户端的仿真通知的发送，
			// 这次对外发送数据包的trans结构体内的信息， 是包含ns3各个应用程序发送数据包后网络仿真的实际仿真运行的，
			// 也就是说是matlab执行仿真需要的数据，matlab客户端接收到这个数据包后进行处理，不再进行循环而是关闭
			// 套接字发送关闭连接的FIN包（空包），直到下一次matlab需要发送数据时客户端启动新一轮套接字通讯，
			// ns3这边接收到空包后判断为0字节关闭子连接,因而第一轮交互时这里不接收matlab数据包，
			// 不管是断开连接的空包还是具有matlab仿真数据的数据包
			printf("Waiting for client..\n");// 表示等待客户端
			printf("Client:");
			memset(recv_buf, 0, sizeof(recv_buf));
			bytes_read = recv(m_sock, &recv_buf, sizeof(recv_buf), 0);
			NS_LOG_UNCOND("MATLAB send packet bytes_read is " << bytes_read);
		}
*/
		// 清空接收结构体和置0手动仿真停止标志，并拷贝接收缓存中的matlab仿真结果数据和手动停止标志
		memset(&rec, 0, sizeof(rec));
		int quit = 0;
		memcpy(&rec, recv_buf, sizeof(rec));
		// 因为我编译的系统为linux64位，其指针类型和long型大小相等（8B）而与不是32位系统的int型4B，
		// (int*)((int)recv_buf + sizeof(rec))会出现：‘char*’ to ‘int’ loses precision（损失精度）
		memcpy(&quit, (long*)((long)recv_buf + sizeof(rec)), sizeof(int));

		// 根据接收到的手动停止标志，设置ExternallyDrivenSim的私有属性，
		// 也是手动停止标志不过是ExternallyDrivenSim类的
		if (quit == 0)
		{
			m_quit = false;
		}
		else
		{
			m_quit = true;
		}

		// 这一部分是从matlab客户端接收到的数据的输出部分
		std::cout << "signal of transmission ";
		for (int i = 0; i<m_nodenumber - 1; i++)
		{
			std::cout << i << ":" << rec[i].sig << " and ";
		}
		std::cout << m_nodenumber - 1 << ":" << rec[m_nodenumber - 1].sig << "\n" << std::endl;

		std::cout << "destination of transmit ";
		for (int i = 0; i<m_nodenumber - 1; i++)
		{
			std::cout << i << ":" << rec[i].des << " and ";
		}
		std::cout << m_nodenumber - 1 << ":" << rec[m_nodenumber - 1].des << "\n" << std::endl;

		std::cout << "content of packet ";
		for (int i = 0; i<m_nodenumber - 1; i++)
		{
			std::cout << i << ":" << rec[i].packetcontent << " and ";
		}
		std::cout << m_nodenumber - 1 << ":" << rec[m_nodenumber - 1].packetcontent << "\n" << std::endl;

		std::cout << "the node is control system send node or not ";
		for (int i = 0; i<m_nodenumber - 1; i++)
		{
			std::cout << i << ":" << rec[i].sys_send_node << " and ";
		}
		std::cout << m_nodenumber - 1 << ":" << rec[m_nodenumber - 1].sys_send_node << "\n" << std::endl;

		/*
		* 创建一个NodeContainer，其中包含通过NodeContainer::Create（）创建的所有节点的列表,
		* 并把其储存在ns3::NodeList中无论何时创建节点，都会将Ptr<node>添加到系统中所有节点的全局列表中。
		* 能够在一个地方找到所有节点有时很有用。此方法创建一个初始化为包含所有模拟节点的容器
		* （实例化对象为m_nodes）
		*/
		m_nodes = NodeContainer::GetGlobal();
		std::cout << "the node update its' position or velocity by schedule or not : " << std::endl;
		for (int i = 0; i<m_nodenumber; i++)
		{
			std::cout << " node " << i << " pos_vel_send_sig: " << rec[i].pos_vel_send_sig << " pos: " << rec[i].pos
				<< " vel: " << rec[i].vel << std::endl;
			if (rec[i].pos_vel_send_sig)
			{
				NS_LOG_UNCOND("the node " << i << " will update its' position or velocity. ");
				m_nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetPosition(Vector(rec[i].pos, 0.0, 0.0));
				m_nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(rec[i].vel, 0.0, 0.0));
			}
			else
			{
				NS_LOG_UNCOND("the node " << i << " won't need to update its' position or velocity. ");
			}
		}
		NS_LOG_UNCOND("simulation quit determination is " << m_quit);
		// 如果在事件驱动模式下，那么将对各个节点进行判断，如果是系统发送的节点，将他们的控制系统目标节点的ID
		// 保存到get_des数组中去，并统计这些控制系统目标节点的数量保存到k_des
		if (g_qudong_mode == 2)
		{
			int i_des = 0;
			for (int i = 0; i < m_nodenumber; i++)
			{
				if (rec[i].sys_send_node == true)
				{
					get_des[i_des] = rec[i].des;
					i_des++;
				}
			}
			k_des = i_des;
		}
		/*
		* 根据结构体rec对所有节点的发送标志sig进行判断
		*/
		bool det_rec_sig = true;
		for (int i = 0; i < m_nodenumber; i++)
		{
			det_rec_sig = det_rec_sig && (rec[i].sig == 0);
		}
		/*
		* 如果rec结构体的sig都为空时执行，即所有个节点都没有接收到数据包，输出在xxx毫秒之内，
		* 没有计划新的事件。（即各个节点的数据包发送事件）
		*/
		if (det_rec_sig)
		{
			// 在时间驱动模式下，每次运行到这时，之前判断f_next.key.m_ts > m_TimeLimit为true，因而会运行
			// 该语句m_TimeLimit += (uint64_t) g_SimulationStep.GetNanoSeconds();
			// 该语句用来更新这次仿真交互中ns3的仿真时间上限为下下一次的同步事件时间点，由于det_rec_sig为true,
			// 则可以确定下一次同步事件时间点后没有被安排发送事件，而下下一次同步事件时间点的后节点发送情况未知，
			// 因此可以确定在下下一次（之后的第二个同步事件时间点)之前,是没有任何数据包发送事件被安排，因而
			// NanoSeconds(m_TimeLimit).GetMilliSeconds()-ns3::Simulator::Now().GetMilliSeconds()
			// 这段时间内将没有发送事件被安排，因此我们输出如下。
			if (g_qudong_mode == 1)
			{
				NS_LOG_UNCOND("within several " << NanoSeconds(m_TimeLimit).GetMilliSeconds()
					- ns3::Simulator::Now().GetMilliSeconds() << " milliseconds till (m_TimeLimit) "
					<< (double)m_TimeLimit / 1000000000
					<< " s. new events(packet transmission events) are not planned\n");
			}
			// 在事件驱动模式下，情况将会有一些不同，每次运行到这时，判断f_next.key.m_ts > m_TimeLimit为false，
			// 因而不会运行该语句m_TimeLimit += (uint64_t) g_SimulationStep.GetNanoSeconds();
			// 这次仿真交互中ns3的仿真时间上限为仍然为下一次的同步事件时间点，由于det_rec_sig为true,
			// 可以确定下一次异步事件时间点没有被安排发送事件（如果m_timeOfStep = 0，就为该时刻），而下下一次
			// 异步事件时间点的是被控对象还是控制器节点接收到数据包，以及该异步事件的发生时间,
			// 还是一直到下一次同步事件时间点之前各节点服务器都没有接收到数据包都是未知的，因此我们无法确定下一个
			// 可能的数据包的具体发送事件的发生时间，因此无法计算从现在这个仿真时间开始到下一次数据包发送的时间
			// 间隔。但是我们知道直到下一次同步事件时间点之前，如果控制器节点没有接收到数据包
			//（在事件驱动模式下，如果控制器接收到数据包，如果m_timeOfStep = 0，那么在该异步事件时间点
			// 将会进行数据包发送）,那么直到下一个同步事件时间点之前将不会有客户端数据包发送事件。因此我们输出如下。
			if (g_qudong_mode == 2)
			{
				NS_LOG_UNCOND("If controller node within several " << NanoSeconds(m_TimeLimit).
					GetMilliSeconds() - ns3::Simulator::Now().GetMilliSeconds() << " milliseconds till (m_TimeLimit) "
					<< (double)m_TimeLimit / 1000000000
					<< " s don't receive packet. During this time period new events(packet transmission events) are not planned\n");
			}
		}
		// 如果有节点的sig标志为1，设置相关ID等属性，计划这个节点的发送事件
		else
		{
			for (int i = 0; i < m_nodenumber; i++)// 逐个检查每个触发信号
			{
				if (rec[i].sig == 1)// 当某个触发信号不为空
				{
					NS_LOG_UNCOND("Node # " << i << " Sending \n");// 输出节点i要发送信息
					m_nodId = i;// 用i赋值要发送数据包的节点的ID
					m_nodeIdTo = rec[i].des;// rec[i].des定义被发送（接收）数据包的节点的ID
					m_PayLoad = rec[i].packetcontent;// rec[i].packetcontent定义节点客户端应用程序需要发送的内容
					m_timeOfStep = 0;// 客户端发送数据包以某一相同时间推迟一段时间(为0）
					// 此函数启动一个client类方法来安排发送新事件。但是不使用m_nodeIdTo来确定节点计划发送的对象
					//RunScheduleTransmit();
					/*
					* 如果您需要将数据包发送到特定节点，并且您已经生成了交换结构m_PayLoad，只需包含必要的信息，
					* 则只需注释“RunScheduleTransmit（）;”字符串，然后取消注释以下代码
					*/
					// 运行多包传输mpcwireless.mdl
					/*if (m_nodId == 0)//被控对象id = 0
					{
						std::vector<std::string> mutipac;
						std::string::iterator head = m_PayLoad.begin();
						std::string::iterator tail;
						for (tail = m_PayLoad.begin(); tail<m_PayLoad.end(); tail++)
						{
							if (*tail != ' ' && (*(tail + 1) == ' ' || *(tail + 1) == '\0'))
							{
								mutipac.push_back(std::string(head, tail + 1));
								head = tail + 1;
							}
						}
						uint64_t delay_send = 0;
						for (uint mutipac_index = 0; mutipac_index<mutipac.size(); mutipac_index++)
						{
							RunScheduleTransmitTo(mutipac[mutipac_index], mutipac_index, mutipac.size(), delay_send);
							delay_send = delay_send + 0;
						}
						std::vector<std::string>().swap(mutipac);
					}
					else
					{
						RunScheduleTransmitTo(m_PayLoad);
					}
					*/
					// 运行普通mdl
					RunScheduleTransmitTo(m_PayLoad);
				}
			}
		}
		std::cout << "\n";
	}

/*
 * 定义此函数，返回指向UdpEchoClientNew*客户端新应用程序对象的普通（非智能）指针，
 * 使用的是指向该节点的智能指针（适用于传输层协议为UDP时）
 */
UdpEchoClientNew* ExternallyDrivenSim::GetClient(Ptr<Node> n)
{
/*
 * 给applic（应用程序对象的指针）赋值，用的是该节点中与该请求索引0关联的应用程序的指针。
 * 即该节点的客户端应用程序，因为仿真脚本中在每个节点上安装应用程序的顺序为先安装了客户端，
 * 在安装的是服务器，所以0是指向客户端，而1是指向服务器
 */
m_applic = n->GetApplication(0);
// 从智能指针中提取普通指针，以便指针类型强制转换
Application* applic = PeekPointer(m_applic);
/*
 * 把applic的Application*指针转变为UdpEchoClientNew* 指针的强制指针类型的转换
 */
UdpEchoClientNew* pucl = (UdpEchoClientNew*) applic;

return pucl;// 返回UdpEchoClientNew指针pucl
}

/*
 * 定义此函数，返回指向TcpClient*客户端新应用程序对象的普通（非智能）指针，
 * 使用的是指向该节点的智能指针（具体情况同上，适用于传输层协议为TCP时）
 */
TcpClient* ExternallyDrivenSim::GetClient_tcp(Ptr<Node> n)
{
m_applic = n->GetApplication(0);
Application* applic = PeekPointer(m_applic);// 智能指针中提取普通指针
TcpClient* pucl = (TcpClient*) applic;
return pucl;
}


// 定义该函数返回目标节点的Ip地址，用的是指向该节点的智能指针
Ipv4Address ExternallyDrivenSim::GetDestIp(Ptr<Node> n)
{
/*
 * 指向所请求的Ipv4接口（类）(相当于IP层）的智能指针，或者如果找不到它，则返回零。
 * 派生类node找到基类Object的另一个派生类Ipv4的智能指针
 */
Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
// 每个节点上其实可以安装多个设备，在这个仿真中我们只在一个节点上安装了一个设备，与此
// 通过索引0得到节点的设备NetDevice（0）,也就是唯一一个节点上的设备的智能指针
Ptr<NetDevice> device = n->GetDevice(0);
// 返回该NetDevice的Ipv4的接口（类）编号，如果未找到，则为-1。
// 这里iface为1，其中0是0表示回环设备的接口号，1则是客户端设备的接口号
uint32_t iface = ipv4->GetInterfaceForDevice(device);
/*
 * 因为可以删除地址，所以在调用此方法时，addressIndex不保证是静态的。
 * 返回与接口和地址索引关联的Ipv4接口地址给实例化对象ifaddr,
 * 由于这个接口只有一个ipv4接口地址被分配，所以索引为0（指向第一个接口地址）
 */
Ipv4InterfaceAddress ifaddr = ipv4->GetAddress(iface, 0);
// Ipv4InterfaceAddress实例化对象调用方法GetLocal得到ip接口地址
Ipv4Address IpTo = (ifaddr).GetLocal();
return IpTo;//返回目标节点ip
}

/*
 *  函数定义 此函数启动一个客户端类方法来安排新传送事件（不从外部应获得的节点索引来设定目标地址）
 */
void ExternallyDrivenSim::RunScheduleTransmit(void)
{
NS_LOG_UNCOND("Now ExternallyDrivenSim::RunScheduleTransmit run");
// 根据传输层协议的不同分别启用UDP客户端类函数和TCP客户端类函数
if(g_trans_protocol_mode == 1)
{
/*
 * m_nodes.Get:实例化对象m_nodes调用Get方法得到索引为m_nodId的节点的智能指针
 * GetClient:此函数返回指向UdpEchoClientNew*客户端新应用程序对象的普通（非智能）指针，
 * 使用的是指向该节点的智能指针
 */
m_nod = m_nodes.Get(m_nodId);
m_pucl = GetClient(m_nod);
// 这里定义ns3的时间类变量JumpTime，用于推算下一次同步事件时间点后被安排的客户端发送数据包事件的时间
/*
 * NanoSeconds:以纳秒为单位创建ns3 :: Time实例。
 * 内联函数TimeStep:返回time实例类型当前仿真时间
 * Seconds：以秒为单位创建ns3::Time实例。
 *
 * ns3的Time类模拟虚拟时间值和全局模拟分辨率，此类还控制基础时间值的分辨率。
 * 分辨率是可表示的最小时间间隔。 默认分辨率为纳秒。
 * 因而在仿真中安排事件的时间分辨率也才用默认分辨率为纳秒。
 */
Time JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
// 索引为m_nodId的客户端节点计划一个在JumpTime之后的发送事件
m_pucl->ScheduleTransmit(JumpTime);
}
else if(g_trans_protocol_mode == 2)
{
m_nod = m_nodes.Get(m_nodId);
m_pucl_tcp = GetClient_tcp(m_nod);

Time JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
m_pucl_tcp->ScheduleTransmit(JumpTime);
}
}

// 函数定义 此函数启动一个客户端类方法来安排新传送事件（从外部应获得的节点索引来设定目标地址，
// 并把数据包内容当做参数）
void ExternallyDrivenSim::RunScheduleTransmitTo(std::string PayLoad,int mutipac,int statenum,uint64_t delaysend)
{
NS_LOG_UNCOND("Now ExternallyDrivenSim::RunScheduleTransmitTo run with PayLoad");
// 将参数数据包的内容赋值给变量Pay_Load，作为计划发送函数的参数
std::string Pay_Load = PayLoad;
m_timeOfStep = delaysend;
// 对于不同的传输层协议模式，由于使用的客户端类是不同的，因此也要根据从仿真脚本获得的
// g_trans_protocol_mode判断来选取不同的客户端类函数来计划数据包发送事件
if(g_trans_protocol_mode == 1)
{
m_nod = m_nodes.Get(m_nodId);
m_pucl = GetClient(m_nod);

Ipv4Address IpTo = GetDestIp(m_nodes.Get(m_nodeIdTo));// 确定目标节点的Ip地址
// 把从节点客户端应用程序获得的需要发送的内容，赋值给对应应用程序的数据包内容属性
Time JumpTime;
// 对于不同的驱动模式，下一次同步事件时间点后被安排的客户端发送数据包事件的时间采用不同的情况推算
if(g_qudong_mode == 1)
{
// 对于时间驱动仿真模式，由于运行到这时，之前判断f_next.key.m_ts > m_TimeLimit为true，
// 因而会运行该语句m_TimeLimit += (uint64_t) g_SimulationStep.GetNanoSeconds();
// 该语句用来更新这次仿真交互中ns3的仿真时间上限为下下一次的同步事件时间点，然而我们计划
// 客户端在下一个同步事件时间点后发送数据包，因此我们通过
// NanoSeconds(m_TimeLimit) - g_SimulationStep 将时间回退到下一个同步事件时间点，
// 之后减去TimeStep(m_currentTs)，如果想让数去推迟一段发送的时间，还需要加上
// NanoSeconds(m_timeOfStep),通过这次推算我们获得了如果客户端需要仿真发送数据包事件，
// 需要推迟多少时间,JumpTime将会做为安排事件函数的时间参数
JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
}
if(g_qudong_mode == 2)
{
// 对于事件驱动模式，将会根据in_sam_trans标志选择不同的JumpTime计算
if(in_sam_trans == true)
{
// 如果in_sam_trans == true，也就是说异步事件发生了，如果在异步事件控制器节点接收
// 到了数据包，如果为事件驱动，那么一般控制器节点那么控制器将会立即计算出控制量后发送
// 给被控对象，这个时间几乎为0,如果m_timeOfStep = 0；那么JumpTime = 0，也就是在
// 该异步事件之后，控制器的客户端立即计划发送控制量数据包事件。
JumpTime = NanoSeconds(0) + NanoSeconds(m_timeOfStep);
}
else
{
// in_sam_trans == flase,意味着被计划的是同步事件时间点的后客户端发送数据包事件，
// 那么任然按照时间驱动仿真模式的方法进行推算
JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
}
}
// 索引为m_nodId的客户端节点计划一个在JumpTime之后的发送事件,这里使用了的客户端计划
// 发送函数的输入变量含有目标IP，以此来安排具体的传送目标节点,同时输出变量Pay_Load作为
// 此次发送的数据包的内容
m_pucl->ScheduleTransmit(JumpTime, IpTo,Pay_Load,mutipac,statenum);
}
else if(g_trans_protocol_mode == 2)// TCP传输层模式，具体逻辑与UDP一样
{
m_nod = m_nodes.Get(m_nodId);
m_pucl_tcp = GetClient_tcp(m_nod);

Ipv4Address IpTo = GetDestIp(m_nodes.Get(m_nodeIdTo));
Time JumpTime;
if(g_qudong_mode == 1)
{
JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
}
if(g_qudong_mode == 2)
{
if(in_sam_trans == true)
{
JumpTime = NanoSeconds(0) + NanoSeconds(m_timeOfStep);
}
else
{
JumpTime = NanoSeconds(m_TimeLimit) - g_SimulationStep -
TimeStep(m_currentTs) + NanoSeconds(m_timeOfStep);
}
}
m_pucl_tcp->ScheduleTransmit(JumpTime, IpTo,Pay_Load,mutipac,statenum);
}
}

// 定义调度整个仿真运行过程，安排发送事件与运行仿真事件的函数，是父类的SimulatorImpl::Run函数的实现
void ExternallyDrivenSim::Run(void)//外部驱动仿真运行
{
// 仿真开始前先对trans结构体数组进行置0处理
for(int i = 0;i < m_nodenumber;i++)
{
time_determine[i] = 0;
}
for (int i = 0; i < m_nodenumber; i++)// 某一节点发送传送包的触发信号置0
{
trans[i].sig = 0;// 初始化sig
memset(trans[i].packetcontent,0,sizeof(trans[i].packetcontent));
memset(trans[i].Timedelay,0,sizeof(trans[i].Timedelay));
memset(trans[i].PacketNum,0,sizeof(trans[i].PacketNum));
memset(trans[i].Sim_time,0,sizeof(trans[i].Sim_time));
memset(trans[i].Send_time,0,sizeof(trans[i].Send_time));
memset(trans[i].TimedelayReal,0,sizeof(trans[i].TimedelayReal));
}
m_stop = false;// 设置仿真结束的标志为假
// 设置ns3第一步的仿真的时间上限为g_SimulationStep（也就是每个同步事件时间点的间隔），
// 也就意味着第一段的仿真事件运行的发生时间不会超过第一个同步事件时间点
m_TimeLimit = (uint64_t)g_SimulationStep.GetNanoSeconds();
if(g_qudong_mode == 2)
{
// 如果选择了事件驱动模式，那么仿真之前先把in_sam_trans置为false
in_sam_trans = false;
}

// 这一部分将根据仿真事件的类型和时间对仿真过程进行调度
/*
 * 当循环重新运行到此判断时，意味着在下一个同步事件时间点之前的仿真事件已经完成，
 * 在该同步事件时间点与matlab进行一次交互后，当事件列表非空，并且m_stop为false时，
 * 且(TimeStep(m_TimeLimit)< m_TimeOfEnd + 2*g_SimulationStep)满足时
 * （只有+ 2*g_SimulationStep仿真时间上限才会达到m_TimeOfEnd，仿真才会运行到结束时间点
 * m_TimeOfEnd+g_SimulationStep），将会开始下下一个同步事件时间点
 * （此时同步事件时间点已经更新其时间上限）之前的仿真，如果此判断成立仿真没有停止，
 * 我们将会从事件列表中取出下一个事件（下一个同步事件时间点之后的事件）继续执行仿真
 * 为什么要运行到m_TimeOfEnd+g_SimulationStep而不是m_TimeOfEnd，因为仿真的第一个同步周期步长
 * 节点在进行初始化，真正的仿真实际上是从第二步开始的
 */
while (!m_events->IsEmpty()&&!m_stop && (TimeStep(m_TimeLimit)
< m_TimeOfEnd + 2*g_SimulationStep))
{
/*
 * m_events->PeekNext:指向下一个最早事件的指针。
 * 调用者获取返回的指针的所有权。如果列表为空，则不能调用此方法。
 * 如果继续执行仿真，那么将选取下一个仿真事件，这里返回下一个事件结构体给f_next。
 */
Scheduler::Event f_next = m_events->PeekNext();

// 测试是否应该结束仿真，当m_TimeLimit != 0且下一个仿真事件时间点大于仿真结束时间点
// m_TimeOfEnd + g_SimulationStep或者
// m_TimeLimit>= (uint64_t) m_TimeOfEnd.GetNanoSeconds() + g_SimulationStep.GetNanoSeconds()
// 时，判断仿真应该结束,此时仅仅发送通知，外部应用因为仿真完成不在发送通知，所以不再进行运行侦听函数
// 只要TransmitNotices()正常发送，没有因为阻塞套接字send失败而阻塞，那么运行Stop(Seconds(0.));和m_quit = true;
// 并到之前的while处循环，这时候由于事件不为空，m_stop为false，
// 而m_TimeLimit == (uint64_t) m_TimeOfEnd.GetNanoSeconds() + g_SimulationStep.GetNanoSeconds()
// 所以这时候while而判断为true，这次进入循环得到的事件为仿真结束事件，它的时间是小于
// m_TimeOfEnd + g_SimulationStep的（即m_TimeLimit）,所以转到while (f_next.key.m_ts < m_TimeLimit)
// 处运行仿真结束事件，仿真结束事件运行ExternallyDrivenSim类公有继承自默认仿真器的公有函数
// DefaultSimulatorImpl::stop(),该函数将m_stop置为true，之后刷新m_TimeLimit的值（加上一个同步周期步长），
// 之后由于m_quit值为true，再次运行TransmitNotices()再给matlab发送一个通知，之后重新进入循环while，
// 此时由于m_stop为true，且（m_TimeLimit == m_TimeOfEnd + 2*g_SimulationStep），
// 所以开始跳出循环,结束run()函数的运行，并运行一系列仿真中定义的对象的析构函数结束仿真。
if ((m_TimeLimit != 0) && (NanoSeconds(f_next.key.m_ts)
>= m_TimeOfEnd+ g_SimulationStep) && (m_TimeLimit
>= (uint64_t) m_TimeOfEnd.GetNanoSeconds() + g_SimulationStep.GetNanoSeconds()))
{
NS_LOG_UNCOND("运行第一个Transmitnotices()仿真应该结束,此时仅仅发送通知，外部应用因为仿真完成不在发送通知，所以不再进行运行侦听函数" << "\n");
TransmitNotices();// 仿真结束前此功能将最后一步仿真结果的通知传送给外部应用程序（matlab）

Stop(Seconds(0.));// 发送通知后计划暂停仿真事件在0.秒time类型（单位）
m_quit = true;// 手动结束仿真标志置为真
}
// 如果上面条件不满足，意味着仿真没有结束，那么将进行下一个仿真事件运行调度
// 下一个事件计划时间大于等于m_TimeLimit,意味着下一个事件将会在下下一个同步事件时间点
// 之后运行这其实就意味着下一个同步事件时间点之后的整个仿真的同步周期步长内部将不会有事件发生，
// 所以我们输出Nothing to do now.
else if (f_next.key.m_ts >= m_TimeLimit)
{
NS_LOG_UNCOND("Next simulationstep nothing to do now.\n" << "\n");
// 如果这个同步周期步长内部没有事件，那么通过更新仿真运行时间上限到下一个同步事件时间点，
// 运行下一段同步周期步长内部的事件
NS_LOG_UNCOND("\n" << " Prevent m_TimeLimit is \n" <<m_TimeLimit<<"\n");
m_TimeLimit +=(uint64_t) g_SimulationStep.GetNanoSeconds();
// 由于该同步周期步长内部没有事件，所以直接在同步事件时间点与matlab进行一次交互，
// 运行输出现在的仿真同步事件时间点、ns3的仿真时间以及联合仿真的时间（由于第一段同步周期步长）
// 仅仅是ns3中各个节点应用的初始化，不是真正意义上的仿真过程，所以联合仿真时间要减去一个
// 同步周期步长
NS_LOG_UNCOND("\n" << " After update now m_TimeLimit is \n" <<m_TimeLimit<<"\n");
NS_LOG_UNCOND("\n" << " Now ns3 simulation time is \n"
<<ns3::Simulator::Now().GetSeconds()<<"\n");
NS_LOG_UNCOND("\n" << " Now co-simulation time is \n"
<<ns3::Simulator::Now().GetSeconds() -
g_SimulationStep.GetSeconds()<<"\n");
if (!m_quit)
{
// 手动停止仿真标志m_quit = false 即仿真不手动停止, 那么在同步事件时间点
// 与matlab进行一次交互,由于仿真没有停止还需要获得matlab的通知来推进仿真
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第二个TransmitNotices()"<<"\n");
TransmitNotices();// 此功能将通知传送给外部应用程序。
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第一个Listen()"<<"\n");
Listen();// 继续接受外部应用的数据包
}
else
{
// 手动停止仿真标志m_quit = false 即仿真需要手动停止,
// 那么此次交互仅仅发送给matlab通知即可，不需要获得matlab通知来推进仿真
NS_LOG_UNCOND("\n" << " 判断m_quit为真，运行第三个TransmitNotices()"<<"\n");
TransmitNotices();
// 在这之后同一时刻计划一个仿真stop时间，仿真stop后仿真器即仿真调用的所有对象将会被析构函数销毁，
// 子套接字连接和原始套接字连接都将会关闭
Stop(Seconds(0.));// 发送通知后计划暂停仿真事件在0.秒time类型（单位）
}
}
// 如果下一个事件的计划时间满足f_next.key.m_ts <= m_TimeLimit
// 也就说明下一个同步周期步长内部将会用事件发生，所以对这个同步周期步长
// 内部的事件进行调度
else
{
// 通过不断的循环，运行该同步周期步长内部的所有仿真事件，如果是事件驱动
// 仿真模式，还将计划一些联合仿真过程中的异步事件
while (f_next.key.m_ts < m_TimeLimit)
{
// 不断提取出事件列表中的下一个仿真事件，通过获得该仿真事件的时间
// 并与m_TimeLimit进行比较
f_next = m_events->PeekNext();
NS_LOG_UNCOND("\n" << " process on next event \n" <<f_next.key.m_ts<<"\n"<<m_TimeOfEnd<<"\n");
// 如果在该同步周期步长内部推进仿真的过程中，判断到下一个事件的计划时间
// 大于等于m_TimeLimit,也就是在下一个同步事件时间点（此时仿真已经进入同步周期步长内部）
// 之后，那么我们将会跳出循环while (f_next.key.m_ts < m_TimeLimit)，
// 意味着这个同步周期步长内部的事件全部运行完成
if (f_next.key.m_ts >= m_TimeLimit)
{
break;
/*
 * 下一个事件的计划时间大于等于m_TimeLimit，跳出循环while (f_next.key.m_ts < m_TimeLimit)
 */
}
// 如果不满足f_next.key.m_ts >= m_TimeLimit，使用推进仿真函数，
// 执行该同步周期步长内部的下一个事件
ProcessOneEvent();
// 如果使用者采用事件驱动模式，那么将会对该同步周期步长内部的服务器接收事件进行响应，
// 把它们当做仿真过程中的异步事件，并在该异步事件处与maltab进行一次交互
if(g_qudong_mode == 2)
{
// 当in_sam_trans标志为true时，也就意味着在该同步周期步长内部有服务器接收事件
if(in_sam_trans == true)
{
// 在异步事件处与maltab进行一次交互
if (!m_quit)// m_quit = false 即仿真不手动停止
{
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第四个TransmitNotices()"<<"\n");
TransmitNotices();// 此功能将通知传送给外部应用程序。
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第二个Listen()"<<"\n");
Listen();// 继续接受外部应用的包
}
else// 手动停止
{
NS_LOG_UNCOND("\n" << " 判断m_quit为真，运行第五个TransmitNotices()"<<"\n");
TransmitNotices();// 此功能将通知传送给外部应用程序。
Stop(Seconds(0.));// 发送通知后计划暂停仿真事件在0.秒time类型（单位）
}
// 一次异步事件处的交互完成，将in_sam_trans置为false，直到下一个服务器接收事件
// 发生才会将其重新置为真，意味着异步事件发生
in_sam_trans = false;
}
}
}
// 当这个同步周期步长内部的事件全部运行完成事跳出到该处， 并用ns3日志进行相应的输出
// 输出时间到，即到达下一个同步事件时间点
NS_LOG_UNCOND("\n" << " Time’s up. \n" << "\n");
NS_LOG_UNCOND("\n" << " Prevent m_TimeLimit is \n" <<m_TimeLimit<<"\n");
m_TimeLimit += (uint64_t) g_SimulationStep.GetNanoSeconds();
NS_LOG_UNCOND("\n" << " After update now m_TimeLimit is \n" <<m_TimeLimit<<"\n");
NS_LOG_UNCOND("\n" << " Now ns3 simulation time is \n"
<<ns3::Simulator::Now().GetSeconds()<<"\n");
NS_LOG_UNCOND("\n" << " Now co-simulation time is \n"
<<ns3::Simulator::Now().GetSeconds() -
g_SimulationStep.GetSeconds()<<"\n");
// 在该同步事件时间点与matlab客户端进行一次交互
if (!m_quit)
{
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第六个TransmitNotices()"<<"\n");
TransmitNotices();// 此功能将通知传送给外部应用程序。
NS_LOG_UNCOND("\n" << " 判断m_quit为假，运行第三个Listen()"<<"\n");
Listen();// 继续接受外部应用的通知
}
else
{
NS_LOG_UNCOND("\n" << " 判断m_quit为真，运行第七个TransmitNotices()"<<"\n");
TransmitNotices();// 此功能将通知传送给外部应用程序。
Stop(Seconds(0.));// 发送通知后计划暂停仿真事件在0.秒time类型（单位）
}
}
}
// NS_ASSERT（NS断言宏）:在运行时，在调试构建时，如果这种情况不成立，程序会打印源文件，
// 行号和未经验证的条件，并通过调用std :: terminate停止
// 如果模拟器由于缺少事件而自然停止，请进行一致性测试以检查我们是否在此过程
// 中没有丢失任何事件。如果有事件丢失那么调用NS_ASSERT宏进行断言,对于事件队列仍有事件时强制跳出循环，
// 或者缺少事件而自然停止又没有发生丢失事件的情况，不进行断言。
NS_ASSERT(!m_events->IsEmpty() || m_unscheduledEvents == 0);
free(time_determine);
}

// 定义推进下一个仿真事件的函数
void ExternallyDrivenSim::ProcessOneEvent(void)//推进下一个事件
{
NS_LOG_INFO("Now run ProcessOneEvent");
Scheduler::Event next;// 定义一个名为next事件
// Scheduler::RemoveNext:如果列表为空，则不能调用此方法。从事件列表中删除下一个最早的事件。
next = m_events->RemoveNext();// 从仿真事件列表中移除即将要运行的事件，并返回该事件
g_EventCount++;// 执行的事件计数器加1
NS_LOG_INFO("The number of the event that is currently being executed is "
<<g_EventCount);
// 当下一个事件的时间大于等于现在的时间则不断言,即判断当前运行的事件的时间点是可运行的
NS_ASSERT(next.key.m_ts >= m_currentTs);
m_unscheduledEvents--;// 未计划调度事件减一
// 通知处理下一个计划事件的时间
NS_LOG_INFO("handle the event in time (ns3 simulation time) " << next.key.m_ts);
uint64_t m_prevTs = m_currentTs;// 获得当前仿真的运行时间，即上一个事件的运行时间
m_currentTs = next.key.m_ts;// 更新当前仿真时间等于下一个事件的时间
m_currentContext = next.key.m_context;// 上下文重新赋值
m_currentUid = next.key.m_uid;// 当前事件的id等于下一个事件的唯一ID。
g_currentTs = m_currentTs;// 当前时间赋值给g_currentTs
// 输出每个事件的仿真过程
NS_LOG_INFO("now is " << g_EventCount << " event; "
<< " total time (ns3 simulation time) :"<< (double) m_currentTs/1000000000
<< "s; value of step : " << TimeStep(
next.key.m_ts - m_prevTs) <<"\n");
// 得到下一个事件eventid给全局变量，此功能收集EventID的信息，如果下一个事件是客户端发送
// 数据包事件,那么在执行客户端发送事件时，即客户端运行SendTo函数时客户端将会执行
// ExternallyDrivenSim::SetEventId()来获得该事件的相关信息,由于SendTo函数不知道即将
// 发送的数据包的目标节点时哪个，我们通过SetEventId()获得的事件信息的ID与计划发送事件时确定
// 的ipid数组（保存的都是即将运行的发送事件的uid和发送的目标节点）中的ID做比较，
// 通过此ID来获得此次调用SendTo函数将要发送的目标节点的IP
GetEventId(next.impl, next.key.m_ts, next.key.m_context, next.key.m_uid);
next.impl->Invoke();// 由仿真引擎调用以通知事件是时候执行了。
/*
 * 减少参考计数。 这个方法不应该被用户代码调用。 SimpleRefCount实例预计会与Ptr模板一起使用，
 * 这会使调用Ref变得不必要和危险。
 * 运行该函数表示该事件被执行了
 */
next.impl->Unref();
}

/*
 * 函数定义 返回此节点的唯一id，这个唯一的ID恰好也是节点进入NodeList的索引，
 * 并产生该节点对外部应用的通知
 */
void ExternallyDrivenSim::GetNotices(UdpEchoServerNew* p)
{
/*
 * 返回此应用程序对象所连接的节点智能指针
 */
Ptr<Application> Applic= (Application*) p;
Ptr<Node> node=Applic->GetNode();
// 由于该节点接收到了数据包，所以给matlab的通知中将发送通知结构体中对应索引的sig标志置为1
trans[node->GetId()].sig = 1;
// 对于时间驱动模式，由于在一个同步周期步长内部是不会进行异步事件与matlab进行交互的，因此
// 如果一个同步周期步长内部某个节点的服务器接收到了多个数据包，那么就需要在同步事件时间点时
// 与matlab交互时将这个仿真同步周期步长内部所有接收到的包都集合起来发送给matlab，因而对trans
// 结构体将会进行多次的写入操作，在该同步周期步长内部第一次接收到包时，使用strcpy和sprintf来
// 写入，而之后接收到的包的写入将使用strcat函数，将之后接收到的包的数据写入在前一次包的后面,
// 每次接收到的包的内容将会用"\n"或"   "进行分隔，方便matlab接收到通知后进行解析，为了区分是
// 该仿真同步周期步长接收的第一个包还是之后接收的包，以此来分别进行写入，我们采用int类数组对象
// time_determine与当前的m_TimeLimit进行比较来判断，time_determine数组保存的是该同步
// 周期步长的m_TimeLimit，当每次节点服务器接收到数据包时，写入数据包数据到对应节点索引的trans
// 结构体分量后，会对该节点索引的time_determine分量进行更新，因此在该仿真同步周期步长内接收
// 到第一个数据包时，接收数据包的节点索引的time_determine分量保存的仍然是上一个仿真同步周期
// 步长的m_TimeLimit(如果上一个仿真同步周期步长内没有接收到数据包，那么保存的可能是更加早的
// m_TimeLimit),然而此时仿真已经进入新的仿真同步周期内，m_TimeLimit已经更新了，因此会满足
// 条件m_TimeLimit != time_determine[node->GetId()]，这样我们的仿真数据的写入方式就会
// 采用strcpy和sprintf函数，并且不用写入分隔的字符串，之后该节点索引的time_determine分量
// 更新了，这就意味着该节点索引的time_determine分量已经满足新的仿真同步周期的m_TimeLimit，
// 这样在该节点服务器第二次接收到数据包时，满足条件
// m_TimeLimit == time_determine[node->GetId()],于是我们采用strcat的写入函数，并写入
// 相应的分隔符，这样在该仿真同步周期结束时，该节点索引的trans结构体分量内保存的就是可以解析的
// 整个仿真同步周期接收到的所有数据包信息的集合
if(g_qudong_mode == 1)
{
if(m_TimeLimit != time_determine[node->GetId()])
{
// strcpy是字符串变量的复制函数
strcpy(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
// sprintf函数可以将数值类型的变量内容写入到char型数组
sprintf(trans[node->GetId()].Timedelay, "%lf", p->packetdelay);
sprintf(trans[node->GetId()].PacketNum, "%d", p->RCPacketNum);
sprintf(trans[node->GetId()].Sim_time, "%lf", p->sim_time);
sprintf(trans[node->GetId()].Send_time, "%lf", p->send_time);
time_determine[node->GetId()] = m_TimeLimit;
}
else if(m_TimeLimit == time_determine[node->GetId()])
{
// 不同数据包的内容通过"\n"进行分隔，由于有的节点接收到的数据包信息包含多个变量
// 因此我们对于不同时刻接收到的数据包采用换行分隔，方便matlab区分是不同数据包的内容
strcat(trans[node->GetId()].packetcontent,"\n");
strcat(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
// 对于这个数据包的时延、编号和接收到该数据包的时间都是单变量，因此通过"   "进行分隔
// 不需要换行来区分
strcat(trans[node->GetId()].Timedelay,"   ");
char timedelay_buf[12];
sprintf(timedelay_buf, "%lf", p->packetdelay);
strcat(trans[node->GetId()].Timedelay,timedelay_buf);
strcat(trans[node->GetId()].PacketNum,"   ");
char PacketNum_buf[12];
sprintf(PacketNum_buf, "%d", p->RCPacketNum);
strcat(trans[node->GetId()].PacketNum,PacketNum_buf);
strcat(trans[node->GetId()].Sim_time,"   ");
char Sim_time_buf[12];
sprintf(Sim_time_buf, "%lf", p->sim_time);
strcat(trans[node->GetId()].Sim_time,Sim_time_buf);

strcat(trans[node->GetId()].Send_time,"   ");
char Send_time_buf[12];
sprintf(Send_time_buf, "%lf", p->send_time);
strcat(trans[node->GetId()].Send_time,Send_time_buf);
time_determine[node->GetId()] = m_TimeLimit;
}
}
// 对于事件驱动模式，我们会在异步事件时间点与matlab进行一次交互，把这次节点服务器接收到的包的
// 内容发送出去，因此不会有一次交互需要发送多个数据包信息的情况，因此直接写入到trans结构体即可
if(g_qudong_mode == 2)
{
strcpy(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
sprintf(trans[node->GetId()].Timedelay, "%lf", p->packetdelay);
sprintf(trans[node->GetId()].PacketNum, "%d", p->RCPacketNum);
sprintf(trans[node->GetId()].Sim_time, "%lf", p->sim_time);
sprintf(trans[node->GetId()].Send_time, "%lf", p->send_time);
sprintf(trans[node->GetId()].TimedelayReal, "%lf", p->packetdelayReal);
}
}

// 前面的函数定义是适用于使用UDP协议的，这里的函数定义是适用于TCP协议的，具体形成通知方式类似
void ExternallyDrivenSim::GetNotices(TcpServer* p)
{
Ptr<Application> Applic= (Application*) p;
Ptr<Node> node=Applic->GetNode();

trans[node->GetId()].sig = 1;
if(g_qudong_mode == 1)
{
if(m_TimeLimit != time_determine[node->GetId()])
{
strcpy(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
sprintf(trans[node->GetId()].Timedelay, "%lf", p->packetdelay);
sprintf(trans[node->GetId()].PacketNum, "%d", p->RCPacketNum);
sprintf(trans[node->GetId()].Sim_time, "%lf", p->sim_time);
sprintf(trans[node->GetId()].Send_time, "%lf", p->send_time);
time_determine[node->GetId()] = m_TimeLimit;
}
else if(m_TimeLimit == time_determine[node->GetId()])
{
strcat(trans[node->GetId()].packetcontent,"\n");
strcat(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
strcat(trans[node->GetId()].Timedelay,"   ");
char timedelay_buf[12];
sprintf(timedelay_buf, "%lf", p->packetdelay);
strcat(trans[node->GetId()].Timedelay,timedelay_buf);
strcat(trans[node->GetId()].PacketNum,"   ");
char PacketNum_buf[12];
sprintf(PacketNum_buf, "%d", p->RCPacketNum);
strcat(trans[node->GetId()].PacketNum,PacketNum_buf);
strcat(trans[node->GetId()].Sim_time,"   ");
char Sim_time_buf[12];
sprintf(Sim_time_buf, "%lf", p->sim_time);
strcat(trans[node->GetId()].Sim_time,Sim_time_buf);

strcat(trans[node->GetId()].Send_time,"   ");
char Send_time_buf[12];
sprintf(Send_time_buf, "%lf", p->send_time);
strcat(trans[node->GetId()].Send_time,Send_time_buf);
time_determine[node->GetId()] = m_TimeLimit;
}
}
if(g_qudong_mode == 2)
{
strcpy(trans[node->GetId()].packetcontent,(p->RCdata).c_str());
sprintf(trans[node->GetId()].Timedelay, "%lf", p->packetdelay);
sprintf(trans[node->GetId()].PacketNum, "%d", p->RCPacketNum);
sprintf(trans[node->GetId()].Sim_time, "%lf", p->sim_time);
sprintf(trans[node->GetId()].Send_time, "%lf", p->send_time);
sprintf(trans[node->GetId()].TimedelayReal, "%lf", p->packetdelayReal);
}
}

/*
 * 函数定义 此函数将通知传送给外部应用程序，也就是matlab的客户端
 */
void ExternallyDrivenSim::TransmitNotices(void)
{
NS_LOG_UNCOND("carry ExternallyDrivenSim::TransmitNotices\n");
// 输出节点接收数据包标志
std::cout<<"m_quit:\n "<<m_quit<<std::endl;
std::cout<<"节点的发送标志及内容:\n";
for(int i = 0;i<m_nodenumber-1;i++)
{
std::cout<<"节点"<<i<<" 发送标志:"<<trans[i].sig<<" and "<<"内容:"<<trans[i].packetcontent<<"\n";

}
std::cout<<"节点"<<m_nodenumber-1<<" 发送标志:"<<trans[m_nodenumber-1].sig<<" and "<<"内容:"<<trans[m_nodenumber-1].packetcontent<<"\n"<<"\n"<<std::endl;

send(m_sock, trans,m_nodenumber*sizeof(*trans), 0);// 发送trans结构体数组
std::cout<<"socket_send"<<m_sock;

std::cout<<"send"<<"m_nodenumber:"<<m_nodenumber<<"\n"<<"sizeof(*trans):"<<sizeof(*trans)<<std::endl;
// 发送完成trans结构体数组后，将trans结构体数组置0，为下一次的交互做准备
for (int i = 0; i < m_nodenumber; i++)
{
trans[i].sig = 0;// 复位节点接收数据包标志
memset(trans[i].packetcontent,0,sizeof(trans[i].packetcontent));
memset(trans[i].Timedelay,0,sizeof(trans[i].Timedelay));
memset(trans[i].PacketNum,0,sizeof(trans[i].PacketNum));
memset(trans[i].Sim_time,0,sizeof(trans[i].Sim_time));
memset(trans[i].Send_time,0,sizeof(trans[i].Send_time));
memset(trans[i].TimedelayReal,0,sizeof(trans[i].TimedelayReal));
}
}

/*
 * 函数定义 此函数收集当前EventID的信息，即当前事件的信息，不同事件有不同的事件id
 */
void ExternallyDrivenSim::GetEventId
(Ptr<EventImpl> m_eventImpl,uint64_t m_ts, uint32_t m_context, uint32_t m_uid)
{
g_EventImpl = m_eventImpl;
g_ts = m_ts;
g_context = m_context;
g_uid = m_uid;
}

/*
 * 此函数返回当前Event的EventId给某个eventid类变量，使得节点服务器知道现在运行的发送事件的m_uid
 */
EventId ExternallyDrivenSim::SetEventId(void)
{
// 使用EventId构造函数构造一个真实的事件，并返回
return EventId(g_EventImpl, g_ts, g_context, g_uid);
}

/*
 * 函数定义 此函数允许设置仿真的同步周期步长（仿真每个同步事件时间点的间隔）
 */
void ExternallyDrivenSim::SetSimulationStep(Time time)
{
NS_LOG_UNCOND ("carry ExternallyDrivenSim::SetSimulationStep");
g_SimulationStep = time;
NS_LOG_UNCOND ("after carry ExternallyDrivenSim::SetSimulationStep g_SimulationStep "
<<g_SimulationStep);
}

// 函数定义 计划停止仿真事件及其时间
void ExternallyDrivenSim::Stop(Time const &time)
{
m_TimeOfEnd = time;// 设置仿真的相对停止时间（即从现在的仿真时间往后多少时间仿真停止）
/*
 * Simulator::Schedule：安排一个事件在相对时间到达“时间”时到期。
 * 这可以被认为是为当前模拟时间和作为参数传递的时间计划的一个事件，
 * 当事件到期时（当它运行时），输入方法将在输入对象上被调用。
 *
 * Simulator::Stop：告诉模拟器，该调用事件应该是最后执行的事件。所以计划该最后执行事件
 * 也就是仿真停止事件在time之后运行
 */
Simulator::Schedule(time, &Simulator::Stop);// 计划仿真停止事件
}

// 函数定义 此函数用于在一个仿真同步周期步长内部，当服务器接收到数据包时触发异步仿真事件标志
// 同时输出该异步仿真事件发生时，仿真时间的相关信息（适用于事件驱动模式）
void ExternallyDrivenSim::trans_noti_insam(void)
{
NS_LOG_UNCOND ("carry ExternallyDrivenSim::trans_noti_insam");
NS_LOG_UNCOND ("\n" << "transmit notice and listen notice in one SimulationStep \n" << "\n");
NS_LOG_UNCOND ("\n" << "Now m_TimeLimit is \n" <<m_TimeLimit<<"\n");
NS_LOG_UNCOND ("current ns3 simulation time is \n"
<< ns3::Simulator::Now().GetSeconds()<<"\n");
NS_LOG_UNCOND ("current co-simulation time is \n"
<< ns3::Simulator::Now().GetSeconds() -
g_SimulationStep.GetSeconds()<<"\n");
in_sam_trans = true;// 触发异步仿真事件标志为真
}

// 函数定义 此函数使用仿真脚本获得的节点数量变量来设置ExternallyDrivenSim的私有属性m_nodenumber
void ExternallyDrivenSim::get_node_number(int nodenumber)
{
NS_LOG_UNCOND ("carry ExternallyDrivenSim::get_node_number");
m_nodenumber = nodenumber;
NS_LOG_UNCOND ("after carry ExternallyDrivenSim::get_node_number m_nodenumber "
<<m_nodenumber);
}

// 函数定义 此函数让UdpEchoServerNew应用获得外部应用程序发送过来的所有要发送的控制系统目标节点ID
// 以及控制系统目标节点的数量
void ExternallyDrivenSim::ser_get_des(UdpEchoServerNew* pp)
{
// 以控制系统目标节点的数量为次数循环，使UdpEchoServerNew类的des数组获得所有的控制系统目标节点ID
for(int i = 0;i<k_des;i++)
{
pp->des[i] = 0;
 pp->des[i] = get_des[i];
}
// 使UdpEchoServerNew类的sys_ser_des变量获得控制系统目标节点的数量
pp->sys_ser_des = k_des;
}

// 函数定义 此函数让TcpServer应用获得外部应用程序发送过来的所有要发送的控制系统目标节点ID以及
// 控制系统目标节点的数量
void ExternallyDrivenSim::ser_get_des(TcpServer* pp)
{
for(int i = 0;i<k_des;i++)
{
pp->des[i] = 0;
pp->des[i] = get_des[i];
}
pp->sys_ser_des = k_des;
}

// 函数定义 此函数使用仿真脚本获得的仿真驱动模式变量来设置ExternallyDrivenSim的私有属性g_qudong_mode
void ExternallyDrivenSim::Setqudongmode(uint16_t qudong_mode)
{
NS_LOG_UNCOND ("carry ExternallyDrivenSim::Setqudongmode");
g_qudong_mode = qudong_mode;
NS_LOG_UNCOND ("after carry ExternallyDrivenSim::Setqudongmode g_qudong_mode "
<<g_qudong_mode);
}

// 函数定义 此函数使用仿真脚本获得的传输层模式变量来设置ExternallyDrivenSim的私有属性g_trans_protocol_mode
void ExternallyDrivenSim::Set_trans_protocolmode(uint16_t trans_protocolmode)
{
NS_LOG_UNCOND ("carry ExternallyDrivenSim::Set_trans_protocolmode");
g_trans_protocol_mode = trans_protocolmode;
NS_LOG_UNCOND ("after carry ExternallyDrivenSim::Set_trans_protocolmode g_trans_protocol_mode "
<<g_trans_protocol_mode);
}
}

