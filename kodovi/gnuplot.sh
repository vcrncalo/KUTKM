#!/usr/bin/env bash

#------------------------------------------------------------------------------------
clear;
alias echo='echo -e';

green="\e[32m";
white="\e[0m";
nums=(2 3 4);
counter=0;

for i in "${nums[@]}"; do
    log_file="TSN${nums[$counter]}_output.log";
    log_file_filtered="TSN${nums[$counter]}_output_filtered.log";
    gnuplot_file="TSN${nums[$counter]}_output.plt";
    main_dir="Time-Sensitive-Networking";
    #------------------------------------------------------------------------------------
    echo "--------------------------------------------------------------------------";
    echo "${green}Promjena direktorija...${white}";

    cd /home/etftk/ns-allinone-3.31/ns-3.31;

    mkdir -p "$main_dir";
    mkdir -p "$main_dir"/images_dir;
    mkdir -p "$main_dir"/gnuplot_dir;
    mkdir -p "$main_dir"/xml_dir;
    mkdir -p "$main_dir"/pcap_dir; 
    mkdir -p "$main_dir"/log_dir; 
    #------------------------------------------------------------------------------------
    echo "${green}Pokretanje TSN${nums[$counter]}.cc skripte. Rezultati su sačuvani u: ${log_file}${white}"

    ./waf --run=TSN"${nums[$counter]}" > "$log_file" 2>&1;

    echo "${green}Modifikovanje $log_file za gnuplot...${white}";

    grep 'Packet.*received.*\ss' TSN"${nums[$counter]}"_output.log | awk -F'[\ ]+' '{printf "%s %s %s %s\n", $2, $6, $10, $14}' | sed 's/://g' > "$log_file_filtered";
    cat "$log_file_filtered"
    echo "";
    #------------------------------------------------------------------------------------
    echo "${green}Kreiranje gnuplot datoteke...${white}";

    cat <<EOF > "$gnuplot_file" 
    set terminal png size 800,600
    set output 'TSN${nums[$counter]}_output.png'

    set title "TSN Packet Delay and Jitter"
    set xlabel "Packet Number"
    set ylabel "Time (s)"
    set grid

    # Plot columns: 1 = packet number, 3 = delay, 4 = jitter
    plot '${log_file_filtered}' using 1:3 with linespoints title 'Delay', \
	 '' using 1:4 with linespoints title 'Jitter'
EOF
    #------------------------------------------------------------------------------------
    echo "${green}Pomjeranje datoteka u $main_dir...${white}"

    mv TSN"${nums[$counter]}"*pcap "$main_dir"/pcap_dir;
    mv TSN"${nums[$counter]}"*xml "$main_dir"/xml_dir;
    mv "$log_file" "$main_dir"/log_dir;
    mv TSN"${nums[$counter]}"*plt "$main_dir"/gnuplot_dir;
    mv "$log_file_filtered" "$main_dir"/gnuplot_dir;

    echo "${green}Promjena direktorija...${white}";

    cd "$main_dir"/gnuplot_dir;

    gnuplot "$gnuplot_file";

    echo "${green}Pomjeranje slika u ../images_dir...${white}";
    mv *png ../images_dir;

    echo "${green}Promjena direktorija...${white}";

    cd ..

    final_dir=$(pwd);

    cd ..;
    ((counter++));
done;
echo "${green}Kreiranje zbirnog throughput grafikona...${white}"

# Zbirni fajl sa throughput-ima svih TSN simulacija
throughput_summary_file="TSN_throughput_summary.dat"

echo "#TSN_Scenario Throughput_bps" > "$throughput_summary_file"

for i in "${nums[@]}"; do
    log_file="Time-Sensitive-Networking/log_dir/TSN${i}_output.log"
    throughput=$(grep "Troughput (bps)" "$log_file" | awk -F':' '{print $2}' | awk '{print $1}')
    echo "TSN${i} $throughput" >> "$throughput_summary_file"
done

# Gnuplot skripta za zbirni throughput graf
cat <<EOF > TSN_throughput_summary.plt
set terminal png size 800,600
set output 'TSN_throughput_summary.png'

set title "Throughput for all 3 TSN cases "
set xlabel "TSN Scenario"
set ylabel "Throughput (bps)"
set style data histograms
set style fill solid
set boxwidth 0.5
set grid

plot '$throughput_summary_file' using 2:xtic(1) title 'Throughput'
EOF

gnuplot TSN_throughput_summary.plt

# Premještanje u odgovarajuće direktorijume
mv "$throughput_summary_file" Time-Sensitive-Networking/gnuplot_dir
mv TSN_throughput_summary.plt Time-Sensitive-Networking/gnuplot_dir
mv TSN_throughput_summary.png Time-Sensitive-Networking/images_dir

echo "--------------------------------------------------------------------------";
echo "Glavni direktorij se nalazi na sljedećoj lokaciji: ${green}$final_dir${white}";
