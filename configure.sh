#!/bin/bash
#
# Dialog config
DIALOG=${DIALOG=dialog}
CMD="--ascii-lines --title "Telepath" --clear"

# Executable / DB Name
#MYSQL="mysql"
#USER="telepath"
#DATABASE="telepath"

# Main configuration JSON file that will be updated from the UI with the interface settings
MAIN_JSON='/opt/telepath/conf/telepath.json'
if [ ! -f $MAIN_JSON ]; then
	touch $MAIN_JSON
	chmod 777 $MAIN_JSON
fi

if [ -n "$(which apt-get)" ]; then
	# apt-get -y install dialog php5 libapache2-mod-php5 gdb php5-sqlite lua-sec lua5.1 lua-socket libcurl-ocaml-dev luarocks jq #sendmail
	apt-get -y install dialog php7.0 libapache2-mod-php7.0 gdb php7.0-sqlite3 lua-sec lua5.1 lua-socket libcurl-ocaml-dev luarocks jq htop  #sendmail
	apt-get -y install php-pear php7.0-dev php7.0-gd php7.0-msgpack run-one
	echo "oracle-java8-installer shared/accepted-oracle-license-v1-1 select true" | sudo debconf-set-selections
	apt-get -y install oracle-java8-installer
	# pecl install msgpack-0.5.7
	# echo extension=msgpack.so > /etc/php5/mods-available/msgpack.ini
	# cd /etc/php5/apache2/conf.d/
	# ln -s ../../mods-available/msgpack.ini ./
	# mv ./msgpack.ini ./20-msgpack.ini
	# cd /etc/php5/cli/conf.d/
	# ln -s ../../mods-available/msgpack.ini ./
	# mv ./msgpack.ini ./20-msgpack.ini
#	fi
	#sudo printf "Y\nY\nY\n" | sendmailconfig

	luarocks install Lua-cURL
	#luarocks install Lua-cURL --server=https://rocks.moonscript.org/dev
fi

if [ -n "$(which yum)" ]; then
	#yum -y install dialog php php5 libapache2-mod-php5 php-cli gdb php-mysql mysql mysql-server mod_ssl lua-socket-devel.x86_64 ocaml-curl-devel.x86_64 lua-devel.x86_64 flex.x86_64 bison.x86_64 jq
	yum -y install dialog php php7.0 libapache2-mod-php7.0 php-cli gdb php7.0-sqlite3 mod_ssl lua-sec lua-socket-devel.x86_64 ocaml-curl-devel.x86_64 lua-devel.x86_64 flex.x86_64 bison.x86_64 jq
	cd /tmp/
	wget http://luarocks.org/releases/luarocks-2.0.6.tar.gz
	tar -xzvf luarocks-2.0.6.tar.gz
	cd luarocks-2.0.6
	./configure
	make
	make install
	luarocks install Lua-cURL
	#luarocks install Lua-cURL --server=https://rocks.moonscript.org/dev
fi

#echo "If mysql wasnt installed before please configure it first."
#echo "service mysqld start"
#echo "mysqladmin -u root password <newpassword>"


CONFDEFAULT='/opt/telepath/conf/atms-default.conf'
CONF='/opt/telepath/conf/atms.conf'
DATACONF='/opt/telepath/conf/database.conf'
#MYSQL_SCRIPT="/opt/telepath/db/telepath.sql"

