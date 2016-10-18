#!/bin/bash
if pgrep "telewatchdog" > /dev/null
then
else
    echo ' ---> WatchDog is DOWN!! Restarting...' >> /var/log/syslog
    telepath restart;
fi