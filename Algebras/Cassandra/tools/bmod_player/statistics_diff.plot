set terminal gif
set output 'simulation_diff.gif'
set autoscale
set grid x y
set key left

set ylabel "Updates per second"
set xlabel "Time (Sec)"
set title "BerlinMODPlayer Simulation"

# Uncomment this to get a square image
#set size square

set format x "%10.f"

plot "statistics.txt" index 0:0 using 1:4 with linespoints ti "Read" , \
     "statistics.txt" index 0:0 using 1:5 with linespoints ti "Write" 

set terminal postscript enhanced 
set out '| epstopdf --filter --autorotate=All > simulation_diff.pdf; pdfcrop --margins 10 simulation_diff.pdf > /dev/null'
replot
