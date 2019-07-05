# Note you need gnuplot 4.4 for the pdfcairo terminal.

# set terminal pdfcairo font "Gill Sans,9" linewidth 4 rounded fontscale 1.0
set terminal pdfcairo font "Gill Sans,7" linewidth 1 rounded fontscale 1.0

# Line style for axes
set style line 80 lt rgb "#808080"

# Line style for grid
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey

#set logscale x 50
#set logscale y 1.1

set grid back linestyle 81
set border 3 back linestyle 80 # Remove border on top and right.  These

# Line styles: try to pick pleasing colors, rather
# than strictly primary colors or hard-to-see colors

# like gnuplot's default yellow.  Make the lines thick
# so they're easy to see in small plots in papers.
set style line 1 lc rgb "#A00000" pt 7
set style line 2 lc rgb "#00A000" pt 9
set style line 3 lc rgb "#5060D0" pt 5
set style line 4 lc rgb "#F25900" pt 18
# Let gnuplot decide the color, if more than 4 lines
set style line 5 pt 7
set style line 6 pt 7
set style line 7 pt 7
set style line 8 pt 7

set output outputname
set xlabel "{/*1.3 thread number}"
set ylabel "{/*1.3 time s}"

set key left top

set xtics (1, 2, 4, 8, 12, 16, 20, 24, 28, 32)
#set xtic auto
set xrange [*:*]    
set yrange [*:*]


# Titles with spaces are not allowed
# These titles should be separated by "_" and here we replace by " "
pretty(title) = system("echo ".title." | sed 's/_/ /g'")

# Input file contains comma-separated values fields
set datafile separator ","

plot for [i=0:words(inputnames) - 1] word(inputnames, i + 1) using 1:2 with linespoint ls i + 1 title pretty(word(titles, i + 1))
replot for [i=0:words(inputnames) - 1] word(inputnames, i + 1) using 1:2 with labels offset char 1,0 notitle
