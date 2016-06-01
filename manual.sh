#!/bin/bash

# Dialog config
DIALOG=${DIALOG=dialog}
CMD="--ascii-lines --title "Telepath" --clear"

# Saving the SVN revision.
VER=`svn info | grep Revision | sed  's/Revision: //'`

BUILD_VER="1"
OS=
OS_VER=

# Check for centos
if [ -f "/etc/redhat-release" ]; then

	OS_MAJOR_VERSION=`sed -rn 's/.*([0-9])\.[0-9].*/\1/p' /etc/redhat-release`
	OS_MINOR_VERSION=`sed -rn 's/.*[0-9].([0-9]).*/\1/p' /etc/redhat-release`

	OS="CENTOS"
	OS_VER=$OS_MAJOR_VERSION.$OS_MINOR_VERSION
	BUILD_DIR="./build_tmp"

fi

# Check for ubuntu
if [ ! $CENTOS ]; then
	if [ -n "$(which lsb_release)" ]; then
		# Saving the UBUNTU version.
		OS="UBUNTU"
		OS_VER=`lsb_release -a 2> /dev/null | grep Release | sed  's/Release:\s*//'`
		BUILD_DIR="./debian/opt/telepath"

	fi
fi

if [ ! $OS ]; then
	echo "Unidentified operating system"
	exit 1;
else
	echo "Detected $OS $OS_VER $BUILD_DIR"
fi

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
VERSION='2.5'

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
	echo "Requires: boost-serialization, dialog, httpd, php, php-mysql, mysql, mysql-server, GeoIP"
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
	echo "killall gulp atms engine telewatchdog > /dev/null 2>&1 || true"
    echo
    echo "%files"
    echo "/"
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

# Make sure we have the dialog package
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

build_directory() {

	if [ -d "$BUILD_DIR" ]; then
		rm -rf $BUILD_DIR
	fi

	# Init directory structure

	mkdir -p $BUILD_DIR
	mkdir -p $BUILD_DIR/bin
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

	# Install Script
	cp ./configure.sh $BUILD_DIR/configure.sh
	chmod 755 $BUILD_DIR/configure.sh

	# Sql structure (depr)
	cp engine/Telepath.sql $BUILD_DIR/db/telepath.sql
	
	# Sql structure (new)
	rsync --exclude .svn -a db/tables $BUILD_DIR/db/
	rsync --exclude .svn -a db/procedures $BUILD_DIR/db/
	rsync --exclude .svn -a db/data $BUILD_DIR/db/

	# Default config file
	cp install/atms.conf $BUILD_DIR/conf/atms-default.conf

	if [ ! -f "./GeoLiteCity.dat" ]; then
		echo "GeoLite Missing, downloading.."
		wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
		gunzip -f ./GeoLiteCity.dat.gz
        fi

	# Copy GeoIP
	cp ./GeoLiteCity.dat $BUILD_DIR/db/geoip

	# Watchdog handle
	cp install/watchdog_handle $BUILD_DIR/bin/
	chmod 755 $BUILD_DIR/bin/watchdog_handle

	# Httpd configuration
	cp install/apache-telepath.conf $BUILD_DIR/conf/
	chmod 644 $BUILD_DIR/conf/apache-telepath.conf

	# Copy atms
	cp ./atms3/atms $BUILD_DIR/bin
	strip --strip-unneeded $BUILD_DIR/bin/atms
	chmod 755 $BUILD_DIR/bin/atms

	# Copy gulp
	cp ./atms3/gulp $BUILD_DIR/bin/gulp
	strip --strip-unneeded $BUILD_DIR/bin/gulp
	chmod 755 $BUILD_DIR/bin/gulp

	# Copy engine
	cp ./engine/engine $BUILD_DIR/bin
	strip --strip-unneeded $BUILD_DIR/bin/engine
	chmod 755 $BUILD_DIR/bin/engine

	# Copy log engine
	# cp ./engine/log_engine $BUILD_DIR/bin
	# strip --strip-unneeded $BUILD_DIR/bin/log_engine
	# chmod 755 $BUILD_DIR/bin/log_engine

	# Copy webservice engine
	# cp ./engine/webservice_engine $BUILD_DIR/bin
	# strip --strip-unneeded $BUILD_DIR/bin/webservice_engine
	# chmod 755 $BUILD_DIR/bin/webservice_engine

	# Copy telepath binary
	cp ./engine/telepath $BUILD_DIR/bin
	chmod 755 $BUILD_DIR/bin/telepath

	# Copy telepath.d
	cp ./engine/telepath.d $BUILD_DIR/bin
	chmod 755 $BUILD_DIR/bin/telepath.d

	# Copy telewatchdog
	cp ./watchdog/telewatchdog $BUILD_DIR/bin
	strip --strip-unneeded $BUILD_DIR/bin/telewatchdog
	chmod 755 $BUILD_DIR/bin/telewatchdog

	# Copy scheduler
	cp ./engine/scheduler $BUILD_DIR/bin
	chmod 755 $BUILD_DIR/bin/scheduler

	# Copy UI (v2 || v3)

	BUILD_UI_DIR='ui_v2'
	case $BUILD_VER in
        1)
		BUILD_UI_DIR='ui_v2'
        ;;
        2)
		BUILD_UI_DIR='ui_v3'
        ;;
	esac	
	
	rsync --exclude .svn --exclude Gui --exclude Docs --exclude metadata --exclude files -a $BUILD_UI_DIR/ $BUILD_DIR/ui/html
	
}

