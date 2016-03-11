set term term_type;
set output output_file;
set grid x y2
set logscale xy
set log x2
unset log y2
set ytics nomirror
set y2tics
set ylabel "Running time"
set y2label "Speedup"
set y2range [1:4]
set xrange [8:256]
set key right bottom title " "
plot\
 'data/optimizations.dat' u 1:2 w lp axes x1y1 title "Unoptimized running time",\
 '' u 1:3 w lp axes x1y1 title "Optimized running time",\
 '' u 1:4 w l axes x1y2 title "Speedup";
unset output;
