    set terminal png size 800,600
    set output 'TSN2_output.png'

    set title "TSN Packet Delay and Jitter"
    set xlabel "Packet Number"
    set ylabel "Time (s)"
    set grid

    # Plot columns: 1 = packet number, 3 = delay, 4 = jitter
    plot 'TSN2_output_filtered.log' using 1:3 with linespoints title 'Delay', 	 '' using 1:4 with linespoints title 'Jitter'
