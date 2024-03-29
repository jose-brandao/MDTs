# Note you need gnuplot 4.4 for the pdfcairo terminal.

# set terminal pdfcairo font "Gill Sans,9" linewidth 4 rounded fontscale 1.0
set terminal pdfcairo font "Gill Sans,7" linewidth 1 rounded fontscale 0.9

# Line style for axes
set style line 80 lt rgb "#595959"

# Line style for grid
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#595959"  # grey
#set ytics 4

#set logscale x 50
#set logscale y 2

set grid back linestyle 81
set border 3 back linestyle 80 # Remove border on top and right.  These

# Line styles: try to pick pleasing colors, rather
# than strictly primary colors or hard-to-see colors

# like gnuplot's default yellow.  Make the lines thick
# so they're easy to see in small plots in papers.
set style line 1 lc rgb "#A00000" pt 7
set style line 2 lc rgb "#00A000" pt 9
set style line 3 lc rgb "#5060D0" pt 5
set style line 4 lc rgb "#939393" pt 18
set style line 5 lc rgb "#DAFF22" pt 11
set style line 6 lc rgb "#F05900" pt 13

# Let gnuplot decide the color, if more than 6 lines
set style line 7 pt 15
set style line 8 pt 8

set pointsize 0.7

set output outputname
set ylabel "ops/sec"
set xlabel "threads"

set key right top

set yrange [0:8000000]
set xrange [*:36]

set ylabel font ",5"
set xlabel font ",5"
set xtics font ",5" 
set ytics font ",5" 

set key left top
set key font ",4"


# Titles with spaces are not allowed
# These titles should be separated by "_" and here we replace by " "
pretty(title) = system("echo ".title." | sed 's/_/ /g'")

# Input file contains comma-separated values fields
set datafile separator ","

plot for [i=0:words(inputnames) - 1] word(inputnames, i + 1) using 1:2 with linespoint ls i + 1 title pretty(word(titles, i + 1))
