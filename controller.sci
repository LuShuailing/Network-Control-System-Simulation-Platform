function [y1]=Controller(u1)
    //输入控制器A
    A=[0;0];
    
    
    y1=[u1(1,1),[u1(2,1),u1(3,1)]*A];
    
    
    
    
    
endfunction 
