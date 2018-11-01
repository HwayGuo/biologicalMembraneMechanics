close all;
Z22=Z2{:,:};
Z33=Z3{:,:};
Z44=Z4{:,:};
figure; hold on;
plot(Z22(1:95,1),Z22(1:95,2), 'LineWidth',2); 
plot(Z33(1:95,1),Z33(1:95,2),'LineWidth',2); 
plot(Z44(1:95,1),Z44(1:95,2),'LineWidth',2);
legend('Z=2R','Z=3R','Z=4R');
title('R (pN)'); set(gca,'FontSize',14);
figure; hold on;
plot(Z22(1:95,1),Z22(1:95,3),'LineWidth',2); 
plot(Z33(1:95,1),Z33(1:95,3),'LineWidth',2); 
plot(Z44(1:95,1),Z44(1:95,3),'LineWidth',2); 
legend('Z=2R','Z=3R','Z=4R');
title('E1 (pN-nm)'); set(gca,'FontSize',14);
%write to mat file
mFile=[];
mfile(:,1)=Z22(1:95,1);
mfile(:,2)=Z22(1:95,2);
mfile(:,3)=Z33(1:95,2);
mfile(:,4)=Z44(1:95,2);
mfile(:,5)=Z22(1:95,3);
mfile(:,6)=Z33(1:95,3);
mfile(:,7)=Z44(1:95,3);
csvwrite('tubeBVP_R_E1.txt',mfile);
save('tubeBVP_R_E1.mat','mfile');