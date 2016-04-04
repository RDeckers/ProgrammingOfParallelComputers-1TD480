set term term_type
set output output_file
set xlabel "J"
set ylabel  "dE/dJ"
set key below
plot 'data/MvJ.dat' i 1 using 2:3 w lp title "host side rng",\
 '' i 0 using 2:3 w lp title "device LCG",\
 '' i 2 using 2:3 w lp title "optimized, k = 1",\
 '' i 3 using 2:3 w lp title "optimized, k = 2",\
 '' i 4 using 2:3 w lp title "optimized, k = 4",\
 '' i 5 using 2:3 w lp title "optimized, k = 8"
set output