clear_binaries() {

	rm ./atms3/atms
	rm ./engine/engine
	rm ./engine/log_engine
	rm ./engine/webservice_engine
	rm ./engine/scheduler
	rm ./watchdog/telewatchdog

}

compile_all() {

	# If there is no atms file in the right location
	# then try building/compiling a new one.
	if [ ! -f "./atms3/atms" ]; then
	  echo "Compiling atms"
	  cd ./atms3
	  bash ./compile.sh
	  cd ..
	fi

	if [ ! -f "./atms3/atms" ]; then
	  $DIALOG $CMD --msgbox "Failed compiling atms" 5 40
	  exit 0
	fi

	# If there is no engine file in the right location
	# then try building/compiling a new one.
	if [ ! -f "./engine/engine" ]; then
	  echo "Compiling engine"
	  cd ./engine
	  make clean
	  make engine_trial
	  cd ..
	fi

	if [ ! -f "./engine/engine" ]; then
	  $DIALOG $CMD --msgbox "Failed compiling engine" 5 40
	  exit 0
	fi

	# If there is no log_engine file in the right location
	# then try building/compiling a new one.
	# if [ ! -f "./engine/log_engine" ]; then
	#  echo "Compiling log_engine"
	#  cd ./engine
	#  make clean
	#  make engine_logger
	#  cd ..
	# fi

	# if [ ! -f "./engine/log_engine" ]; then
	#  $DIALOG $CMD --msgbox "Failed compiling log engine" 5 40
	#  exit 0
	# fi

	# If there is no webservice_engine file in the right location
	# then try building/compiling a new one.
	# if [ ! -f "./engine/webservice_engine" ]; then
	#  echo "Compiling webservice_engine"
	#  cd ./engine
	#  make clean
	#  make engine_webservice
	#  cd ..
	# fi

	#if [ ! -f "./engine/webservice_engine" ]; then
	#  $DIALOG $CMD --msgbox "Failed compiling webservice engine" 5 40
	#  exit 0
	# fi

	# If there is no scheduler file in the right location 
	# then try building/compiling a new one.
	if [ ! -f "./engine/scheduler" ]; then
	  echo "Compiling scheduler"
	  cd ./engine
	  make clean
	  make scheduler
	  cd ..
	fi

	if [ ! -f "./engine/scheduler" ]; then
	  $DIALOG $CMD --msgbox "Failed compiling scheduler" 5 40
	  exit 0
	fi


	# If there is no telewatchdog file in the right location 
	# then try building/compiling a new one.
	if [ ! -f "./watchdog/telewatchdog" ]; then
	  echo "Compiling watchdog"
	  cd ./watchdog
	  make clean
	  make telewatchdog
	  cd ..
	fi

	if [ ! -f "./watchdog/telewatchdog" ]; then
	  $DIALOG $CMD --msgbox "Failed compiling telewatchdog" 5 40
	  exit 0
	fi

}

