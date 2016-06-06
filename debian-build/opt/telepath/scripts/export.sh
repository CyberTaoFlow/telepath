#!/bin/bash

if ! grep -qF 'path.repo: ["/mount/backups", "/mount/longterm_backups"]' /opt/telepath/db/elasticsearch/config/elasticsearch.yml;
then 
	echo 'path.repo: ["/mount/backups", "/mount/longterm_backups"]' >> /opt/telepath/db/elasticsearch/config/elasticsearch.yml
else
	echo "no need to add repo"
fi

# Repository
curl -XPUT 'http://localhost:9200/_snapshot/my_backup' -d '{"type": "fs","settings": {"location": "/mount/backups/my_backup","compress": true }}'

# Snapshot
curl -XPUT 'localhost:9200/_snapshot/my_backup/snapshot_1?wait_for_completion=true'

# Compress
cd /mount/backups/my_backup
tar -czvf /tmp/dump.tar.gz ./*

# Delete
curl -XDELETE 'localhost:9200/_snapshot/my_backup'

# Backing up the Brain's data
cd /opt/telepath/db/kb_bin/
tar -czvf /tmp/brain.tar.gz ./*

# Saving current configurations and whitelist 
echo -n ">>> Please enter your MYSQL Username: "
read user
echo -n ">>> Please enter your MYSQL Password: "
read pass
mysqldump -u$user -p$pass telepath config whitelist_ip > /tmp/table_backup.sql

# Sending the files via SSH
echo -n ">>> Need to use SCP to remote server? Y/n : "
read answer
if [ "$answer" = "Y" ];
then
	echo -n ">>> Please provide an remote server connection ( user@domain.com ):"
	read sshDst
	scp /tmp/dump.tar.gz $sshDst:/tmp/dump.tar.gz
	scp /tmp/brain.tar.gz $sshDst:/tmp/brain.tar.gz
	scp /tmp/table_backup.sql $sshDst:/tmp/table_backup.sql
	rm /tmp/dump.tar.gz
	rm /tmp/brain.tar.gz
	rm /tmp/table_backup.sql
	echo ">>> Done! please check /tmp folder at your remote server"
else
	echo ">>> Done! please check /tmp folder"
fi
rm -r /mount/backups/

