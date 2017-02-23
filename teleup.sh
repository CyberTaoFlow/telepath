#!/bin/bash
if pgrep "telewatchdog" > /dev/null
then
    exit
else
#    es=$(grep MemTotal /proc/meminfo | awk '{print $2/2/1000000}'  | head -c1)'g'; export ES_HEAP_SIZE=$es;
    echo ' ---> WatchDog is DOWN!! Restarting...' >> /var/log/syslog; /usr/sbin/telepath restart;
fi
