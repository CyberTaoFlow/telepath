echo "----------------------------------"
echo "DOWNLOADING AND COMPILING SURICATA"
echo "----------------------------------"

rm -rf /tmp/suricata
mkdir /tmp/suricata
cd /tmp/suricata

if [ -n "$(which apt-get)" ]; then
        #apt-get --force-yes -y install wget git gcc libpcre3 libpcre3-dbg libpcre3-dev build-essential autoconf automake libtool libpcap-dev libnet1-dev libyaml-0-2 libyaml-dev pkg-config zlib1g zlib1g-dev libcap-ng-dev libcap-ng0
        #make libmagic-dev git libnetfilter-queue-dev libnetfilter-queue1 libnfnetlink-dev libnfnetlink0
        apt-get --force-yes -y install wget git gcc libpcre3 libpcre3-dbg libpcre3-dev build-essential autoconf automake libtool libpcap-dev libnet1-dev libyaml-0-2 libyaml-dev zlib1g zlib1g-dev libcap-ng-dev libcap-ng0 libnetfilter-queue-dev libnetfilter-queue1 libnfnetlink-dev libnfnetlink0 make libmagic-dev libjansson-dev libjansson4 pkg-config libnuma-dev flex bison subversion
       #apt-get             -y install              libpcre3 libpcre3-dbg libpcre3-dev build-essential autoconf automake libtool libpcap-dev libnet1-dev libyaml-0-2 libyaml-dev zlib1g zlib1g-dev libcap-ng-dev libcap-ng0                                                                           make libmagic-dev                                       libnuma-dev
       #apt-get             -y install      git     libpcre3 libpcre3-dbg libpcre3-dev build-essential autoconf automake libtool libpcap-dev libnet1-dev libyaml-0-2 libyaml-dev zlib1g zlib1g-dev libcap-ng-dev libcap-ng0                                                                           make libmagic-dev                            pkg-config libnuma-dev flex bison subversion 
fi

if [ -n "$(which yum)" ]; then
        yum -y install libtool git wget gcc libpcap-devel pcre-devel libyaml-devel file-devel zlib-devel jansson-devel nss-devel libcap-ng-devel libnet-devel
fi

rm -rf LuaJIT-2.0.0
wget http://luajit.org/download/LuaJIT-2.0.0.tar.gz
tar -zxf LuaJIT-2.0.0.tar.gz
cd LuaJIT-2.0.0
make && make install
cd ..
ldconfig

# download pfring
#git clone https://github.com/ntop/PF_RING.git
#cd PF_RING/kernel/
#make && sudo make install
#cd ../userland/lib
#./configure --prefix=/usr/local/pfring && make && sudo make install
#cd ../libpcap
#./configure --prefix=/usr/local/pfring && make && sudo make install
#cd ../tcpdump
#./configure --prefix=/usr/local/pfring && make && sudo make install
#sudo ldconfig

#modprobe pf_ring
#-----

#wget http://sourceforge.net/projects/ntop/files/PF_RING/PF_RING-6.0.3.tar.gz
#tar -zxf PF_RING-6.0.3.tar.gz
#cd PF_RING-6.0.3/
#make

#cd kernel; make install
#cd ../userland/lib; make install

#modprobe pf_ring

#rm -rf oisf
#git clone git://phalanx.openinfosecfoundation.org/oisf.git
#cd oisf
#git clone https://github.com/ironbee/libhtp.git -b 0.5.x
#./autogen.sh

#cd oisf
#rm -rf /opt/telepath/suricata/bin
cd /home
VER=3.2
wget "http://www.openinfosecfoundation.org/download/suricata-$VER.tar.gz"
tar -xvzf "suricata-$VER.tar.gz"
cd "suricata-$VER"


mkdir -p /opt/telepath/suricata/logs

#./configure --prefix=/opt/telepath/suricata/ --sysconfdir=/opt/telepath/suricata/ --localstatedir=/opt/telepath/suricata/ --disable-detection --enable-luajit --with-libnss-libraries=/usr/lib --with-libnss-includes=/usr/include/nss/ --with-libnspr-libraries=/usr/lib --with-libluajit-includes=/usr/local/include/luajit-2.0/ --with-libluajit-libraries=/usr/lib/x86_64-linux-gnu/ --enable-pfring --with-libpfring-includes=/usr/local/pfring/include --with-libpfring-libraries=/usr/lib/x86_64-linux-gnu/
./configure --prefix=/opt/telepath/suricata/ --sysconfdir=/opt/telepath/suricata/ --localstatedir=/opt/telepath/suricata/ --disable-detection --enable-luajit --with-libnss-libraries=/usr/lib --with-libnss-includes=/usr/include/nss/ --with-libnspr-libraries=/usr/lib --with-libluajit-includes=/usr/local/include/luajit-2.0/ --with-libluajit-libraries=/usr/lib/x86_64-linux-gnu/ 
make
make install
ln -s /opt/telepath/suricata/bin/suricata /opt/telepath/suricata/suricata
ldconfig

echo "DONE!"