set term term_type
set output output_file
set logscale xy
plot 'data/naive_rng_timing.dat' i 0 using 1:4 w lp title "RNG only",\
 '' i 1 using 1:4 w lp title "full simulation"
set output
