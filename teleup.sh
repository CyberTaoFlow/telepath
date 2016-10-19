#!/bin/bash
if pgrep "telewatchdog" > /dev/null
then
    exit
else
    echo ' ---> WatchDog is DOWN!! Restarting...' >> /var/log/syslog; sudo telepath restart;
fi