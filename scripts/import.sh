#!/bin/bash

# Delete
curl -XDELETE 'localhost:9200/_snapshot/my_backup'
rm -r /mount/backups/

if [ ! grep -qF 'path.repo: ["/mount/backups", "/mount/longterm_backups"]' /opt/telepath/db/elasticsearch/config/elasticsearch.yml ];
then 
        echo 'path.repo: ["/mount/backups", "/mount/longterm_backups"]' >> /opt/telepath/db/elasticsearch/config/elasticsearch.yml
else
        echo "no need to add repo"
fi

# Repository
curl -XPUT 'http://localhost:9200/_snapshot/my_backup' -d '{"type": "fs","settings": {"location": "/mount/backups/my_backup","compress": true }}'

# Decompress
mv /tmp/dump.tar.gz /mount/backups/my_backup/
cd /mount/backups/my_backup
tar -xzvf ./dump.tar.gz 
mv /tmp/brain.tar.gz /opt/telepath/db/kb_bin/
cd /opt/telepath/db/kb_bin/
tar -xzvf ./brain.tar.gz

# Importing the SQL's tables 
echo -n ">>> Please enter your MYSQL Username: "
read user
echo -n ">>> Please enter your MYSQL Password: "
read pass
mysql -u $user -p$pass -h localhost telepath < /tmp/table_backup.sql

# Restore
curl -XPOST 'localhost:9200/_snapshot/my_backup/snapshot_1/_restore?wait_for_completion=true'

# Delete
curl -XDELETE 'localhost:9200/_snapshot/my_backup'

echo "Done importing"


