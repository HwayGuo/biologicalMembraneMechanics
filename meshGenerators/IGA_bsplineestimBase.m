% B-spline curve estimation (control points estimation) from data about points on curve.
% Author : rudraa (Implemented using the bspline library by Levente Hunyadi)
% https://www.mathworks.com/matlabcentral/fileexchange/27374-b-splines
%
clear all; 
% spline order
k = 3; %Order in IGa notation is one less than the notation here. So k=3 is a 2nd order curve.
% knot sequence
%t = [0 0 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1 1 1 ];
numKnots=80;
t = [0 0 linspace(0,1,numKnots/2) 1 1 ]; %k repetitions of 0 and 1 at the ends of the knot vector
%
R=20.0;
r=0.5*R;
%
%populate M (2XN points on the curve... where N is some large number that is sufficient for estimating the control points)
%tube
%theta=linspace(4*R,r,4*numKnots); 

%arc 
theta=linspace(pi/4,0,ceil(0.6*numKnots)); theta=theta(2:end);
M=[R+r-r*cos(theta); r-r*sin(theta)];

theta=linspace(r,2*R,numKnots); 
M=[M(1,:) R*ones(size(theta));M(2,:) theta];

%shift
M(2,:)=M(2,:)-min(M(2,:));

%line
%theta=linspace(R+r,1.5*R,ceil(numKnots)); theta=theta(2:end);
%M=[M(1,:) theta; M(2,:) R*zeros(size(theta))];

D = bspline_estimate(k,t,M);
C = bspline_deboor(k,t,D);

% plot control points and spline
figure;
hold all;
plot(M(1,:), M(2,:), 'k');
plot(D(1,:), D(2,:), 'rx');
plot(C(1,:), C(2,:), 'c');
legend('data', 'estimated control points', 'estimated curve', ...
    'Location', 'Best');
hold off;
axis equal;
%save to file
order=k-1;
knots=t;
controlPoints=D;
save(['base45Deg' num2str(numKnots) '.mat'],'order','knots','controlPoints','-v6') %version 6 format needed to read into python using scipy.io.loadmat
