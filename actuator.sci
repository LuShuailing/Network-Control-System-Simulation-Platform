function [y1,y2]=Actuator(u1,u2)

if u1(2,1)<u2(2,1) then
    
    y1=[u2(3,1)];
    y2=u2;
else
    y1=[u1(3,1)];
    y2=u1;
end
    
    
        
        
        
   
    
endfunction   
