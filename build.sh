#!/bin/bash

#source compiled.list

#VER=`svn info | grep Revision | sed  's/Revision: //'`
VER=`git rev-parse HEAD | grep ... | sed -e 's/^\(.\{4\}\).*/\1/'`
BUILD_VER="1"
OS=
OS_VER=
ARCH=''
DESCRIPTION='Hybrid Telepath'
GROUP='Applications'
LICENSE='Restricted'
NAME='telepath'
PRINTSPEC=false
RELEASE=$(date +%Y.%m.%d+%S)
SUMMARY='Hybrid Telepath'
TARFILE=$(pwd)"/telepath.tar"
TARGET='/opt/telepath'
URL='http://www.hybridsec.com'


# The version needs to be set in the command line
ARG=$1

if [ -z "$1" ]; then
    echo "Please choose a version"
    exit 1
fi

VERSION="2.9"

if [ "$ARG" == "v3" ]; then
	echo "Choosed v3"
	VERSION="3.0"
elif [ "$ARG" == "v2" ]; then
	echo "Choosed v2"
	VERSION="2.9"
else
	echo "Please choose a valid version (v2 || v3)"
	exit 1
fi


# Check for centos
if [ -f "/etc/redhat-release" ]; then

	OS_MAJOR_VERSION=`sed -rn 's/.*([0-9])\.[0-9].*/\1/p' /etc/redhat-release`
	OS_MINOR_VERSION=`sed -rn 's/.*[0-9].([0-9]).*/\1/p' /etc/redhat-release`

	OS="CENTOS"
	OS_VER=$OS_MAJOR_VERSION.$OS_MINOR_VERSION
	BUILD_DIR="./build_tmp"
	echo "CENTOS Detected"
fi

# Check for ubuntu
if [ ! $CENTOS ]; then
	if [ -n "$(which lsb_release)" ]; then
		# Saving the UBUNTU version.
		OS="UBUNTU"
		OS_VER=`lsb_release -a 2> /dev/null | grep Release | sed  's/Release:\s*//'`
		BUILD_DIR="./debian/opt/telepath"

		echo "UBUNTU Detected"

	fi
fi

if [ ! $OS ]; then
	echo "Unidentified operating system"
	exit 1;
else
	echo "Detected $OS $OS_VER $BUILD_DIR"
fi

function spec {

	RELEASE=$(date +%Y.%m.%d).$VER

    echo "Name: $NAME"
    echo "Summary: $SUMMARY"
    echo "Version: $VERSION"
    echo "Group: $GROUP"
    echo "License: $LICENSE"
    echo "Release: $RELEASE"
    echo "Prefix: $TARGET"
    echo "BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-$(whoami)-%(%{__id_u} -n)"

	# CENTOS INSTALL DEPENDENCIES
	# echo "CENTOS INSTALL DEPENDENCIES"
	# V3
	if [ $ARG == "v3" ]; then
		echo "Requires: boost-serialization, dialog, php, GeoIP, libyaml, libnet, java-1.8.0-openjdk, redis, php-redis, wget"
	# V2
	elif [ $ARG == "v2" ]; then
		echo "Requires: boost-serialization, dialog, php, GeoIP, libyaml, libnet, redis, wget"
	fi

    if [ -n "$ARCH" ]; then
        echo "BuildArch: $ARCH"
    elif [ -n "$URL" ]; then
        echo "URL: $URL"
    fi
    echo
    echo "%description"
    echo $DESCRIPTION
	echo
	echo "%post"
	echo "/opt/telepath/configure.sh check"
    echo "%build"
    echo "cp $TARFILE %{_builddir}/archive.tar"
    echo
    echo "%install"
    echo "mkdir -p \$RPM_BUILD_ROOT$TARGET"
    echo "mv archive.tar \$RPM_BUILD_ROOT/archive.tar"
    echo "cd \$RPM_BUILD_ROOT$TARGET"
    echo "tar -xf \$RPM_BUILD_ROOT/archive.tar"
    echo "rm \$RPM_BUILD_ROOT/archive.tar"
    echo
    echo "%clean"
    echo "rm -fr \$RPM_BUILD_ROOT"
	echo "rm -rf /etc/httpd/conf.d/apache-telepath.conf > /dev/null 2>&1 || true"
	echo "rm -rf /etc/init.d/telepath.d > /dev/null 2>&1 || true"
    echo
    echo "%files"
    while IFS='/' read -ra pathSegments; do
        path=''
        for pathSegment in ${pathSegments[@]}; do
            path="$path/$pathSegment"
            echo $path
        done
    done <<< $TARGET
    while read -ra fileNames; do
        for fileName in ${fileNames[@]}; do
            echo $TARGET/$fileName
        done
    done <<< $(tar -tf $TARFILE)
}

