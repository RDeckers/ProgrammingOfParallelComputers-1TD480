set term term_type;
set output output_file;

set logscale xy;
plot for [IDX=0:6] 'data/varying_omega.dat' i IDX u 1:2 w lines title columnheader(1)
unset output;
