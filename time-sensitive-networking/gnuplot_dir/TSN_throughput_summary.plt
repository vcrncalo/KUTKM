set terminal png size 800,600
set output 'TSN_throughput_summary.png'

set title "Throughput for all 3 TSN cases "
set xlabel "TSN Scenario"
set ylabel "Throughput (bps)"
set style data histograms
set style fill solid
set boxwidth 0.5
set grid

plot 'TSN_throughput_summary.dat' using 2:xtic(1) title 'Throughput'
