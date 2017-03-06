#!/bin/bash
timestamp() {
  date +"%T"
}
echo "";
echo "--------------------------------";
echo "<<<< Starting Reset Process >>>>";
echo "--------------------------------";
echo "";
telepath stop;
sleep 10;
echo "";
echo "-----------------------------------";
echo "<<<< Delete ElasticSearch Data >>>>";
echo "-----------------------------------";
echo "";
su elastic -c "/opt/telepath/db/elasticsearch/bin/elasticsearch -d";
sleep 15;
curl -XDELETE 'http://localhost:9200/telepath-20*?timeout=5m';
curl -XDELETE 'http://localhost:9200/telepath-domains?timeout=5m';
curl -XDELETE 'http://localhost:9200/telepath-actions?timeout=5m';
curl -XDELETE 'http://localhost:9200/telepath-rules?timeout=5m';
curl -XDELETE 'http://localhost:9200/telepath-users?timeout=5m';
echo "";
echo timestamp >> /var/log/reset.log
#echo "---------------------------";
#echo "<<<< Delete MySQL Data >>>>";
#echo "---------------------------";
#echo "";

#echo -n ">>> Please enter the MySQL username: ";
#read user;

#echo -n ">>> Please enter the MySQL password: ";
#read pass;

#mysql -u $user -p$pass telepath << EOF
#update config set value='1' where action_code=1;
#update config set value='0' where action_code=70;
#update config set value='1000000' where action_code=6;
#EOF

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


rm /opt/telepath/db/kb_bin/paths/*
rm /opt/telepath/db/kb_bin/user_paths/*
rm /opt/telepath/db/kb_bin/user_attributes/*
rm /opt/telepath/db/kb_bin/attributes/all_attributes
rm /opt/telepath/db/kb_bin/globals/sessions
rm /opt/telepath/db/kb_bin/flows/*

redis-cli flushall
telepath restart
echo "";
echo "--------------------------------";
echo "<<<< Reset Process Complete >>>>";
echo "--------------------------------";


