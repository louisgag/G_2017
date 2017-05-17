clear; close all;

M = dlmread('./module-gsoc-puneet-mahajan.mov');
x = M(:,2); y = M(:,3); z = M(:,4);
xv = M(:,8) ; yv = M(:,9); zv = M(:,10);
t = 0:1.e-3:5;

plot(t,xv,'b','LineWidth',2,'MarkerSize',2,'DisplayName','Velocity');
hold on
plot(t,x,'r','LineWidth',2,'MarkerSize',1,'DisplayName','Distance');
legend('show')
grid on;
xlabel('time (m)','FontSize',14);