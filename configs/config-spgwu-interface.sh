#!/bin/bash
TARGET_IFACE="eth1"
TARGET_SGI_IFACE="eth0"
TARGET_IP_S1U="192.168.15.18"
TARGET_IP_SGI="172.17.0.2"

ip addr add 172.55.55.102/24 brd + dev $TARGET_IFACE label $TARGET_IFACE:sxu
ip addr add $TARGET_IP_S1U brd + dev $TARGET_IFACE label $TARGET_IFACE:s1u
LIST=$(grep -ris lte /etc/iproute2/rt_tables)
if [ -z "$LIST" ]; then echo '200 lte' | tee --append /etc/iproute2/rt_tables; else echo "lte table has already created"; fi
ip r add default via $TARGET_IP_SGI dev $TARGET_SGI_IFACE table lte
ip rule add from 12.0.0.0/8 table lte
