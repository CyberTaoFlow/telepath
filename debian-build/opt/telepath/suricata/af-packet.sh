#!/bin/bash
clusterBase=99
counter=0
#MYUSER="telepath"
#MYPASS=$(cat /opt/telepath/conf/atms.conf |grep password)
#MYPASS=$(echo $MYPASS | sed 's/^.*="//')
#MYPASS=$(echo $MYPASS | sed 's/"//')

curl=$(curl -XGET "http://localhost:9200/telepath-config/interfaces/interface_id/_source")
interfaces=$(echo $curl | jq ".interfaces[].interface_name")
interfaces=$(echo $interfaces | sed 's/^.*="//')

#echo $interfaces
cp /opt/telepath/suricata/suricata.yaml.bak /opt/telepath/suricata/suricata.yaml


for i in ${interfaces[@]//\"/}
do
        clusterId=$(($clusterBase+$counter))
        block="af-packet:\n - interface: $i\n   threads: 8\n   cluster-id: $clusterId\n   cluster-type: cluster_flow\n   defrag: yes\n   use-mmap: yes\n   ring-size: 20480\n\n"
        let "counter+=1"
        content+=$block
	ifconfig $i up;
#       echo $i
done

#echo "$content"
#SELECT NetworkInterface FROM agents;
sed -i "s/^#af-interfaces/$content/g" /opt/telepath/suricata/suricata.yaml
if [ -f "/etc/redis.conf" ]; then
        if grep -q logfile /var/log/redis/redis-server.log "/etc/redis.conf"; then
                sed -i 's|^logfile /var/log/redis/redis-server.log|#logfile /var/log/redis/redis-server.log|g' /etc/redis.conf
        fi
fi
if [ -f "/etc/redis/redis.conf" ]; then
        if grep -q logfile /var/log/redis/redis-server.log "/etc/redis/redis.conf"; then
                sed -i 's|^logfile /var/log/redis/redis-server.log|#logfile /var/log/redis/redis-server.log|g' /etc/redis/redis.conf
        fi
fi
