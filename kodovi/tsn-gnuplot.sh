#!/usr/bin/env bash

# Defining colors
red="\e[31m";
green="\e[32m";
magenta="\e[35m";
white="\e[0m";

# Defining directories
ns_dir="/home/etftk/ns-allinone-3.31/ns-3.31"
fin_dir="$ns_dir/tsn-glavni_direktorij";
results_dir="$fin_dir/tsn-rezultati";
pcap_dir="$fin_dir/pcap_datoteke";
gnuplot_dir="$fin_dir/gnuplot_grafici";
gnuplot_dir_plt="$gnuplot_dir/plt";
gnuplot_dir_png="$gnuplot_dir/png";
xml_dir="$fin_dir/xml_direktorij";

# Defining GnuPlot function
function gnuplot_calc(){
	local delay="$1";
	local jitter="$2";
	local output="$3";
	local packet_size="$4";
	local plt="$5";
	local png="$6";
	local gnuplot_dir="$7";
	
	echo -e "${magenta}$delay${white}";	
	echo -e "${magenta}$jitter${white}";	

	echo -e "\nKreiranje GnuPlot grafika...";

	cat << EOF > "$delay"
	set terminal png
	set output "${delay%.plt}.png"
	set title "End-to-End Delay per Packet - Packet size: $packet_size B"
	set xlabel "Packet Sequence Number"
	set ylabel "Delay (seconds)"
	set grid
	plot "$output" using 1:3 with linespoints title "Delay"
	replot
EOF
	cat << EOF > "$jitter"
	set terminal png
	set output "${jitter%.plt}.png"
	set title "Packet Delay Variation (Jitter) - Packet size:$packet_size B"
	set xlabel "Packet Sequence Number"
	set ylabel "Jitter (seconds)"
	set grid
	plot "$output" using 1:4 with linespoints lt rgb "red" title "Jitter"
	replot
EOF
	gnuplot "$delay" 
	gnuplot "$jitter" 

	mv "$gnuplot_dir/"*.png "$png";
	mv "$gnuplot_dir/"*.plt "$plt";
}

# Initializing the code
clear;
echo "--------------------------------------------------";
echo -e "Inicijalizacija skripte...\n";
echo -e "Kreiranje direktorija u kojem će se nalaziti sve relevantne TSN datoteke: ${magenta}$fin_dir${white}\n";

# Checking if directories exist
if [[ -e "$fin_dir" ]]; then	
	echo -e "Direktorij $fin_dir već ${green}postoji${white}.";
else
	mkdir "$fin_dir";
	echo "Direktorij $fin_dir kreiran.";
fi;

# Creating directories
mkdir "$results_dir";
echo "Direktorij $results_dir kreiran.";
mkdir "$pcap_dir";
echo "Direktorij $pcap_dir kreiran.";
mkdir "$gnuplot_dir";
echo "Direktorij $gnuplot_dir kreiran.";
mkdir "$gnuplot_dir";
echo "Direktorij $gnuplot_dir kreiran.";
mkdir "$gnuplot_dir_png";
echo "Direktorij $gnuplot_dir_png kreiran.";
mkdir "$gnuplot_dir_plt";
echo "Direktorij $gnuplot_dir_plt kreiran.";
mkdir "$xml_dir";
echo "Direktorij $xml_dir kreiran.";

echo -e "\nProvjera da li direktorij ${magenta}$ns_dir${white} postoji...\n";

echo -en "Direktorij ${magenta}$ns_dir${white}";
if [[ -e "$ns_dir" ]]; then
	echo -e "${green} postoji${white}.";
else
	echo -e "${red} ne postoji${white}. Kreirajte direktorij sa instalaranim ns3 simulatorom i smještenim TSN kodovima. Upute se nalaze na GitHub-u: ${magenta}https://github.com/vcrncalo/KUTKM${white}";
	echo "--------------------------------------------------";
	return 0;
fi 

echo "";

# Changing the active directory
echo -e "Promjena aktivnog direktorija: ${magenta}$(pwd)${white} --> ${magenta}$ns_dir${white}\n";

cd "$ns_dir";

# Finding TSN scripts
mapfile -t scratch < <(ls scratch | grep -E 'TSN(1|2)');
echo -e "Pronađene TSN skripte: ${magenta}${scratch[@]}${white}, ukupno ${#scratch[@]} slučaja.";

# Running TSN scripts
counter=1;

for ((i=0; i<=${#scratch[@]}-1; i++)); do
	echo -e "\nPokretanje ${magenta}${scratch[$i]}${white} skripte...\n"; 
	
	while true; do
		read -p "Odaberite maksimalni broj paketa koji će se slati u simulaciji $(($i+1)): " packets;
		if [[ "$packets" =~ ^[0-9]+$ ]]; then
			break
		else
			echo -e "\nNiste unijeli broj. Ponovo unesite maksimalni broj paketa.\n";
		fi;
	done;

	while true; do
		read -p "Odaberite veličinu paketa koji će se slati u simulaciji $(($i+1)) (u bajtima): " size;
		if [[ "$size" =~ ^[0-9]+$ ]]; then
			break
		else
			echo -e "\nNiste unijeli broj. Ponovo unesite veličinu paketa.\n";
		fi;
	done;

	#sleep 1;
	
	current_file="$results_dir/${scratch[$i]%.cc}_ispis.txt";
	gnuplot_file_delay="$gnuplot_dir/${scratch[$i]%.cc}-delay.plt";
	gnuplot_file_jitter="$gnuplot_dir/${scratch[$i]%.cc}-jitter.plt";

	./waf --run "${scratch[$i]%.cc} --maxPackets=$packets --PacketSize=$size" > "$current_file";

	tee "$current_file" <<< "$(grep '^Packet.*received' "$current_file" | awk 'NR == 1 {gsub(/^/, "#", $1); gsub(":", "", $13); printf "%s %s %s %s\n", tolower($1), tolower($5), tolower($8), tolower($13)}{gsub(/s$/, "", $6); gsub(/^:/, "", $10); printf "%s %s %s %s\n", $2, $6, $10, $14}')"  
	echo "";

	# Moving files
	mv $(ls | grep TSN${counter}.*pcap) "$pcap_dir";
	mv $(ls | grep TSN${counter}.xml) "$xml_dir";
	((counter++));

	gnuplot_calc "$gnuplot_file_delay" "$gnuplot_file_jitter" "$current_file" "$size" "$gnuplot_dir_plt" "$gnuplot_dir_png" "$gnuplot_dir";	
done;
echo -e "\nKreiran je glavni TSN direktorij u kojem se nalaze sve relevantne datoteke za ovu simulaciju: ${magenta}$fin_dir${white}";
echo "--------------------------------------------------";