build_directory() {
	
	echo "Building DIR"

	if [ -d "$BUILD_DIR" ]; then
		rm -rf $BUILD_DIR
		echo "$BUILD_DIR removed"
	fi

	# Init directory structure
	mkdir -p $BUILD_DIR
	mkdir -p $BUILD_DIR/bin
	mkdir -p $BUILD_DIR/scripts
	mkdir -p $BUILD_DIR/conf
	mkdir -p $BUILD_DIR/ui/html
	mkdir -p $BUILD_DIR/db/kb_bin
	mkdir -p $BUILD_DIR/db/geoip
	mkdir -p $BUILD_DIR/db/kb_bin/Backup
	mkdir -p $BUILD_DIR/db/kb_bin/attributes
	mkdir -p $BUILD_DIR/db/kb_bin/behavior
	mkdir -p $BUILD_DIR/db/kb_bin/flows
	mkdir -p $BUILD_DIR/db/kb_bin/globals
	mkdir -p $BUILD_DIR/db/kb_bin/paths
	mkdir -p $BUILD_DIR/db/kb_bin/user_attributes
	mkdir -p $BUILD_DIR/db/kb_bin/user_paths
	mkdir -p $BUILD_DIR/db/kb_bin/Temp
	echo "Init directories built"

	# MKDIR Suricata
	mkdir -p $BUILD_DIR/suricata
	mkdir -p $BUILD_DIR/suricata/logs
	rsync --exclude .svn -a suricata $BUILD_DIR/
	echo "Suricata directories built"

        # MKDIR Openresty
	rsync --exclude .svn -a openresty $BUILD_DIR/


	# Sql structure (new)
	rsync --exclude .svn -a db/tables $BUILD_DIR/db/
	rsync --exclude .svn -a db/procedures $BUILD_DIR/db/
	rsync --exclude .svn -a db/data $BUILD_DIR/db/
	echo "SQL directories built"

	# Default config file
	cp install/atms.conf $BUILD_DIR/conf/atms-default.conf
	echo "ATMS config copied"


	echo "Geocity START"
	if [ ! -f "./GeoLiteCity.dat" ]; then
		echo "GeoLite Missing, downloading.."
		wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
		gunzip -f ./GeoLiteCity.dat.gz
    fi

	# Copy GeoIP
	cp ./GeoLiteCity.dat $BUILD_DIR/db/geoip
	echo "Geocity END"


	echo "Copy binaries START"
	# Httpd configuration
	cp install/apache-telepath.conf $BUILD_DIR/conf/
	chmod 644 $BUILD_DIR/conf/apache-telepath.conf

	cp ./../$SVNDIR/engine $BUILD_DIR/bin
	cp ./../$SVNDIR/watchdog/telewatchdog $BUILD_DIR/bin
	cp ./../$SVNDIR/scheduler $BUILD_DIR/bin

	echo "Copied binaries"
		
	chmod 755 $BUILD_DIR/bin/scheduler

	strip --strip-unneeded $BUILD_DIR/bin/engine
	chmod 755 $BUILD_DIR/bin/engine

	# Copy telewatchdog
	strip --strip-unneeded $BUILD_DIR/bin/telewatchdog
	chmod 755 $BUILD_DIR/bin/telewatchdog
	echo "Copied watchdog"

	# Copy telepath binary
	cp ./engine/telepath $BUILD_DIR/bin
	chmod 755 $BUILD_DIR/bin/telepath
	echo "Copied Telepath binaries"

	# Copy telepath.d
	cp ./engine/telepath.d $BUILD_DIR/bin
	chmod 755 $BUILD_DIR/bin/telepath.d
	echo "Copied Telepath.d"

	# Copy scheduler
	#cp ./engine/scheduler $BUILD_DIR/bin
	#chmod 755 $BUILD_DIR/bin/scheduler
	#echo "Copied scheduler"
	#echo "Copy binaries END"

	# FORK V2 || V3 requirements and config

	BUILD_UI_DIR='ui_v2'

	if [ "$ARG" == "v3" ]; then
		echo "Copy UI v3 START"
		# COPY UI DIR
		BUILD_UI_DIR='ui_v3'

		# V3 Elastic config dir
		mkdir -p $BUILD_DIR/db/elasticsearch
		echo "Elasticsearch directory created, copy default rules"
		#cp ./db/rules.py $BUILD_DIR/db/elasticsearch/rules.py
		#cp ./db/rules.json $BUILD_DIR/db/elasticsearch/rules.json



	elif [ "$ARG" == "v2" ]; then
		echo "Copy UI v2 START"
		# COPY UI DIR
		BUILD_UI_DIR='ui_v2'

	fi

	echo "Copy UI END"

	cp ./scripts/export.sh $BUILD_DIR/scripts/export.sh
        chmod 755 $BUILD_DIR/scripts/export.sh
        echo "Copied export.sh"

        cp ./scripts/import.sh $BUILD_DIR/scripts/import.sh
        chmod 755 $BUILD_DIR/scripts/import.sh
        echo "Copied import.sh"

	cp ./network_config.sh $BUILD_DIR/network_config.sh
        chmod 755 $BUILD_DIR/network_config.sh
        echo "Copied network_config.sh"

	cp ./.networkconf $BUILD_DIR/.networkconf
        chmod 755 $BUILD_DIR/.networkconf
        echo "Copied .networkconf"

	cp ./generate-ssl.sh $BUILD_DIR/generate-ssl.sh
        chmod 755 $BUILD_DIR/generate-ssl.sh
        echo "Copied generate-ssl.sh"

	cp ./reset.sh $BUILD_DIR/reset.sh
        chmod 755 $BUILD_DIR/reset.sh
        echo "Copied reset.sh"

	cp ./configure.sh $BUILD_DIR/configure.sh
	chmod 755 $BUILD_DIR/configure.sh
	echo "Copied configure.sh"
	echo "RSYNC"
	echo rsync --exclude .svn --exclude Gui --exclude Docs --exclude metadata --exclude files -a $BUILD_UI_DIR/ $BUILD_DIR/ui/html
	rsync --exclude .svn --exclude Gui --exclude Docs --exclude metadata --exclude files -a $BUILD_UI_DIR/ $BUILD_DIR/ui/html

	# Copy captcha script
	cp ./captcha.php $BUILD_DIR/ui

}

