#/bin/bash

PCAP="/usr/lib/libpcap.a"
CRYPTO="/usr/lib/libcrypto.a"
MYSQL="/usr/lib/libmysqlclient.a"
ZLIB="/usr/lib/libz.a"
if [ -f /usr/lib/x86_64-linux-gnu/libpcap.a ]; then
  PCAP="/usr/lib/x86_64-linux-gnu/libpcap.a"
fi
if [ -f /usr/lib/x86_64-linux-gnu/libcrypto.a ]; then
  CRYPTO="/usr/lib/x86_64-linux-gnu/libcrypto.a"
fi
if [ -f /usr/lib/x86_64-linux-gnu/libmysqlclient.a ]; then
  MYSQL="/usr/lib/x86_64-linux-gnu/libmysqlclient.a"
fi
if [ -f /usr/lib/x86_64-linux-gnu/libz.a ]; then
  ZLIB="/usr/lib/x86_64-linux-gnu/libz.a"
fi


if [ -f /usr/lib64/libpcap.a ]; then
  PCAP="/usr/lib64/libpcap.a"
fi
if [ -f /usr/lib64/libcrypto.a ]; then
  CRYPTO="/usr/lib64/libcrypto.a"
fi
if [ -f /usr/lib64/mysql/libmysqlclient.a ]; then
  MYSQL="/usr/lib64/mysql/libmysqlclient.a"
fi
if [ -f /usr/lib64/libz.a ]; then
  ZLIB="/usr/lib64/libz.a"
fi


#VER=`svnversion -n .`
VER=`svn info | grep Revision | sed  's/Revision: //'`
SVNDEV="-DSVN_REV=\""$VER"\""
echo "Building Version: " $SVNDEV
#COMP COMP="-g -std=gnu++0x -c -Wall"
COMP="-g -c -Wall"
COMP2="-g -c -Wall -I src/ -I/usr/include/mysql/ -L/usr/include/"
echo "building bro"
cd bro
g++ -o ip.o $COMP -I. ip.cpp
g++ -o tcp.o $COMP -I. tcp.cpp
g++ -o zip.o $COMP -I. zip.cpp
g++ -o mime.o $COMP -I. mime.cpp
g++ -o http.o $COMP -I. http.cpp
g++ -o frag.o $COMP -I. frag.cpp
g++ -o util.o $COMP -I. util.cpp
g++ -o ipaddr.o $COMP -I. ipaddr.cpp
g++ -o base64.o $COMP -I. base64.cpp
g++ -o reassem.o $COMP -I. reassem.cpp
g++ -o reporter.o $COMP -I. reporter.cpp
g++ -o net_util.o $COMP -I. net_util.cpp
g++ -o readfile.o $COMP -I. readfile.cpp
g++ -o contentline.o $COMP -I. contentline.cpp
g++ -o modp_numtoa.o $COMP -I. modp_numtoa.cpp
g++ -o ssl_wrapper.o $COMP -I. ssl_wrapper.cpp
g++ -o tcp_endpoint.o $COMP -I. tcp_endpoint.cpp
g++ -o bro_inet_ntop.o $COMP -I. bro_inet_ntop.cpp
g++ -o tcp_reassembler.o $COMP -I. tcp_reassembler.cpp
g++ -o tunnelencapsulation.o $COMP -I. tunnelencapsulation.cpp
cd ..
echo "building ssl"
g++ -o ssl/ssl_servers.o $COMP ssl/ssl_servers.cpp
g++ -o ssl/ssl_sessions.o $COMP ssl/ssl_sessions.cpp
g++ -o ssl/ssl_decoder.o $COMP ssl/ssl_decoder.cpp
g++ -o ssl/utils.o $COMP ssl/utils.cpp
echo "building json"
g++ -o json/json_value.o  $COMP -Isrc json/json_value.cpp
g++ -o json/json_reader.o $COMP -Isrc json/json_reader.cpp
g++ -o json/json_writer.o $COMP -Isrc json/json_writer.cpp
echo "building mxml"
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-attr.o mxml/mxml-attr.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-entity.o mxml/mxml-entity.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-file.o mxml/mxml-file.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-get.o mxml/mxml-get.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-index.o mxml/mxml-index.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-node.o mxml/mxml-node.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-search.o mxml/mxml-search.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-set.o mxml/mxml-set.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-private.o mxml/mxml-private.c
gcc $COMP -D_THREAD_SAFE -D_REENTRANT -o mxml/mxml-string.o mxml/mxml-string.c

