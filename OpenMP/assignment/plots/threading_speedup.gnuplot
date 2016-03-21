set term term_type;
set output output_file;
set logscale x;

set grid xtics front linetype -1
set grid ytics front linetype -1
first(x) = ($0 > 0 ? base : base = x)
set xlabel "Number of max threads"
set ylabel "Speedup"
plot 'data/threading_speedup.dat' u 1:(first($2), base/$2) w lp notitle
unset output