# Default values
#MYSQL_HOST="localhost"
#MYSQL_PORT="3306"
#MYSQL_USER="root"
conf_maintenence() {
    # Make sure we have exactly one copy of once a minute reports cron
	command="run-one php /opt/telepath/ui/html/index.php cron reports"
	job="* * * * * $command"
	cat <(fgrep -i -v "$command" <(crontab -l)) <(echo "$job") | crontab -

    #chmod +x /opt/telepath/suricata/run.sh
	chmod +x /opt/telepath/suricata/suricata
	chmod +x /opt/telepath/suricata/af-packet.sh
    chmod 755 /opt/telepath/suricata/af-packet.sh
	chmod 777 /opt/telepath/suricata
    chmod +x /opt/telepath/teleup.sh

	# Restart telepath if it running in the background at this point
	pgrep telewatchdog && telepath restart
	rm /opt/telepath/suricata/logs/stats.log -f
}
conf_maintenence_old() {
	
	# Make sure we have exactly one copy of once a minute reports cron
	command="run-one php /opt/telepath/ui/html/index.php cron reports"
	job="* * * * * $command"
	cat <(fgrep -i -v "$command" <(crontab -l)) <(echo "$job") | crontab -
	
	# Read the config file
	source $CONF
	
	if [ "$database_address" == "localhost" ] || [ "$database_address" == "127.0.0.1" ]; then 
	
		my_conf_ubuntu="/etc/mysql/my.cnf"
		my_conf_centos="/etc/my.cnf"
		my_conf=""
		
		if [ -f "$my_conf_ubuntu" ]
		then
			my_conf=$my_conf_ubuntu
		else
			if [ -f "$my_conf_centos" ]
			then
				my_conf=$my_conf_centos
			fi
		fi
		
		if [ -n "$my_conf" ]; then
			
			sed -i '/server_timeout/d' $my_conf
			sed -i '/interactive_timeout/d' $my_conf
			sed -i '/\[mysqld\]/a wait_timeout = 900\ninteractive_timeout = 900\n' $my_conf
			
		else
			echo "Could not find my.cnf file, please configure wait_timeout and interactive_timeout variables manually"
		fi
		
	fi
	
	username=""
	password=""
	hostname=""

	while read -r line
	do

			name=$line
			IFS=': ' read -ra arr <<< "$name"
			
			if [ ${#arr[@]} -eq 2 ] ; then
			
				if [ "${arr[0]}" = "user" ] ; then
					username="${arr[1]}"
					username=$(echo "${username//[$'\t\r\n']}")
				fi
				if [ "${arr[0]}" = "password" ] ; then
					password="${arr[1]}"
					password=$(echo "${password//[$'\t\r\n']}")
				fi
				if [ "${arr[0]}" = "host" ] ; then
					hostname="${arr[1]}"
					hostname=$(echo "${hostname//[$'\t\r\n']}")
				fi

			fi

	done < "$DATACONF"

	if [ -n "$hostname" ] && [ -n "$username" ] && [ -n "$password" ] ; then
		echo "Updating MYSQL runtime variables..."
		mysql -h $hostname -u $username -p$password -e 'SET GLOBAL connect_timeout=900; SET GLOBAL interactive_timeout=900;'
	fi

	chmod +x /opt/telepath/suricata/run.sh
	chmod +x /opt/telepath/suricata/suricata
	chmod +x /opt/telepath/suricata/af-packet.sh
    chmod 755 /opt/telepath/suricata/af-packet.sh
	chmod 777 /opt/telepath/suricata
    chmod +x /opt/telepath/teleup.sh

	# Restart telepath if it running in the background at this point
	pgrep telewatchdog && telepath restart
	rm /opt/telepath/suricata/logs/stats.log -f

}

check_installed() {



	if [ ! -f $CONF ]; then
	echo "Configuration file not found, please run /opt/telepath/configure.sh for initial configuration"
		exit
	fi

	
	
	echo "Configuration file found, running upgrade database script"

	# Read the config file
	source $CONF
	conf_maintenence
	
	telepath start
	
	#set initial Telepath start when machine loads
	sed -i '/telepath/d' /etc/rc.local
	sed -i -e '$i \telepath start &\n' /etc/rc.local
		
	# Done!
	exit

}

install() {

	# Validate packages
	binaries

	# Collect info from user
	# conf_interface
	# conf_mysql
	# conf_admin_user
	# conf_admin_password

	# Create configuration file
	# conf_create_file

	# Additional configuration steps
	conf_setup_perms
	#conf_create_db
	#conf_create_tbls
	conf_update_apache
	conf_sqlite_db
	conf_init_cron
    cron_jobs
	conf_maintenence

}

check_packages() {

	if [ -n "$(which apt-get)" ]; then
		apt-get -y install dialog gdb php5-mysql mysql-common mysql-server
	fi

	if [ -n "$(which yum)" ]; then
		yum install dialog gdb php php-cli php-mysql mysql mysql-server
	fi

	echo "If mysql wasnt installed before please configure it first."
	echo "service mysqld start"
	echo "mysqladmin -u root password <newpassword>"

}

validate_dialog() {

	if [ ! -n "$(which dialog)" ]; then

		if [ -n "$(which apt-get)" ]; then
			apt-get install dialog
		fi

		if [ -n "$(which yum)" ]; then
			yum install dialog
		fi

	fi

}

menu() {

validate_dialog

retv=$?

choice=$(dialog --stdout --ascii-lines --title "Telepath configuration utility" \
--menu "Please choose an option:" 15 55 10 \
1 "Configuration Wizard")

[ $retv -eq 1 -o $retv -eq 255 ] && exit

case $choice in
	1)
	install
	exit
	;;
	4)
	echo 0 > /selinux/enforce
	echo "executed:: echo 0 > /selinux/enforce , run it again with 1 to switch back on."
	exit
	;;
esac


}


