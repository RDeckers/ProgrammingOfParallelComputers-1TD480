set term term_type;
set output output_file;
unset key
set xlabel "i"
set ylabel "j"
set border 895
set view 65,220, 1.0
load'plots/parula.pal'
splot 'data/compute_result.dat' i 0 matrix w pm3d lc 'white'


unset output;
