set term png;
set output 'pingpong.png';
a=1;
b=1;
f(x) = a+b*x;
fit f(x) 'out.dat' using 1:2 via a,b;
ti = sprintf("Latency: %.3f us, Bandwith: %.3f MB/s", a*10**6, (1/b/10**6))
fit_ti = sprintf("%e+%ex", a, b);
set title ti;
plot 'out.dat' using 1:2 w p title 'data', f(x) title fit_ti;
unset output;