terminate() {
	$DIALOG $CMD --msgbox 'Configuration aborted.' 5 40
	exit 1
}

binaries() {

	#MYSQL=`which mysql` || true
	#MYSQLADMIN=`which mysqladmin` || true
	DIALOG_CHK=`which dialog` || true

	if [ -z $DIALOG_CHK ]; then
		echo "Required package dialog is missing."
		exit 1
	fi

	# CHECK IF WE HAVE ELASTICSEARCH DIR IN DB
	if [ -d "/opt/telepath/db/elasticsearch" ]; then

			if [ ! -f "/opt/telepath/db/elasticsearch/config/elasticsearch.yml" ]; then
					cd /opt/telepath/db/
					wget https://artifacts.elastic.co/downloads/elasticsearch/elasticsearch-5.2.0.tar.gz
					# wget https://download.elastic.co/elasticsearch/release/org/elasticsearch/distribution/tar/elasticsearch/2.4.1/elasticsearch-2.4.1.tar.gz
					tar -xf elasticsearch-5.2.0.tar.gz
					mv elasticsearch-5.2.0/* elasticsearch
					rm elasticsearch-5.2.0.tar.gz
					rm -rf elasticsearch-5.2.0
					#sed -i 's/^#index.number_of_shards: 1/index.number_of_shards: 1/g' /opt/telepath/db/elasticsearch/config/elasticsearch.yml
					#sed -i 's/^#network.bind_host: 192.168.0.1/network.bind_host: 127.0.0.1/g' /opt/telepath/db/elasticsearch/config/elasticsearch.yml
					sed -i 's/com.amazonaws: WARN/ ^#com.amazonaws: WARN/g' /opt/telepath/db/elasticsearch/config/loggin.yml
					echo -e "*	soft nofile 100000\n* hard nofile  100000" >> /etc/security/limits.conf
					#echo "es=\$(grep MemTotal /proc/meminfo | awk '{print \$2/4/1000000}'  | head -c1)'g'; export ES_HEAP_SIZE=\$es;" >> ~/.bashrc
					#echo "es=\$(grep MemTotal /proc/meminfo | awk '{print \$2/4/1000000}'  | head -c1)'g'; export ES_HEAP_SIZE=\$es;" >> /root/.bashrc
					#echo "es=\$(grep MemTotal /proc/meminfo | awk '{print \$2/4/1000000}'  | head -c1)'g'; export ES_HEAP_SIZE=\$es; exit 0" > /etc/rc.local
					#echo "script.groovy.sandbox.enabled: true" >> /opt/telepath/db/elasticsearch/config/elasticsearch.yml
					echo "http://localhost:9200" > /opt/telepath/db/elasticsearch/config/connect.conf
					# create elasticsearch user
					adduser --disabled-password --gecos "" elastic
					chown -R elastic:elastic /opt/telepath/db/elasticsearch/
			else
					echo "Elasticsearch was detected under /etc/elasticsearch , skipping."
			fi


			# We need to install default rules
			# echo "Starting Elastissearch"
			# /opt/telepath/db/elasticsearch/bin/elasticsearch -d

	fi

	# CHECK IF WE HAVE SURICATA COMPILED AND INSTALLED IN OUR DIR
	if [ ! -f "/opt/telepath/suricata/suricata" ]; then

			echo "Running suricata install script, this might take up to 5 minutes and requires internet connection"
			chmod +x /opt/telepath/suricata/run
			chmod +x /opt/telepath/suricata/suricata.sh
			/opt/telepath/suricata/suricata.sh

	else
			echo "Suricata was detected under /opt/telepath/suricata , skipping."
	fi

        # CHECK IF WE HAVE NGINX COMPILED AND INSTALLED IN OUR DIR
        if [ ! -f "/opt/telepath/openresty/nginx/sbin/nginx" ]; then

            echo "Running NGINX install script, this might take up to 5 minutes and requires internet connection"
            chmod +x /opt/telepath/openresty/openresty.sh
			/opt/telepath/openresty/openresty.sh
			mkdir /opt/telepath/openresty/nginx/certs/
			chmod 777 /opt/telepath/openresty/nginx/certs/

        else
                        echo "NGINX was detected under /opt/telepath/openresty/nginx/sbin/nginx , skipping."
        fi

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
    #echo -e "vm.swappiness=0\nvm.overcommit_memory=1" >> /etc/sysctl.conf;
    
	#if [ -z $MYSQL ]; then
	#	$DIALOG $CMD --msgbox "MySql was not found, aborting." 5 40
	#	terminate
	#fi

	#if [ -z $MYSQLADMIN ]; then
	#	$DIALOG $CMD --msgbox "MySqlAdmin was not found, aborting." 5 40
	#	terminate
	#fi

	pip install elasticsearch
        pip install tqdm
        #chmod +x /opt/telepath/db/elasticsearch/rules.py
        #echo "Setting Default Rules"
        #ython /opt/telepath/db/elasticsearch/rules.py

	rm -rf /usr/sbin/telepath > /dev/null 2>&1 || true
	ln -s /opt/telepath/bin/telepath /usr/sbin/telepath > /dev/null 2>&1 || true

	rm -rf /etc/init.d/telepath.d > /dev/null 2>&1 || true
	ln -s /opt/telepath/bin/telepath.d /etc/init.d/telepath.d > /dev/null 2>&1 || true
	update-rc.d telepath.d defaults 98 02 > /dev/null 2>&1 || true

	mv /opt/telepath/bin/gulp /usr/sbin/

	# Our special SSD config
	#if [ -d /sas ]; then

		# If our system has apparmor
	#	if [ -d /etc/apparmor.d ]; then
	#		sed -i "s/\\}/\\/sas\\/telepath\\/ r,\n\\/sas\\/telepath\\/** rwk,\n\\}/g" /etc/apparmor.d/usr.sbin.mysqld
	#	fi

	#fi

	# Create telepath user (needed for memcached webservice mode
	useradd telepath
		
	# Disable SELINUX if enabled
	if [ -n getenforce ]; then
		sed -i 's/SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config
		setenforce permissive
    fi


}

conf_update_apache() {

	chmod +x /opt/telepath/ui/html/application/controllers/start_log_parser.sh > /dev/null 2>&1 || true

	if [ $(cat /etc/passwd | grep apache) ]; then
           USER="apache"
        fi
        if [ $(cat /etc/passwd | grep www-data) ]; then
           USER="www-data"
	fi

	# Cleanup sudoers
	sed -i '/telepath/d' /etc/sudoers
	sed -i "/Defaults:$USER !requiretty/d" /etc/sudoers

	# Modify sudoers
    echo "Defaults:$USER !requiretty" >> /etc/sudoers
	echo "$USER ALL = (ALL) NOPASSWD: /opt/telepath/bin/telepath, /opt/telepath/openresty/nginx/sbin/nginx"	 >> /etc/sudoers
    
	if [ ! -d /opt/telepath/ui/html/files ]; then
		mkdir /opt/telepath/ui/html/files
	fi 

	chmod 777 /opt/telepath/ui/html/files > /dev/null 2>&1 || true

	if [ -n "$(which httpd)" ]; then
		ln -s /opt/telepath/conf/apache-telepath.conf /etc/httpd/conf.d/apache-telepath.conf > /dev/null 2>&1 || true
	fi

	if [ -n "$(which apache2)" ]; then

		if [ -d "/etc/apache2/conf-available/" ]; then
			ln -s /opt/telepath/conf/apache-telepath.conf /etc/apache2/conf-available/apache-telepath.conf > /dev/null 2>&1 || true
			a2enconf apache-telepath
		else

			if [ -d "/etc/apache2/conf.d/" ]; then
				ln -s /opt/telepath/conf/apache-telepath.conf /etc/apache2/conf.d/apache-telepath.conf > /dev/null 2>&1 || true
			fi

		fi
		sed -i 's|DocumentRoot /var/www/html|DocumentRoot /opt/telepath/ui/html|g' /etc/apache2/sites-available/000-default.conf
		sed -i 's|:80>|:8000>|g' /etc/apache2/sites-available/000-default.conf
		sed -i 's|Listen 80|Listen 8000|g' /etc/apache2/ports.conf
	fi
	a2enmod rewrite
	#chmod +x /opt/telepath/generate-ssl.sh
	#/opt/telepath/generate-ssl.sh
	if [ -n "$(which httpd)" ]; then
                service httpd restart
        fi

        if [ -n "$(which apache2)" ]; then
		service apache2 restart
	fi
}
conf_sqlite_db() {
    cp /opt/telepath/ui/html/application/databases/telepath_users_default.db /opt/telepath/ui/html/application/databases/telepath_users.db
    
	if [ $(cat /etc/passwd | grep apache) ]; then
           USER="apache"
        fi
        if [ $(cat /etc/passwd | grep www-data) ]; then
           USER="www-data"
	fi
    chown $USER:$USER /opt/telepath/ui/html/application/databases/telepath_users.db
    mkdir /opt/telepath/ui/html/application/sessions
    mkdir /opt/telepath/ui/html/upload
    chmod 700 /opt/telepath/ui/html/application/databases
    chmod 700 /opt/telepath/ui/html/application/sessions
    chown $USER:$USER /opt/telepath/ui/html/application/databases
    chown $USER:$USER /opt/telepath/ui/html/application/sessions
    chown $USER:$USER /opt/telepath/ui/html/application/config
    chown $USER:$USER /opt/telepath/ui/html/upload
}
    
conf_create_db() {

	if $MYSQL -s $MUO -BNe 'show databases' | grep -q -E "^$DATABASE\$"; then
		echo "Database already exists"
	else
		echo "Creating database $DATABASE..."
		if ! $MYSQLADMIN -s $MUO -f create $DATABASE; then
			$DIALOG $CMD --msgbox "Failed to create database $DATABASE" 5 40
			terminate
		fi
	fi

	if [ "$MYSQL_HOST" = "localhost" -o "$MYSQL_HOST" = "127.0.0.1" ]; then

		if ! $MYSQL -s $MUO $DATABASE -f -e "GRANT ALL ON $DATABASE.* TO '${USER}'@'localhost' IDENTIFIED BY '${PASSWD}'"; then
			$DIALOG $CMD --msgbox "Failed to grant permissions" 5 40
			terminate
		fi
		# create a link to mysql database
		if [ -d "/var/lib/mysql/"$DATABASE ]; then
		  ln -s /var/lib/mysql/$DATABASE/ /opt/telepath/db/sql
		fi

	else

		if ! $MYSQL -s $MUO $DATABASE -f -e "GRANT ALL ON $DATABASE.* TO ${USER}@'%' IDENTIFIED BY '${PASSWD}'"; then
			$DIALOG $CMD --msgbox "Failed to grant permissions" 5 40
			terminate
		fi

		#LOCALDB=`ython /opt/telepath/bin/get_ip.py $MYSQL_HOST` > /dev/null 2>&1 || true
		#if [ ! -z $LOCALDB ]; then
		#  if [ -d "/var/lib/mysql/"$DATABASE ]; then
		#	ln -s /var/lib/mysql/$DATABASE/ /opt/telepath/db/sql
		#  fi
		#fi

	fi

	$MYSQL -s $MUO $DATABASE -f -e "flush privileges" > /dev/null 2>&1 || true
	echo "flush privileges" | $MYSQL -s $MUO -f $DATABASE > /dev/null 2>&1 || true

}
cron_jobs(){
    crontab -l > currentCrons
    echo "* * * * * run-one php /opt/telepath/ui/html/index.php cases flag_requests_by_cases >> /var/log/flag_requests_by_cases.log 2>&1 || true" >> currentCrons
    echo "*/5 * * * * run-one php /opt/telepath/ui/html/index.php webusers store_users >> /var/log/web_users.log 2>&1 || true" >> currentCrons
    echo "0 * * * * run-one php /opt/telepath/ui/html/index.php cases store_similar_case_sessions >> /var/log/store_similar_case_sessions.log 2>&1 || true" >> currentCrons
    echo "*/2 * * * * run-one /opt/telepath/teleup.sh" >> currentCrons
    crontab currentCrons
    rm currentCrons
}
conf_create_tbls() {

	echo "Creating/Updating table structure..."
	php /opt/telepath/ui/html/index.php setup cli $MYSQL_HOST $MYSQL_PORT $MYSQL_USER $MYSQL_PASS

	echo "Create Hybrid telepath management user"
	echo "INSERT INTO registered_users(user, password) VALUES ('"$ADMIN_USER"','"$ADMIN_PASS"') ON DUPLICATE KEY UPDATE password='"$ADMIN_PASS"';" | $MYSQL -s $MUO -f $DATABASE > /dev/null 2>&1 || true

}

