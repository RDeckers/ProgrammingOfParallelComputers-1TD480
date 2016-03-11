set term term_type
set output output_file
set logscale xy
set key outside;
set key center top;
set format y "10^{%L}"
set format x "10^{%L}"
set ylabel "runtime (ns)"
set xlabel "array size"
set grid;
plot\
 'timings_serial.dat' using 1:2 w lp title 'Serial',\
 'timings_fork.dat' using 1:2 w lp title 'Divide-and-conquer, forking',\
 'timings_tasks.dat' using 1:2 w lp title 'Divide-and-conquer, tasks',\
 'timings_peer_8.dat' using 1:2 w lp title 'Peer algorithm, 8 threads',\
 'timings_peer_16.dat' using 1:2 w lp title 'Peer algorithm, 16 threads'
set output
