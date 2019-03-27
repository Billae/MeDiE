set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color

ymin= 0
ymax= 5e6
set yrange [ymin:ymax]
#set for [i=60:1470:60] arrow from i,ymin to i,ymax nohead lc rgb "violet" lt 3


set xtics nomirror
set ytics nomirror

set key box opaque top center

set title "workload of each server"
set xlabel "time (in minutes)"
set ylabel "number of processed requests"

set output "~/average_load.eps"


plot '~/mesure_perf/server/load0' using ($0*15):1 w lp lw 2 title "server 0",\
'~/mesure_perf/server/load1'using ($0*15):1 w lp lw 2 title "server 1",\
'~/mesure_perf/server/load2'using ($0*15):1 w lp lw 2 title "server 2",\
'~/mesure_perf/server/load3'using ($0*15):1 w lp lw 2 title "server 3",\
'~/mesure_perf/server/rebalancing' using ($1*15):(ymax) title "Rebalancing" lt 3 lc rgb "violet"