conf_init_cron() {
	# Add new job update+email (PHP)
	croncmd="run-one php /opt/telepath/ui/html/index.php cron"
        cronjob="0 * * * * $croncmd"
        ( crontab -l | grep -v "$croncmd" ; echo "$cronjob" ) | crontab -
	if [ -n "$(which yum)" ]; then
        # Add new job update+email (PHP)
        	croncmd="yum clean metadata && yum upgrade telepath -y"
        	cronjob="0 * * * * $croncmd"
        	( crontab -l | grep -v "$croncmd" ; echo "$cronjob" ) | crontab -
	fi
	# executing init cron jobs
	# eval $croncmd
	echo "--> Setting .htaccess production ENV."
	echo -e "\nSetEnv CI_ENV production" >> /opt/telepath/ui/html/.htaccess
}

conf_setup_perms() {
	chown root:root -R /opt/telepath/ui
}

conf_interface() {

	interface_list=""
	counter=1

	for i in `cat /proc/net/dev | grep ':' | cut -d ':' -f 1`
	do
			interface_list="$interface_list $i $counter off"
			counter=$[counter+1]
	done

	choices=$($DIALOG $CMD --stdout --checklist 'Select interfaces for sniffing:' 20 40 20 $interface_list)

	if [ $? -eq 0 ]
	then
		INTERFACES=$choices
	else
		terminate
	fi

}

