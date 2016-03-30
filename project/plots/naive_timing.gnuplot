set term term_type
set output output_file
set logscale xy
plot 'data/naive_copy_timing.dat' i 0 using 1:4 w lp title "RNG only",\
 '' i 1 using 1:4 w lp title "full simulation"
set output
