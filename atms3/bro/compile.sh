#/bin/sh

g++ -g -o contentline.o -std=gnu++0x -c -Wall -I. contentline.cpp
g++ -g -o mime.o -std=gnu++0x -c -Wall -I. mime.cpp
g++ -g -o base64.o -std=gnu++0x -c -Wall -I. base64.cpp
g++ -g -o http.o -std=gnu++0x -c -Wall -I. http.cpp
g++ -g -o zip.o -std=gnu++0x -c -Wall -I. zip.cpp

g++ -g -o frag.o -std=gnu++0x -std=gnu++0x -c -Wall -I. frag.cpp
g++ -g -o util.o -std=gnu++0x -c -Wall -I. util.cpp
g++ -g -o tcp.o -std=gnu++0x -c -Wall -I. tcp.cpp
g++ -g -o ip.o -std=gnu++0x -c -Wall -I. ip.cpp
g++ -g -o ipaddr.o -std=gnu++0x -c -Wall -I. ipaddr.cpp
g++ -g -o reassem.o -std=gnu++0x -c -Wall -I. reassem.cpp
g++ -g -o reporter.o -std=gnu++0x -c -Wall -I. reporter.cpp
g++ -g -o tcp_endpoint.o -std=gnu++0x -c -Wall -I. tcp_endpoint.cpp
g++ -g -o tcp_reassembler.o -std=gnu++0x -c -Wall -I. tcp_reassembler.cpp
g++ -g -o tunnelencapsulation.o -std=gnu++0x -c -Wall -I. tunnelencapsulation.cpp
g++ -g -o bro_inet_ntop.o -std=gnu++0x -c -Wall -I. bro_inet_ntop.cpp
g++ -g -o modp_numtoa.o -std=gnu++0x -c -Wall -I. modp_numtoa.cpp
g++ -g -o net_util.o -std=gnu++0x -c -Wall -I. net_util.cpp
g++ -g -o readfile.o -std=gnu++0x -c -Wall -I. readfile.cpp
g++ -g -o main.o -std=gnu++0x -c -Wall -I. main.cpp

g++ -g -L/usr/include/ -g -o"snif" frag.o util.o tcp.o ip.o ipaddr.o reassem.o reporter.o tcp_endpoint.o tcp_reassembler.o tunnelencapsulation.o bro_inet_ntop.o modp_numtoa.o net_util.o readfile.o main.o contentline.o mime.o base64.o http.o zip.o -lz -lpcap
#g++ -g -o src/json/json_value.o -c -Wall -Isrc src/json/json_value.cpp
#g++ -g -o src/json/json_writer.o -c -Wall -Isrc src/json/json_writer.cpp

#gcc -fPIC -Os -g -Wall -D_THREAD_SAFE -D_REENTRANT -c -g -o src/mxml/mxml-attr.o src/mxml/mxml-attr.c

