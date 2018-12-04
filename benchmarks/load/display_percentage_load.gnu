set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color

ymin= 0
ymax= 100
set yrange [ymin:ymax]
#set for [i=0:120:60] arrow from i,ymin to i,ymax nohead lc rgb "violet" lt 3
#set for [i=60:120:60] label "rebalancing" at i-1,5 rotate textcolor rgb "violet"


set xtics nomirror
set ytics nomirror

set key top center

set title "workload of each server"
set xlabel "time (in minutes)"
set ylabel "percentage of processed requests"

set output "load_%.eps"


plot 'results.csv' using ($0*15):1 w lp lw 2 title "server 0",\
'results.csv' using ($0*15):2 w lp lw 2 title "server 1"