clear_binaries() {

	if [ ! -f "./engine/engine" ]; then
		rm ./engine/engine
	fi
	if [ ! -f "./engine/scheduler" ]; then
		rm ./engine/scheduler
	fi
	if [ ! -f "./watchdog/telewatchdog" ]; then
		rm ./watchdog/telewatchdog
	fi
	echo "Binaries cleared"
}

compile_all() {
	echo "Compiling"

	# #################################
	# ENGINE
	# #################################

	cd ./../
	cd $SVNDIR

	echo "Compiling ENGINE"
	make clean

	
	# TODO: V2 || V3
	
	make engine_trial

	if [ ! -f "./engine" ]; then
		echo "ENGINE compile failed, aborting"
		exit
	fi
		
	echo "Compiling WATCHDOG"
	cd watchdog
	make clean
	make telewatchdog
	
	if [ ! -f "./telewatchdog" ]; then
		echo "WATCHDOG compile failed, aborting"
		exit
	fi

	cd ../

	echo "Compiling SCHEDULER"
	make clean
	make scheduler

	if [ ! -f "./scheduler" ]; then
		echo "SCHEDULER compile failed, aborting"
		exit
	fi

	cd ../../trunk
	
}

check_binaries() {

	result=""

	if [ ! -f "./engine/engine" ]; then
		result="$result ENGINE"
	fi
	if [ ! -f "./engine/scheduler" ]; then
		result="$result SCHEDULER"
	fi
	if [ ! -f "./watchdog/telewatchdog" ]; then
		result="$result TELEWATCHDOG"
	fi
	echo "$result"

	echo "Binaries checked"
}

display_missing() {

	missing_binaries=$(check_binaries)

	if [ "$missing_binaries" != "" ]; then
		$DIALOG $CMD --msgbox "Missing binaries ::$missing_binaries" 5 40
	else
		$DIALOG $CMD --msgbox 'All binaries in place' 5 40
	fi

}

create_tarball() {

	if [ -f telepath.tar ]; then
		rm telepath.tar
	fi

	tar -cf telepath.tar -C build_tmp .
	rm -rf build_tmp
	clear
	echo "telepath.tar created."

}

# UPDATE SVN

#cd ..
#svn up
#git pull
#cd trunk

#VER=`svn info | grep Revision | sed  's/Revision: //'`
VER=`git rev-parse HEAD | grep ... | sed -e 's/^\(.\{4\}\).*/\1/'`
# REMOVE BUILDS

#clear_binaries

# RECOMPILE
if [ "$ARG" == "v2" ]; then
	SVNDIR="tags/2.7.868/"
