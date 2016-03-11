set term term_type;
set output output_file;
unset key
set view map scale 1
set xrange [ -0.500000 : 11.50000 ] noreverse nowriteback
set yrange [ -0.500000 : 11.50000 ] noreverse nowriteback
set xtics 0,1
set ytics 0,1
set mxtics 2
set mytics 2
set grid mxtics front linetype -1
set grid mytics front linetype -1

set palette rgbformulae -7, 2, -7
unset colorbox
plot 'data/task_01.dat' matrix using 1:2:3 with image, \
     '' matrix using 1:2:( sprintf("%g",$3) ) with labels
unset output;
