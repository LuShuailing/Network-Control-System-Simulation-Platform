function [y,z]=plant(x,u)
    //输入被控对象
    A=[0.9,0;0,0.8];
    B=[1;0.5];
    C=[1,0;0,1];
    y=A*x+B*u;
    z=C*x;
    
    
    
    
endfunction  
