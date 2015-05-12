set terminal gif
set output 'simulation.gif'
set autoscale
set grid x y
set key left

set ylabel "Updates"
set xlabel "Time (Sec)"
set title "BerlinMODPlayer simulation"

# Uncomment this to get a square image
#set size square

set format x "%10.f"

plot "statistics.txt" index 0:0 using 1:2 with linespoints ti "Read" , \
     "statistics.txt" index 0:0 using 1:3 with linespoints ti "Write" 

set terminal postscript enhanced 
set out '| epstopdf --filter --autorotate=All > simulation.pdf; pdfcrop --margins 10 simulation.pdf > /dev/null'
replot
