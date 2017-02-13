clear all
instrreset

a = c2000Link(115200, 10);

% Load signals and play lists libraries
% (I send them after wards) 
load firstLib;


for i = 1:10
    a.sendSignal(i, signalArray(i,:));
end

a.sendPlayList([1000, 2, 1000, 2, 1400, 6], 1);
%a.prueba;
%a.plotSignals
%figure
a.playAndSaveFeedback(1);
a.start;
pause(2);
a.stop;
%%
x = zeros(1, 2*800);
for i = 1:800
   x((2*i-1):(2*i)) = [10000, 2];
   
   if i == 25
       x((2*i-1):(2*i)) = [1000000, 2];
   end
   
   if i == 100
       x((2*i-1):(2*i)) = [30000, 4];
   end
   
   if i == 600
       x((2*i-1):(2*i)) = [100000, 6];
   end
   
   if i > 600
       x((2*i-1):(2*i)) = [50000, 6];
   end
   
end 


%%
x = zeros(1, 2*63);
for i = 1:63
   if i < 10
       x((2*i-1):(2*i)) = [2000, 2];
   end
   
   if i == 10
       x((2*i-1):(2*i)) = [10000, 6];
   end
   
   if (i > 10)&&(i < 40)
       x((2*i-1):(2*i)) = [6000, 6];
   end
   
   if i >= 40
       x((2*i-1):(2*i)) = [5000, 6];
   end
   
end 

%%
y = [0, random('Normal', 17, 0.7, 1, 10), 15, random('Normal', 25, 0.7, 1, 10), 0];
x = linspace(1, 200, 23);
bar(x, y)
axis([0 200 0 30])
xlabel('tiempo [µs]')
ylabel('Señal de estimulación [V]')

%%
y2 = [zeros(1, 50), 17, 25, zeros(1, 100), 15, 23, zeros(1, 100), 15, 25, zeros(1,100), 17, 28, zeros(1,100), 14, 20, zeros(1,20)];
x2 = linspace(0, 10, length(y2));

bar(x2, y2)
axis([0 10 0 30])
xlabel('tiempo [ms]')
ylabel('Señal de estimulación [V]')







