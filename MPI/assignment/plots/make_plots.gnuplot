set term latex;
set output "plot.tex";
at(file, row, col) = system( sprintf("awk -v row=%d -v col=%d 'NR == row {print $col}' %s", row, col, file) )
row=1;col=3;
set yrange [0:32]
set grid x2tics
set x2tics 16 format "" scale 0
plot x title "Ideal",\
 '../data/timings_3_120.dat' u 2:(at("../data/timings_3_120.dat",row,col)/$3) w lp title "N = 120",\
 '../data/timings_3_480.dat' u 2:(at("../data/timings_3_480.dat",row,col)/$3) w lp title "N = 480",\
 '../data/timings_3_840.dat' u 2:(at("../data/timings_3_840.dat",row,col)/$3) w lp title "N = 840",\
 '../data/timings_3_1200.dat' u 2:(at("../data/timings_3_1200.dat",row,col)/$3) w lp title "N = 1200"
unset output