check_binaries() {
	
	result=""
	
	if [ ! -f "./atms3/atms" ]; then
		result="$result ATMS"
	fi
	if [ ! -f "./engine/engine" ]; then
		result="$result ENGINE"
	fi
	#if [ ! -f "./engine/log_engine" ]; then
	#	result="$result LOG_ENGINE"
	#fi
	#if [ ! -f "./engine/webservice_engine" ]; then
	#	result="$result WEBSERVICE_ENGINE"
	#fi
	if [ ! -f "./engine/scheduler" ]; then
		result="$result SCHEDULER"
	fi
	if [ ! -f "./watchdog/telewatchdog" ]; then
		result="$result TELEWATCHDOG"
	fi
	echo "$result"
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

menu() {

validate_dialog

retv=$?

# CENTOS


choice=$(dialog --stdout --ascii-lines --backtitle "SVN $VER || $OS $OS_VER || Building Telepath $build_ver" --title "Telepath build utility 0.4" \
--menu "Please choose an option:" 17 70 15 \
1 "Check for compiled binaries" \
2 "Compile missing binaries" \
3 "Recompile all binaries" \
4 "Create a tarball archive" \
5 "Create an install package (RPM/DEB)" \
6 "Update GeoLiteCity.dat from Maxmind servers" \
7 "SVN Update" \
8 "Build UI Update" \
9 "Update local repository and RSYNC with remote (RPM/CENTOS)" \
10 "RSYNC builds and update remote repository (DEB/UBUNTU)")


[ $retv -eq 1 -o $retv -eq 255 ] && exit

case $choice in
	1)

	display_missing
	menu

	;;
	2)

	compile_all
	display_missing
	menu

	exit
	;;
	3)

	clear_binaries
	compile_all
	display_missing
	menu

	exit
	;;
	4)

	BUILD_DIR="./build_tmp"
	build_directory
	create_tarball

	exit
	;;

	5)

	build_directory

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

	fi

	if [ $OS == "UBUNTU" ]; then

		sed -e "s/Version: 2.1.1/Version: 2.5."$VER"ubuntu"$OS_VER"/" debian/control > ./debian/DEBIAN/control
		cp ./debian/postrm ./debian/DEBIAN
		chmod 755 ./debian/DEBIAN/postrm

		cp ./debian/postinst ./debian/DEBIAN
		chmod 755 ./debian/DEBIAN/postinst

		rsync --exclude .svn -a debian/ debian-build
		rm -rf debian-build/control
		dpkg-deb --build debian-build > /dev/null 2>&1
		mv debian-build.deb "telepath_amd64_2.5."$VER"ubuntu"$OS_VER".deb"

		exit
		rm -rf debian-build

		clear
		echo "Complete, your package is telepath_amd64_2.5."$VER"ubuntu"$OS_VER".deb"
		echo "Have a nice day!"

	fi

	exit
	;;

	6)

	wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
	gunzip -f ./GeoLiteCity.dat.gz

	$DIALOG $CMD --msgbox "GeoIP database updated" 5 40

	menu

	;;

	7)

	svn up
	VER=`svn info | grep Revision | sed  's/Revision: //'`

	menu

	;;
	
	8)

	VER=`svn info | grep Revision | sed  's/Revision: //'`
	clear
	read -p "Build UI Update package version .$VER ? " -n 1 -r
	
	echo
	if [[ ! $REPLY =~ ^[Yy]$ ]]
		
		rm -rf ui_package
		mkdir ui_package
		cp -R ui_v2 ui_package
		cp -R ui_v3 ui_package
		tar -czf ui_update_${VER}.tar -C ui_package .
		
		# RSync
		echo "Please login hybridsec to authenticate rsync"
		rsync -apvzW ./ui_update*.tar root@hybridsec.com:/var/www/updates/
		
		#rm -rf build_tmp
		
		
	then
		exit 1
	fi

	;;

	9)

	if [ ! $OS == "CENTOS" ]; then

		$DIALOG $CMD --msgbox "You can do this only from centos" 5 40
		menu

	else

		# Update local repo
		echo "Moving RPMs"
		mv *.rpm /var/www/html/repo
		# Create Repo
		echo "Creating / Updating repo"
		createrepo /var/www/html/repo
		# RSync
		echo "rsyncing.."
		rsync -avzW /var/www/html/repo/* root@hybridsec.com:/var/www/repo/yum/

	fi

	;;

	10)

	if [ ! $OS == "UBUNTU" ]; then

		$DIALOG $CMD --msgbox "You can do this only from ubuntu" 5 40
		menu

	else

	# RSync
	echo "Please login hybridsec to authenticate rsync"
	rsync -avzW ./*.deb root@hybridsec.com:/root/

	# Running remote repo update script
	echo "Please login hybridsec to run publish script"
	ssh root@hybridsec.com "cd /root/ && ./publish.sh"

	rm ./*.deb

	fi

	;;


esac

}

BUILD_VER=$(dialog --stdout --ascii-lines --backtitle "SVN $VER || $OS $OS_VER" --title "Telepath build utility 0.4" \
--menu "Choose your build version:" 17 70 15 \
1 "Telepath v2" \
2 "Telepath v3")

menu

