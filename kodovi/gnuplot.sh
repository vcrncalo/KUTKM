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
echo "--------------------------------------------------------------------------";
echo "Glavni direktorij se nalazi na sljedećoj lokaciji: ${green}$final_dir${white}";
