set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#33CCFF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps

set output "max_speed.eps"

set xtics nomirror
set ytics nomirror
set logscale x 2
#set logscale y 2
unset logscale y
set autoscale y

set key top center


set title "max speed of request compute for a server cluster"
set xlabel "number of servers in the cluster"
set ylabel "number of requests per secondes"

#average speed
plot 'second_run/min' using 1:($2*10000/$3) w lp notitle