conf_mysql() {

	conf_mysql_server
	conf_mysql_port
	conf_mysql_user
	conf_mysql_password

	# MYSQL - BUILD CHECK
	MUO="-u ${MYSQL_USER} "
	if [ ! -z $MYSQL_PASS ]; then
		MUO=$MUO"-p${MYSQL_PASS} "
	fi

	if [ -z $MYSQL_HOST -o "$MYSQL_HOST" = "localhost" ]; then
		MUO=$MUO
	else
		MUO=$MUO" --host="$MYSQL_HOST
	fi

	if [ -z $MYSQL_PORT -o "$MYSQL_PORT" = "3306" ]; then
		MUO=$MUO
	else
		MUO=$MUO" --port="$MYSQL_PORT
	fi

	# MYSQL - CHECK
	RES=`echo "select 5+13;" | $MYSQL -s $MUO 2> /dev/null` || true
	if [ -z $RES ]; then
	  $DIALOG $CMD --msgbox 'Failed connecting to mysql using provided settings.' 5 60
	  conf_mysql
	fi

}

conf_mysql_server() {

	input=$($DIALOG $CMD --stdout --inputbox 'MySQL server address:' 8 40 $MYSQL_HOST)

	if [ $? -eq 0 ]
	then

		if [ ! $input ]; then
			conf_mysql_server
			return
		fi

		MYSQL_HOST=$input
	else
		terminate
	fi

}
conf_mysql_port() {

	input=$($DIALOG $CMD --stdout --inputbox 'MySQL server port:' 8 40 $MYSQL_PORT)

	if [ $? -eq 0 ]
	then

		if [ ! $input ]; then
			conf_mysql_port
			return
		fi

		MYSQL_PORT=$input
	else
		terminate
	fi
}
conf_mysql_user() {

	input=$($DIALOG $CMD --stdout --inputbox 'MySQL server administator username:' 8 40 $MYSQL_USER)

	if [ $? -eq 0 ]
	then

		if [ ! $input ]; then
			$DIALOG $CMD --msgbox 'Input a valid username for mysql root account.' 5 60
			conf_mysql_user
			return
		fi

		MYSQL_USER=$input
	else
		terminate
	fi

}
conf_mysql_password() {

	input=$($DIALOG $CMD --stdout --passwordbox 'MySQL server administator password:' 8 40)

	if [ $? -eq 0 ]
	then
		MYSQL_PASS=$input
	else
		terminate
	fi

}