elif [ "$ARG" == "v3" ]; then
	SVNDIR="trunk/engine/"
fi

compile_all

# UPDATE GEOLITE

wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
gunzip -f ./GeoLiteCity.dat.gz

echo "GeoLiteCity updated"

# COMPILE A DIRECTORY

build_directory

# BUILD / RSYNC .RPM
if [ $OS == "CENTOS" ]; then

	create_tarball

	spec > /tmp/tar2rpm-$$.spec
	rpmbuild -bb /tmp/tar2rpm-$$.spec > /tmp/tar2rpm-$$.log 2>&1
	cat /tmp/tar2rpm-$$.log | grep -i "Wrote:.*$NAME-$VERSION-$RELEASE.*rpm"
	if [ $? -gt 0 ]; then
		echo "ERROR: RPM build failed. Check log: /tmp/tar2rpm-$$.log Spec file: /tmp/tar2rpm-$$.spec"
		exit 1
	else 
		TMP=$(cat /tmp/tar2rpm-$$.log | grep -i "Wrote:.*rpm")
		mv ${TMP/Wrote:/} .
	fi
	rm /tmp/tar2rpm-$$.spec
	rm /tmp/tar2rpm-$$.log

	# Update local repo
	echo "Moving RPMs"
	mv *.rpm /var/www/html/repo
	# Create Repo
	echo "Creating / Updating repo"
	createrepo /var/www/html/repo
	restorecon -r /var/www/html
	# RSync
	echo "rsyncing.."
	rsync -avzW /var/www/html/repo/* root@hybridsec.com:/var/www/html/repo/yum/
fi

# BUILD / RSYNC .DEB
if [ $OS == "UBUNTU" ]; then

	sed -e "s/Version: 3.0/Version: "$VERSION"."$VER"ubuntu"$OS_VER"/" debian/control > ./debian/DEBIAN/control

	cp ./debian/postrm ./debian/DEBIAN
	chmod 755 ./debian/DEBIAN/postrm

	cp ./debian/postinst ./debian/DEBIAN
	chmod 755 ./debian/DEBIAN/postinst

	rsync --exclude .svn -a debian/ debian-build
	rm -rf debian-build/control
	dpkg-deb --build debian-build > /dev/null 2>&1
	mv debian-build.deb "telepath_amd64_"$VERSION"."$VER"ubuntu"$OS_VER".deb"
	#rm -rf debian-build

	if [ "$ARG" == "v3" ]; then
		echo "V3 Detected"
		# VERSION 3 STAGING
		if [ "$2" == "staging" ]; then
			echo "V3 -> Staging Detected"
			#reprepro -b /var/www/html/repo_v3/apt/debian includedeb trusty *.deb
			echo "Please login hybridsec to authenticate rsync"
                        #read -rsp $'Press any key to continue...\n' -n1 key
                        rsync -avzW ./*.deb root@hybridsec.com:/root/
                        echo "Please login hybridsec to run publish script"
                        ssh root@hybridsec.com "cd /root/ && ./publish_staging.sh"
			rm ./*.deb
			echo "ENJOY YOUR NEW BUILD!!!"
		fi

		# VERSION 3 PRODUCTION
		if [ "$2" == "production" ]; then
			echo "V3 -> Production Detected"
			echo "Please login hybridsec to authenticate rsync"
			#read -rsp $'Press any key to continue...\n' -n1 key
			rsync -avzW ./*.deb root@hybridsec.com:/root/
			echo "Please login hybridsec to run publish script"
			ssh root@hybridsec.com "cd /root/ && ./publish.sh"
			rm ./*.deb
			echo "ENJOY YOUR NEW BUILD!!!"
		fi
	fi


	if [ "$ARG" == "v2" ]; then
		# VERSION 2 STAGING
		echo "V2 Detected"
		if [ "$2" == "staging" ]; then
			echo "V2-> Staging Detected"
			reprepro -b /var/www/html/repo_v2/apt/debian includedeb trusty *.deb
			rm ./*.deb
			echo "ENJOY YOUR NEW BUILD!!!"
		fi

		# VERSION 2 PRODUCTION
		if [ "$2" == "production" ]; then
			echo "V2-> Staging Detected"
			echo "Please login hybridsec to authenticate rsync"
			rsync -avzW ./*.deb root@hybridsec.com:/root/
			echo "Please login hybridsec to run publish script"
			ssh root@hybridsec.com "cd /root/ && ./publish.sh"
			rm ./*.deb
			echo "ENJOY YOUR NEW BUILD!!!"
		fi
	fi
fi
