set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color

set xtics nomirror
set ytics nomirror
set logscale xy 2
#set logscale y 10
#unset logscale y
#set autoscale y
#set ytics add ("5000" 5000)

set key top center

set title "average system throughput"
set xlabel "number of concurrent clients"
set ylabel "number of requests per second"

set output "average_mean.eps"

f(x)=x*10000

#average speed (without 2 servers = too much)
plot 'means_1_srv' using 1:($1*10000/$2) w lp lw 2 title "with 1 server",\
'means_2_srv' using 1:($1*10000/$2) w lp lw 2title "with 2 servers",\
'means_4_srv' using 1:($1*10000/$2) w lp lw 2title "with 4 servers",\
'means_8_srv' using 1:($1*10000/$2) w lp lw 2 title "with 8 servers",\
'means_16_srv' using 1:($1*10000/$2) w lp lw 2 title "with 16 servers",\
f(x) title "perfect scaling"
