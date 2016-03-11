set term term_type;
set output output_file;
unset key
set xlabel "iteration"
set ylabel "omega"
set logscale x
set border 895
#set view 65,220, 1.0
load'plots/parula.pal'
set view map
set contour base
set cntrlabel onecolor
set xrange [1:4096]
set yrange [1:2]
set cntrparam levels incr -3,0.25,3
set cntrlabel font ",7"
splot 'data/omega_field.dat' u 1:2:(log($3)) w pm3d lc 'white'


unset output;
