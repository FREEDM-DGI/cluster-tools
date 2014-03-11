#! /bin/bash
set -x

for i in $(seq 1 6); do
	for j in $(seq 1 6); do
		ssh root@192.168.200.1$i "route add -net 192.168.1${j}0.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0"
	done
	
	ssh root@192.168.200.1$i "route del -net 192.168.1${i}0.0/24 gw 192.168.1${i}0.41 dev eth0"

	for k in $(seq 17 22); do
		ssh root@192.168.200.1$i "route add -host 192.168.200.$k gw 192.168.1${i}0.41 dev eth0"
	done
done

for n in $(seq 17 22); do
	for m in $(seq 1 6); do	
		ssh root@192.168.200.$n "route add -net 192.168.1${m}0.0/24 gw 192.168.200.41 dev eth0"
	done
done

#	ssh root@192.168.200.1$i "route add -net 192.168.120.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0"
# && route add -net 192.168.130.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0 && route add -net 192.168.140.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0 && route add -net 192.168.150.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0 && route add -net 192.168.160.0 netmask 255.255.255.0 gw 192.168.1${i}0.41 dev eth0"


