set term term_type
set output output_file
set logscale x 2
set logscale y 10
set format x '2^{%L}'
set xlabel "System Size"
set ylabel "FLIPS (FLIps Per Second)"
set key outside below
set grid ytics lc rgb "#bbbbbb" lw 1 lt 0
set grid xtics lc rgb "#bbbbbb" lw 1 lt 0
plot 'data/timing.dat' i 0 using 1:(10**9*($1**2)/$4) w lp title "naive host side",\
 'data/timing.dat' i 1 using 1:(10**9*($1**2)/$4) w lp title "naive LCG",\
 'data/timing.dat' i 2 using 1:(10**9*($1**2)/$4) w lp title "optimized,k = 1",\
 'data/timing.dat' i 3 using 1:(10**9*($1**2)/$4) w lp title "optimized,k = 4",\
 'data/timing.dat' i 4 using 1:(10**9*($1**2)/$4) w lp title "optimized,k = 8"
set output
