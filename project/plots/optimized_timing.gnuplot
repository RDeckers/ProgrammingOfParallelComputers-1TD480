set term term_type
set output output_file
set logscale xy
plot 'data/naive_rng_timing.dat' i 1 using 1:4 w lp title "naive",\
 'data/optimized_timing.dat' using 1:4 w lp title "optimized"
set output
