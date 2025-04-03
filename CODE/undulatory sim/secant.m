function [x,v] = secant(g,xcurr,xnew,uncert);
if nargin < 4
uncert=10^(-5);
if nargin < 3
if nargin == 1
xcurr=0;
xnew=1;
elseif nargin == 0
    g='g';
else
disp(’Cannot have 2 arguments.’);
return;
end
end
end
g_curr=feval(g,xcurr);
while abs(xnew-xcurr)>xcurr*uncert,
xold=xcurr;
xcurr=xnew;
g_old=g_curr;
g_curr=feval(g,xcurr);
xnew=(g_curr*xold-g_old*xcurr)/(g_curr-g_old);
end 
if nargout >= 1
x=xnew;
if nargout == 2
v=feval(g,xnew);
end
else
final_point=xnew
value=feval(g,xnew)
end 

g=@(x) (2x-1)^2+4(4-1024x)^4;
x0=0;
x=1;
epsilon=1e-5;
root=secant(g,x0,x1,epsilon);
disp(root)
disp(g(root));