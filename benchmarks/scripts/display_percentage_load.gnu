set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color


# ARG1 is the path and ARG2 is the type of run (sh, dh, indedh)

ymin= 20
ymax= 30
set yrange [ymin:ymax]

#rebalancing moment only on dh
#if (ARG2 eq "dh") {
#    set for [i=0:1470:60] arrow from i,ymin to i,ymax nohead lc rgb "violet" lt 3
#}

set xtics nomirror
set ytics nomirror

set key box opaque top right


set title "workload of each server"
set xlabel "time (in minutes)"
set ylabel "percentage of received requests"

set output ARG1."/load_%.eps"


#if (ARG2 eq "dh") {
#    plot ARG1.'/percentages.csv' using ($0*5):1 w lp lw 2 title "server 0",\
#    ARG1.'/percentages.csv' using ($0*5):2 w lp lw 2 title "server 1",\
#    ARG1.'/percentages.csv' using ($0*5):3 w lp lw 2 title "server 2",\
#    ARG1.'/percentages.csv' using ($0*5):4 w lp lw 2 title "server 3",\
#    NaN title "Rebalancing" lt 3 lc rgb "violet"
#}
if (ARG2 eq "sh") {
    plot ARG1.'/percentages.csv' using ($0*5):1 w lp lw 2 title "server 0",\
    ARG1.'/percentages.csv' using ($0*5):2 w lp lw 2 title "server 1",\
    ARG1.'/percentages.csv' using ($0*5):3 w lp lw 2 title "server 2",\
    ARG1.'/percentages.csv' using ($0*5):4 w lp lw 2 title "server 3"
}
if (ARG2 eq "indedh" || ARG2 eq "windowed" || ARG2 eq "dh") {
    plot ARG1.'/percentages.csv' using ($0*5):1 w lp lw 2 title "server 0",\
    ARG1.'/percentages.csv' using ($0*5):2 w lp lw 2 title "server 1",\
    ARG1.'/percentages.csv' using ($0*5):3 w lp lw 2 title "server 2",\
    ARG1.'/percentages.csv' using ($0*5):4 w lp lw 2 title "server 3",\
    ARG1.'/useful_rebalancing.txt' using ($1*5):(ymax):1 w impulse title " useful rebalancing" lt 3 lc rgb "green",\
    ARG1.'/useless_rebalancing.txt' using ($1*5):(ymax):1 w impulse title "useless rebalancing" lt 3 lc rgb "red"
}
