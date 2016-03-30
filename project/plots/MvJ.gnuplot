set term term_type
set output output_file
plot 'data/MvJ.dat' i 0 using 2:3 w lp title "java rng",\
 '' i 1 using 2:3 w lp title "host side rng"
set output
