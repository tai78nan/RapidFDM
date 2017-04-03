function obj = make_lag_obj(f1,a)
%a>1 lead
%a<1 lag
wz = f1 * 2*pi*a;
wp = 2*pi*f1;
b =  [1,-((400.*wp - wp*wz)/(400.*wp + wp*wz))]*wp*(400. + wz)/((400. + wp)*wz);
a = [1,-((400.*wz - wp*wz)/(400.*wz + wp*wz))];
obj = struct('A',a,'B',b,'x',[0,0],'y',[0,0],'inited',false);
end