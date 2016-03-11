set term term_type
set output output_file
set logscale x
set key outside;
set key center top;
set format x "10^{%L}"
set ylabel "runtime (ns)"
set xlabel "array size"
set grid;
plot\
 '< paste timings_serial.dat timings_fork.dat' using 1:($2/$4) w lp title 'Divide-and-conquer, forking',\
 '< paste timings_serial.dat timings_tasks.dat' using 1:($2/$4) w lp title 'Divide-and-conquer, tasks',\
 '< paste timings_serial.dat timings_peer_8.dat' using 1:($2/$4) w lp title 'peer, 8 threads',\
 '< paste timings_serial.dat timings_peer_16.dat' using 1:($2/$4) w lp title 'peer, 16 threads'
set output
