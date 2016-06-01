#!/bin/bash

clear;

#echo ""
sleep 1;
#sudo su;
echo "";
echo -e "\e[38;5;82m------------------------------------------";
echo -e " Starting Network Configuration \e[27m ";
echo -e "------------------------------------------\e[27m";
echo "";
list=$(ifconfig -a | sed 's/[ \t].*//;/^\(lo\|\)$/d');
sudo cp /etc/network/interfaces /etc/network/interfaces.bk;
count=1
echo "";
echo -e "\e[39m---------------------";
echo -e "\e[1mAvailable network interfaces: \e[0m";
echo "";

for j in ${list[@]}
do
	echo "	$count ) $j";
	let count+=1;
done

echo "";
echo "---------------------";
echo "";
echo "";

echo -n -e "\e[38;5;82m>>>\e[0m Network interfaces for remote management \e[36m(seperated by spaces only e.g- eth0 eth1)\e[0m: ";
read interfaces;
if [ ! -z "$interfaces" ]; then
	while [ -z "$interfaces" ];
	do
	clear;
	sleep 0.3;
	echo "";
	echo "";
	echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
	echo "";
	echo -n -e "\e[38;5;82m>>> MUST\e[0m - Network interfaces for remote management \e[36m(seperated by spaces only e.g- eth0 eth1)\e[0m: ";
	read interfaces;
	done
else
	interfaces='eth0';
fi

content="# The loopback network interface\nauto lo\niface lo inet loopback\n\n"
#echo $interfaces;
for i in ${interfaces[@]}
do
	echo "";
	echo -e "\e[0;33m>>> $i configuration:\e[0m";
	echo -n -e "\e[38;5;82m>>>\e[0m IP address \e[36m(default: 192.168.1.250)\e[0m: ";
	read ip;
	if [ ! -z "$ip" ]; then
		while [[ ! $ip =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
		do
		clear;
		sleep 0.3;
		echo "";
		echo "";
		echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
		echo "";
		echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid IP address \e[36m(default: 192.168.1.250)\e[0m: ";
		read ip;
		done
	else
		ip='192.168.1.250';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Netmask \e[36m(default: 255.255.255.0)\e[0m: ";
        read netmask;
        if [ ! -z "$netmask" ]; then
		while [[ ! $netmask =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
        	do
        	clear;
        	sleep 0.3;
        	echo "";
        	echo "";
        	echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
        	echo "";
        	echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid Netmask IP address \e[36m(default: 255.255.255.0)\e[0m: ";
        	read netmask;
        	done
	else
		netmask='255.255.255.0';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Default gateway \e[36m(default: 192.168.1.1)\e[0m: ";
        read gateway;
	if [ ! -z "$gateway" ]; then
		while [[ ! $gateway =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
        	do
        	clear;
        	sleep 0.3;
        	echo "";
        	echo "";
        	echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
        	echo "";
        	echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid default gateway IP address \e[36m(default: 192.168.1.1)\e[0m: ";
        	read gateway;
        	done
	else
		gateway='192.168.1.1';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Network address \e[36m(default: 192.168.1.0)\e[0m: ";
        read network;
	if [ ! -z "$network" ]; then
		while [[ ! $network =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
        	do
        	clear;
        	sleep 0.3;
        	echo "";
        	echo "";
        	echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
        	echo "";
	        echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid network IP address \e[36m(default: 192.168.1.0)\e[0m: ";
        	read network;
        	done
	else
		network='192.168.1.0';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Network broadcast address\e[36m(default: 192.168.1.255)\e[0m: ";
        read broadcast;
	if [ ! -z "$broadcast" ]; then
		while [[ ! $broadcast =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
	        do
	        clear;
	        sleep 0.3;
	        echo "";
	        echo "";
	        echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
	        echo "";
	        echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid Network broadcast IP address \e[36m(default: 192.168.1.255)\e[0m: ";
	        read broadcast;
	        done
	else
		broadcast='192.168.1.255';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Main DNS server address \e[36m(default: 8.8.8.8)\e[0m: ";
        read dns1;
	if [ ! -z "$dns1" ]; then
		while [[ ! $dns1 =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
        	do
        	clear;
        	sleep 0.3;
        	echo "";
        	echo "";
        	echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
        	echo "";
        	echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid main DNS IP address \e[36m(default: 8.8.8.8)\e[0m: ";
        	read dns1;
        	done
	else
		dns1='8.8.8.8';
	fi

	echo -n -e "\e[38;5;82m>>>\e[0m Scondary DNS server address \e[36m(e.g: 8.8.4.4) leave empty if not necessary\e[0m: ";
        read dns2;
	if [ ! -z "$dns2" ]; then
	        while [[ ! $dns2 =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]];
        	do
	        clear;
	        sleep 0.3;
	        echo "";
	        echo "";
	        echo -e "\e[1m\e[31m>>>>>        ERROR!        <<<<<\e[0m";
	        echo "";
	        echo -n -e "\e[38;5;82m>>> MUST\e[0m - Please enter a valid scondary DNS IP address: \e[36m(e.g 8.8.4.4)\e[0m ";
	        read dns2;
	        done
	fi

	block="# The $i network interface\nauto $i\niface $i inet static\n	address $ip\n	gateway $gateway\n	netmask $netmask\n	network $network\n	broadcast $broadcast\n	dns-nameservers $dns1 $dns2\n\n";
        content+=$block;
	#echo $content;
done
echo -e "\e[38;5;82m------------------------------------------\e[0m";
echo ""
echo -e $content;
echo -e "\e[38;5;82m------------------------------------------\e[0m";
echo -e "Please verify the details above before changing the configuration.";
echo  ""
echo -n -e "Continue? \e[38;5;82myes\e[0m/\e[31mno\e[0m: ";
read continue;
y="yes";
n="no";
while [[ ! $continue = "$y" ]] && [[ ! $continue = "$n" ]];
do
	clear;
	echo "";
	echo -n -e "Continue?  - Please use only \e[38;5;82myes\e[0m/\e[31mno\e[0m : "
	read continue;
done

if [ $continue = "$y" ]; then
	echo -e $content > /tmp/interfaces.new;
	sudo mv /tmp/interfaces.new /etc/network/interfaces;
	#sudo /etc/init.d/networking restart;
	echo "";
	echo -e "\e[38;5;82m>>> Network configuration changed. \e[0m"
	echo "";
	sudo rm /opt/telepath/.networkconf;
	echo -n -e "\e[31m>>> Would you like to reboot in order to save the configuration?\e[0m \e[38;5;82myes\e[0m/\e[31mno\e[0m: ";
	read reboot;
	if [ $reboot = "$y" ]; then
                echo "";
                echo -e "\e[38;5;82m>>> The machine will reboot now..  \e[0m"
                sudo reboot;
        fi
else
	sudo rm /etc/network/interfaces.bk;
	echo "";
	echo -e "\e[31m>>> Network configuration was not changed. \e[0m"
	echo "";
	sudo rm /opt/telepath/.networkconf;
fi


#sudo /etc/init.d/networking restart
#ifconfig eth0 192.168.2.123 netmask 255.255.255.0; route add default gw 192.168.2.1