conf_admin_user() {

	input=$($DIALOG $CMD --stdout --inputbox 'Telepath administrator username:' 8 50 admin)
	if [ $? -eq 0 ]
	then

		if [ ! $input ]; then
			conf_admin_user
			return
		fi

		ADMIN_USER=$input

	else
		terminate
	fi

}
conf_admin_password() {

	PASS_1=$($DIALOG $CMD --stdout --passwordbox 'Telepath administrator password:' 8 50)

	if [ $? -eq 0 ]; then

		if [ ! $PASS_1 ]; then
			$DIALOG $CMD --msgbox 'Input a valid password for telepath admin account.' 5 60
			conf_admin_password
			return
		fi

		PASS_2=$($DIALOG $CMD --stdout --passwordbox 'Telepath administrator password (repeat):' 8 50)

		if [ $? -eq 0 ]; then

			if [ $PASS_1 == $PASS_2 ]; then
				ADMIN_PASS=$PASS_1
			else
				$DIALOG $CMD --msgbox 'Passwords dont match, try again.' 5 60
				conf_admin_password
			fi

		else
			terminate
		fi

	else
		terminate
	fi

}

conf_create_file() {

	if [ ! -f $CONF ]; then
		echo "Install default config file"
		cp -a $CONFDEFAULT $CONF
	fi

	#OLDP=`grep "password=" $CONF 2>/dev/null | sed 's/[^=]*=\s*//' 2>/dev/null` || true
	#OLDP=`echo $OLDP | sed -s "s/^\(\(\"\(.*\)\"\)\|\('\(.*\)'\)\)\$/\\3\\5/g"` || true
	#if [ "$OLDP" = "" -o "$OLDP" = "password" ]; then
	#	PASSWD=`</dev/urandom tr -dc A-Za-z0-9 | head -c16` || true
	#else
	#	PASSWD=$OLDP
	#fi

	cfg_cont=`grep -v -E "(gulp_network_interface|username|password|database_\\w+)=" $CONF` 

	echo "database_address=\"$MYSQL_HOST\"" > $CONF
	if [ "$MYSQL_PORT" != "" ] && [ "$MYSQL_PORT" != "3306" ]; then
		echo "database_port=\"${MYSQL_PORT}\"" >> $CONF
	else
		echo "database_port=\"3306\"" >> $CONF
	fi

	echo 'database_name="telepath"' >> $CONF
	echo 'database_type="MYSQL"' >> $CONF
	echo "username=\"$USER\"" >> $CONF
	echo "password=\"$PASSWD\"" >> $CONF

	echo "gulp_network_interface=eth0" >> $CONF
	echo "$cfg_cont" >> $CONF

	echo "[connection_string]" > $DATACONF
	echo "host: $MYSQL_HOST" >> $DATACONF
	echo "user: $MYSQL_USER" >> $DATACONF
	echo "password: $MYSQL_PASS" >> $DATACONF
	echo "db: telepath" >> $DATACONF

}

usage() {
echo "Telepath configuration called with unknown argument '$1'
should be configure | upgrade | abort-upgrade | abort-remove | abort-deconfigure" >&2
}

case "$1" in
	configure)

		install

	;;
	upgrade|abort-upgrade|abort-remove|abort-deconfigure)
	;;

	check)

		check_installed

	;;

	*)

		#menu
        install
	;;

esac

	
telepath start
# Run Tor and Bad IPs list
php /opt/telepath/ui/html/index.php cron

clear
echo ""
echo "Telepath v3 installation completed!"
echo "Please open the browser for Telepath application at:"
echo "http://YOUR_SERVER_ADDRESS:8000/"

exit 0
