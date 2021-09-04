//xcos程序运行与通信
//作者：卢帅领
//日期：2020.12


 exec('/home/ling/Scilab/scilab-6.1.0/client/controller.sci', -1)
 exec('/home/ling/Scilab/scilab-6.1.0/client/actuator.sci', -1)
 exec('/home/ling/Scilab/scilab-6.1.0/client/Plant.sci', -1)
 

//程序开始（设置节点数、发送数据包的初值）
    order=2;    nodenumber=4;//设置节点个数。
    //设置发送给NS3的控制量的初始值，这里设定的节点node 0。
    //第一位程序初始化node0的发送时刻1.5s
    C_A_data=[0,0];
    //设置发送给NS3的传感值的初始值，这里设定的是节点node 3。
    //第一位程序初始化node3的发送时刻1.5s
    S_C_data=[1,1];
    
    //运行接口程序设定NS3内的四个节点，控制器节点C node 0->执行器节点A node 2；
    //传感器节点S node 3->控制器节点C node 1，
    //这里node 1和node 0为不同时刻的控制器节点
    main;//运行交互客户端接口程序，进行一次交互。
    
    
   //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive_time.txt");
   //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_time.txt");
   //node1recv_time=node1receive_time;
   //node2recv_time=node2receive_time;
   current_time=1.5;
   
   //获取节点0和节点3的接受数据包时间
   //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node0send_time.txt");
   //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node3send_time.txt");
    //node0send_time=node2recv_time-node2receive_delay
    //node3send_time=node1recv_time-node1receive_delay
    State.values=[1,1];//状态量设置初始值
    State.time=(5.01:5:10)';//设置运行时间点 
//打开文件缓冲区的文件并读取ns3仿真内容
    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive.txt");//ns3仿真后node 1接受的数据包内容
    while  node1receive==9999
        main
        loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive.txt");
        end
    S_C_SCI.time=(5.01:5:10)';
    //从NS3导出的数据加上时间戳
    S_C_SCI.values=[node1receive]';//数据传递到scilab运行空间

    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive.txt");//ns3仿真后node 2输的数据包内容
    //设置判断节点是否接受
    while  node2receive==9999
        main
        loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive.txt");
        end
        
    C_A_SCI.time=(5.01:5:10)';
    C_A_SCI.values=[node2receive]';//数据传递到scilab运行空间
    
    
    ut_memory.values=C_A_SCI.values;
    ut_memory.time=(5.01:5:10)';
    
    
    
    
    //打开并运行xcos控制系统模型
    xcos('/home/ling/Scilab/scilab-6.1.0/client/daxingtest01.zcos');//打开xcos模型
    [result]=importXcosDiagram('/home/ling/Scilab/scilab-6.1.0/client/daxingtest01.zcos');
    xcos_simulate(scs_m, 4);//从scilab数据传递到xcos运行


    outtemp=[S_C_NS3.values];
    //fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/record.txt', ry.values, "%1f");


    //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_delay.txt");
    //打开文件缓冲区获取延时数据存入中间变量temp
    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_delay.txt");//node2的延迟数据
    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive_delay.txt");
    midnumber=node1receive_delay+node2receive_delay;
    //建立中间缓存区
    temp=[midnumber];
    temptest1=C_A_data;
    temptest2=C_A_SCI.values;
    
    
    ut_memory.values=[ut_memory_update.values];
    ut_memory.time=(5.01:5:10)';
//构建循环体进行多次采样交互
//在循环体内进行xcos仿真
  t=1;    //循环次数初值，即仿真时常=仿真次数*仿真步长
  while t<100//设置循环体运行次数（采样sample），进行联合仿真
    //传感器发送数据到C_A_NS3,时间戳为
    //传感器节点3的发送数据包的时间node3send_time
    //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_delay.txt");
    //delaytemp=node1receive_delay;
    C_A_data=[C_A_NS3.values];
    
    
    S_C_data=[S_C_NS3.values];
    State.values=S_C_NS3.values;

    main;
    
    temptest1=[temptest1,C_A_NS3.values];
    temptest2=[temptest2,C_A_SCI.values];
    //node2延迟数据保留到record
    
    //C_A_SCI.values
    
   
   loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive_delay.txt");
   //V.time=node2receive_delay;
   
   loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_delay.txt");
   midnumber=node1receive_delay+node2receive_delay;
   
   //midnumber=temp;
   temp=[temp,midnumber];
   //fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/record.txt', temp, "%1f");
   
   
   //从文件中提取节点接受到的数据
    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive.txt");
    while  node1receive==9999
        main
        loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive.txt");
    end
    loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive.txt");
    
    while  node2receive==9999
        main
        loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive.txt");
        end
   S_C_SCI.values=[node1receive]';
   
   
   C_A_SCI.values=[node2receive]';
   
   //运行xcos仿真模型
   [result]=importXcosDiagram('/home/ling/Scilab/scilab-6.1.0/client/daxingtest01.zcos');
   xcos_simulate(scs_m, 4);
   //fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/record.txt', q, "%1f");
    //loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/record.txt");
    //midnumber=record;
    
    //缓存系统输出值到文件outrecord.txt
   outtemp=[outtemp;S_C_NS3.values];
   
    //更新Actutor中存储的ut的值
    ut_memory.values=[ut_memory_update.values];
    ut_memory.time=(5.01:5:10)';
    
   
   
   //fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/recordtest1.txt', temptest1, "%1f");
   //fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/recordtest2.txt', temptest2, "%1f");
   
   
   //获取节点1和节点2的接受数据包时间
   loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node2receive_time.txt");
   loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/node1receive_time.txt");
   node1recv_time=node1receive_time;
   node2recv_time=node2receive_time;
   
   
   //获取闭环延时
   
   
   
   
   current_time=current_time+0.03;
   //获取闭环延时
   close_loop_delay(t)=current_time-ut_memory.values(1,2);
   t=t+1;
   
  end//结束循环体
  fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/outrecord.txt', outtemp, "%1f");
  fprintfMat('/home/ling/Scilab/scilab-6.1.0/client/record.txt', temp, "%1f");
  
 //tt=1:29;
 //plot(tt,close_loop_delay) ;
//整理数据，选择数据进行画图
//x=(1:100);
//loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/record.txt");
//y=record;
//plot(x,y);
//x=(1:31);
//loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/recordtest1.txt");
//loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/recordtest2.txt");
//y1=recordtest1;
//y2=recordtest2;

//plot(x,y);

 
 
