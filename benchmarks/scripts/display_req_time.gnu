set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color

# ARG1 is the path and ARG2 is the type of run (sh, dh, indedh)

ymin= 1e-4
ymax= 1.5e-4
set yrange [ymin:ymax]

#rebalancing moments only on dh
if (ARG2 eq "dh") {
    set for [i=0:1470:60] arrow from i,ymin to i,ymax nohead lc rgb "violet" lt 3
}

set xtics nomirror
set ytics nomirror

set key box opaque top center

set title "mean time for a request to be processed"
set xlabel "time (in minutes)"
set ylabel "processing time for a request (in seconds)"

set output ARG1."/request_time.eps"


if (ARG2 eq "dh") {
    plot ARG1.'/mean_time.csv' using ($0*5):1 w lp lw 2 title "processing time",\
    NaN title "Rebalancing" lt 3 lc rgb "violet"
}
if (ARG2 eq "sh") {
    plot ARG1.'/mean_time.csv' using ($0*5):1 w lp lw 2 title "processing time"
}
if (ARG2 eq "indedh" || ARG2 eq "windowed") {
    plot ARG1.'/mean_time.csv' using ($0*5):1 w lp lw 2 title "processing time",\
    ARG1.'/server/rebalancing' using ($1*5):(ymax):1 w impulse title "Rebalancing" lt 3 lc rgb "violet"
}