echo "building code"
g++ $COMP -o ./src/atmstypes.o ./src/atmstypes.cpp
g++ $COMP -o ./src/backend.o ./src/backend.cpp
g++ $COMP -o ./src/base64.o ./src/base64.cpp
g++ $COMP -o ./src/hregex.o ./src/hregex.cpp
g++ $COMP -o ./src/log.o  ./src/log.cpp
g++ $COMP -o ./src/session.o ./src/session.cpp
g++ $COMP2 $SVNDEV -o ./src/serv.o ./src/serv.cpp
g++ $COMP2 -o ./src/parser.o ./src/parser.cpp
g++ $COMP -o ./src/sha1.o ./src/sha1.cpp
g++ $COMP -o ./src/sniffer.o ./src/sniffer.cpp
g++ $COMP2 -o ./src/sqlfeeder.o ./src/sqlfeeder.cpp
g++ $COMP -o ./src/sendarrayobj.o ./src/sendarrayobj.cpp
g++ $COMP -o ./src/config.o ./src/config.cpp
echo "final"
echo "g++ -g -Wall -I src/ -oatms -L/usr/lib/x86_64-linux-gnu/ ./src/atmstypes.o ./src/backend.o ./src/base64.o ./src/hregex.o ./src/log.o ./src/session.o ./src/parser.o ./src/serv.o ./src/sha1.o ./src/sniffer.o ./src/sqlfeeder.o ./src/sendarrayobj.o ./src/config.o json/json_reader.o json/json_value.o json/json_writer.o mxml/mxml-attr.o mxml/mxml-entity.o mxml/mxml-file.o mxml/mxml-get.o mxml/mxml-index.o mxml/mxml-node.o mxml/mxml-search.o mxml/mxml-set.o mxml/mxml-private.o mxml/mxml-string.o bro/ip.o bro/tcp.o bro/zip.o bro/mime.o bro/http.o bro/frag.o bro/util.o bro/ipaddr.o bro/base64.o bro/reassem.o bro/reporter.o bro/net_util.o bro/readfile.o bro/contentline.o bro/ssl_wrapper.o bro/modp_numtoa.o bro/tcp_endpoint.o bro/bro_inet_ntop.o bro/tcp_reassembler.o bro/tunnelencapsulation.o ssl/ssl_servers.o ssl/ssl_sessions.o ssl/ssl_decoder.o ssl/utils.o -static-libgcc -static-libstdc++ $MYSQL $CRYPTO $PCAP $ZLIB -ldl -lpthread"
g++ -g -Wall -I src/ -oatms -L/usr/lib/x86_64-linux-gnu/ ./src/atmstypes.o ./src/backend.o ./src/base64.o ./src/hregex.o ./src/log.o ./src/session.o ./src/parser.o ./src/serv.o ./src/sha1.o ./src/sniffer.o ./src/sqlfeeder.o ./src/sendarrayobj.o ./src/config.o json/json_reader.o json/json_value.o json/json_writer.o mxml/mxml-attr.o mxml/mxml-entity.o mxml/mxml-file.o mxml/mxml-get.o mxml/mxml-index.o mxml/mxml-node.o mxml/mxml-search.o mxml/mxml-set.o mxml/mxml-private.o mxml/mxml-string.o bro/ip.o bro/tcp.o bro/zip.o bro/mime.o bro/http.o bro/frag.o bro/util.o bro/ipaddr.o bro/base64.o bro/reassem.o bro/reporter.o bro/net_util.o bro/readfile.o bro/contentline.o bro/ssl_wrapper.o bro/modp_numtoa.o bro/tcp_endpoint.o bro/bro_inet_ntop.o bro/tcp_reassembler.o bro/tunnelencapsulation.o ssl/ssl_servers.o ssl/ssl_sessions.o ssl/ssl_decoder.o ssl/utils.o -static-libgcc -static-libstdc++ $MYSQL $CRYPTO $PCAP $ZLIB -ldl -lpthread

#chmod +s ./atms
