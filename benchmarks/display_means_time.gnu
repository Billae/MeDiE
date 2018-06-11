set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#33CCFF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"

set terminal postscript eps color

set xtics nomirror
set ytics nomirror
set logscale xy 2
#unset logscale y

set key top center

#mean time
set title "mean time for one client to perform 10 000 puts"
set xlabel "number of concurrents clients"
set ylabel "time in seconds"

set output "mean_time.eps"

#mean time
plot 'second_run/means_1_srv' w lp title "with 1 servers",\
'second_run/means_2_srv' w lp title "with 2 servers",\
'second_run/means_4_srv' w lp title "with 4 servers",\
'second_run/means_8_srv' w lp title "with 8 servers",\
'second_run/means_16_srv' w lp title "with 16 servers"
