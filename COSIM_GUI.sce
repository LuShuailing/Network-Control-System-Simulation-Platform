// This GUI file is generated by guibuilder version 4.2.1


// 作者：卢帅领
//日期：2021.04.13

//打开SCILAB工作区间
cd /home/ling/Scilab/scilab-6.1.0/client

//////////
f=figure('figure_position',[466,27],'figure_size',[990,887],'auto_resize','on','background',[33],'figure_name','图像窗口%d','dockable','off','infobar_visible','off','toolbar_visible','off','menubar_visible','off','default_axes','on','visible','off');
//////////
handles.dummy = 0;
handles.NS3_initial=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.0807692,0.3171174,0.2248718,0.1654804],'Relief','default','SliderStep',[0.01,0.1],'String','NS3_initial','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','NS3_initial','Callback','NS3_initial_callback(handles)')
handles.SCILAB_initial=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.4158974,0.3171174,0.2248718,0.1654804],'Relief','default','SliderStep',[0.01,0.1],'String','SCILAB_initial','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','SCILAB_initial','Callback','SCILAB_initial_callback(handles)')
handles.MODULE_set=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.7617949,0.3171174,0.2148718,0.1654804],'Relief','default','SliderStep',[0.01,0.1],'String','MODULE_set','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','MODULE_set','Callback','MODULE_set_callback(handles)')
handles.START=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.2641026,0.0125979,0.5679487,0.1535231],'Relief','default','SliderStep',[0.01,0.1],'String','START','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','START','Callback','START_callback(handles)')
handles.Delay_plot=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.2175682,0.510263,0.2494306,0.0658453],'Relief','default','SliderStep',[0.01,0.1],'String','Delay_plot','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','Delay_plot','Callback','Delay_plot_callback(handles)')
handles.GUI_name=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','left','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.3538384,0.9606701,0.3181818,0.0480412],'Relief','default','SliderStep',[0.01,0.1],'String','                   SCILAB_NS3_COSIMULATION_GUI','Style','text','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','GUI_name','Callback','')
handles.plot_clear=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.8487879,0.5035294,0.0807071,0.0841176],'Relief','default','SliderStep',[0.01,0.1],'String','Clear_plot','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','plot_clear','Callback','plot_clear_callback(handles)')
handles.controller_set=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.329899,0.2111765,0.1094949,0.0635294],'Relief','default','SliderStep',[0.01,0.1],'String','Controller_set','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','controller_set','Callback','controller_set_callback(handles)')
handles.plant_set=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.4929293,0.2111765,0.1094949,0.0635294],'Relief','default','SliderStep',[0.01,0.1],'String','Plant_set','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','plant_set','Callback','plant_set_callback(handles)')
handles.actuator_set=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.6809091,0.2111765,0.1094949,0.0635294],'Relief','default','SliderStep',[0.01,0.1],'String','Actuator_set','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','actuator_set','Callback','actuator_set_callback(handles)')
handles.out_plot=uicontrol(f,'unit','normalized','BackgroundColor',[-1,-1,-1],'Enable','on','FontAngle','normal','FontName','Ubuntu','FontSize',[12],'FontUnits','points','FontWeight','normal','ForegroundColor',[-1,-1,-1],'HorizontalAlignment','center','ListboxTop',[],'Max',[1],'Min',[0],'Position',[0.5320202,0.510263,0.2494306,0.0658453],'Relief','default','SliderStep',[0.01,0.1],'String','Out_plot','Style','pushbutton','Value',[0],'VerticalAlignment','middle','Visible','on','Tag','out_plot','Callback','out_plot_callback(handles)')
handles.out_plot_axe= newaxes();handles.out_plot_axe.margins = [ 0 0 0 0];handles.out_plot_axe.axes_bounds = [0.1922222,0.0494118,0.6139394,0.3352941];


f.visible = "on";


//////////
// Callbacks are defined as below. Please do not delete the comments as it will be used in coming version
//////////









function NS3_initial_callback(handles)
//Write your callback for  NS3_initial  here
exec('/home/ling/Scilab/scilab-6.1.0/client/StartNS3.sce', -1)
endfunction


function SCILAB_initial_callback(handles)
//Write your callback for  SCILAB_initial  here
exec('/home/ling/Scilab/scilab-6.1.0/client/testmain.sce', -1)
endfunction


function MODULE_set_callback(handles)
//Write your callback for  MODULE_set  here
xcos('/home/ling/Scilab/scilab-6.1.0/client/daxingtest01.zcos')
endfunction


function START_callback(handles)
//Write your callback for  START  here
exec('/home/ling/Scilab/scilab-6.1.0/client/firststep.sci', -1)
endfunction


function Delay_plot_callback(handles)
//Write your callback for  Delay_plot  here
x=(1:100);
loadmatfile("/home/ling/Scilab/scilab-6.1.0/client/record.txt");
y=record;
plot(x,y);
a=gca();
a.data_bounds=[1 0.0025;100 0.008];

endfunction


function plot_clear_callback(handles)
//Write your callback for  plot_clear  here
delete(handles.out_plot_axe.children);
endfunction


function controller_set_callback(handles)
//Write your callback for  controller_set  here
scinotes('/home/ling/Scilab/scilab-6.1.0/client/controller.sci')
endfunction


function plant_set_callback(handles)
//Write your callback for  plant_set  here
scinotes('/home/ling/Scilab/scilab-6.1.0/client/Plant.sci')
endfunction


function actuator_set_callback(handles)
//Write your callback for  actuator_set  here
scinotes('/home/ling/Scilab/scilab-6.1.0/client/actuator.sci')
endfunction


function out_plot_callback(handles)
//Write your callback for  out_plot  here
D = fscanfMat('outrecord.txt');

d=(1:100);
plot(d,D);
endfunction

