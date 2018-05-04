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
set logscale x 2
#set logscale y 10
unset logscale y
set autoscale y

set key top center


set title "average speed of request compute for a server cluster"
set xlabel "number of concurrents clients"
set ylabel "number of requests per secondes"

set output "average_mean.eps"

#average speed
plot 'second_run/means_2_srv' using 1:($1*10000/$2) w lp title "with 2 servers",\
'second_run/means_4_srv' using 1:($1*10000/$2) w lp title "with 4 servers",\
'second_run/means_8_srv' using 1:($1*10000/$2) w lp title "with 8 servers",\
'second_run/means_16_srv' using 1:($1*10000/$2) w lp title "with 16 servers"
