set linetype 1 lc rgb "#009900"  # "#555555"
set linetype 2 lc rgb "#FFCC33"  # "#CC1122"
set linetype 3 lc rgb "#9900CC"  # "#000099"
set linetype 4 lc rgb "#3377FF"  # "#FF8C00"
set linetype 5 lc rgb "#FF0099"  # "#BB00BB"
set linetype 6 lc rgb "#555555"  # "#009922"

set datafile separator ";"
set terminal postscript eps color

# ARG1 is the path of the av_err.txt and max_err.txt files
# ARG2 is the type of run

set xtics nomirror
set ytics nomirror

#y2 axis is for indedh only
if (ARG2 eq "indedh") {
    set y2range [0:300]
    set y2tics 50
    set y2label "number of rebalancing"
}

set key box opaque top center

set ylabel "error (in percentage of load)"
set style data histograms
set style fill solid


set output ARG1."/error_param.eps"
set xlabel "values of parameters"
set title "load repartition error depending of all parameter combinaisons"


#alpha value
#set output ARG1."/error_alpha.eps"
#set xlabel "value of alpha"
#set title "load repartition error depending of the alpha parameter"

#n_entry value
#set title "load repartition error depending of the number of entries in the MLT"
#set output ARG1."/error_n_entry.eps"
#set xlabel "Number of entries in the MLT"

#percent value
#set output ARG1."/error_percent.eps"
#set title " Load repartition error depending of the threshold variation use in load evaluation"
#set xlabel "percentage variation of threshold in load evaluation"

if (ARG2 eq "indedh") {
    plot ARG1.'/av_err_all.txt' using 2:xtic(1) lw 2 title "average error" axis x1y1,\
    ARG1.'/max_err_all.txt' using 2:xtic(1) lw 2 title "max error" axis x1y1,\
    ARG1.'/nb_rebalancing.txt' using 2:xtic(1) lw 2 title "number rebalancing" axis x1y2
}
else {
    plot ARG1.'/av_err_all.txt' using 2:xtic(1) lw 2 title "average error" axis x1y1,\
    ARG1.'/max_err_all.txt' using 2:xtic(1) lw 2 title "max error" axis x1y1
}
