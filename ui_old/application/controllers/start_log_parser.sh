#!/bin/bash

telepath stop
sleep 1

/opt/telepath/bin/log_engine /opt/telepath/conf/atms.conf > /dev/null

while true 
do
        ps -A | grep log_engine > /dev/null
        if [ $? -eq 0 ]; then
                sleep 1
        else
                break
        fi
done

sleep 1
telepath start

exit 0

