
MSGPACK = -I./msgpack-c-master/include/
BOOST_SERIAL = /usr/lib/libboost_serialization.a
GeoIP = /usr/lib/libGeoIP.a
Redis = /usr/lib/x86_64-linux-gnu/libhiredis.a
Unordered = -std=gnu++11
Comp =  -g -Wall -O -c -o

GITDEV := -D'GIT_REV="$(shell git rev-parse HEAD | grep ... | sed -e 's/^\(.\{8\}\).*/\1/')"'
GITFLAG := $(GITDEV)
COUNTDEV := -D'COUNT_REV="$(shell cat /home/hybrid/trunk/.counter)"'
COUNTFLAG := $(COUNTDEV)
TIME2 := -D'COMPILATION_TIME=$(shell date +%s)'

CRYPTO=/usr/lib/libcrypto.a
PCRE=/usr/lib/libpcre.a 

LIBSTDC=/usr/lib/gcc/x86_64-redhat-linux/4.8.2/libstdc++.so

ifeq ($(wildcard $(LIBSTDC) ),)
        LIBSTDC = -static-libstdc++
endif

ifeq ($(wildcard $(GeoIP) ),) 
	GeoIP = /usr/lib/x86_64-linux-gnu/libGeoIP.a
endif

ifeq ($(wildcard $(GeoIP) ),)
        GeoIP = /usr/local/lib/libGeoIP.a
endif

ifeq ($(wildcard $(GeoIP) ),)
        GeoIP = /usr/lib64/libGeoIP.a
endif

ifeq ($(wildcard $(GeoIP) ),)
        GeoIP = /usr/lib64/libGeoIP.so
endif

ifeq ($(wildcard $(PCRE) ),) 
	PCRE = /usr/lib/x86_64-linux-gnu/libpcre.a
endif

ifeq ($(wildcard $(PCRE) ),) 
        PCRE = /usr/lib64/libpcre.a
endif

ifeq ($(wildcard $(PCRE) ),)
        PCRE = /usr/lib64/libpcre.so
endif

ifeq ($(wildcard $(CRYPTO) ),) 
       CRYPTO = /usr/lib/x86_64-linux-gnu/libcrypto.a
endif

ifeq ($(wildcard $(CRYPTO) ),)
        CRYPTO = /usr/lib64/libcrypto.a
endif

ifeq ($(wildcard $(CRYPTO) ),)
        CRYPTO = /usr/lib/libcrypto++.a
endif

ifeq ($(wildcard $(CRYPTO) ),)
        CRYPTO = /usr/lib64/libcrypto.so
endif

ifeq ($(wildcard $(Redis) ),)
        Redis = /usr/lib64/libhiredis.a
endif

ifeq ($(wildcard $(Redis) ),)
        Redis = /usr/lib/x86_64-linux-gnu/libhiredis.a
endif

ifeq ($(wildcard $(Redis) ),)
        Redis = /usr/lib64/libhiredis.so
endif

ifeq ($(wildcard $(BOOST_SERIAL) ),) 
	BOOST_SERIAL = /usr/lib64/libboost_serialization.a
endif

ifeq ($(wildcard $(BOOST_SERIAL) ),)
        BOOST_SERIAL = /usr/lib/x86_64-linux-gnu/libboost_serialization.a
endif

ifeq ($(wildcard $(BOOST_SERIAL) ),)
        BOOST_SERIAL = /usr/lib64/libboost_serialization.so
endif


scheduler: objects/scheduler.o
	g++ -Wall -o scheduler objects/scheduler.o -L./ -static-libgcc -lcurl

objects/scheduler.o: scheduler.cpp
	g++ -Wall -c -o objects/scheduler.o scheduler.cpp $(Unordered)




######

objects/teleindex_debug.o: teleindex.cpp
	g++ -g -O -c -o objects/teleindex_debug.o $(MSGPACK) -DVERSION3 -DDEBUG teleindex.cpp $(Unordered)

objects/teleindex_v3.o: teleindex.cpp
	g++ -g -O -c -o objects/teleindex_v3.o $(MSGPACK) -DVERSION3 teleindex.cpp $(Unordered)

objects/teletypes.o: teletypes.cpp 
	g++ -g -Wall -O -c -o objects/teletypes.o teletypes.cpp

objects/teleobject.o: teleobject.cpp 
	g++ -g -Wall -O -c -o objects/teleobject.o teleobject.cpp

######

objects/mxml-attr.o: mxml/mxml-attr.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-attr.o mxml/mxml-attr.c

objects/mxml-entity.o: mxml/mxml-entity.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-entity.o mxml/mxml-entity.c

objects/mxml-file.o: mxml/mxml-file.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-file.o mxml/mxml-file.c

objects/mxml-get.o: mxml/mxml-get.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-get.o mxml/mxml-get.c

objects/mxml-index.o: mxml/mxml-index.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-index.o mxml/mxml-index.c

objects/mxml-node.o: mxml/mxml-node.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-node.o mxml/mxml-node.c

objects/mxml-search.o: mxml/mxml-search.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-search.o mxml/mxml-search.c

objects/mxml-set.o: mxml/mxml-set.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-set.o mxml/mxml-set.c

objects/mxml-private.o: mxml/mxml-private.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-private.o mxml/mxml-private.c

objects/mxml-string.o: mxml/mxml-string.c
	gcc -g -c -Wall -D_THREAD_SAFE -D_REENTRANT -o objects/mxml-string.o mxml/mxml-string.c

mXML.a: objects/mxml-attr.o objects/mxml-entity.o objects/mxml-file.o objects/mxml-get.o objects/mxml-index.o objects/mxml-node.o objects/mxml-search.o objects/mxml-search.o objects/mxml-set.o objects/mxml-private.o objects/mxml-string.o
	ar r mXML.a objects/mxml-attr.o objects/mxml-entity.o objects/mxml-file.o objects/mxml-get.o objects/mxml-index.o objects/mxml-node.o objects/mxml-search.o objects/mxml-search.o objects/mxml-set.o objects/mxml-private.o objects/mxml-string.o

######

objects/base64.o: base64.cpp
	g++ $(Comp) objects/base64.o base64.cpp

objects/Syslog.o: Syslog.cpp 
	g++ $(Comp) objects/Syslog.o -DREDIS Syslog.cpp

objects/Node.o: Node.cpp 
	g++ $(Comp) objects/Node.o Node.cpp

objects/Edge.o: Edge.cpp 
	g++ $(Comp) objects/Edge.o Edge.cpp

objects/Tree.o: Tree.cpp 
	g++ $(Comp) objects/Tree.o Tree.cpp

objects/Numeric.o: Numeric.cpp 
	g++ $(Comp) objects/Numeric.o Numeric.cpp

objects/Enumeration.o: Enumeration.cpp 
	g++ $(Comp) objects/Enumeration.o Enumeration.cpp

objects/Parameter.o: Parameter.cpp 
	g++ $(Comp) objects/Parameter.o Parameter.cpp

objects/Query.o: Query.cpp 
	g++ $(Comp) objects/Query.o Query.cpp

objects/Coordinate.o: Coordinate.cpp 
	g++ $(Comp) objects/Coordinate.o Coordinate.cpp

objects/Cluster.o: Cluster.cpp 
	g++ $(Comp) objects/Cluster.o Cluster.cpp

objects/DB-SCAN.o: DB-SCAN.cpp 
	g++ $(Comp) objects/DB-SCAN.o DB-SCAN.cpp

objects/NodeExtended.o: NodeExtended.cpp 
	g++ $(Comp) objects/NodeExtended.o NodeExtended.cpp

objects/LinkExtended.o: LinkExtended.cpp 
	g++ $(Comp) objects/LinkExtended.o LinkExtended.cpp

objects/Page.o: Page.cpp 
	g++ $(Comp) objects/Page.o Page.cpp

objects/Link.o: Link.cpp 
	g++ $(Comp) objects/Link.o Link.cpp

objects/Core.o: Core.cpp 
	g++ $(Comp) objects/Core.o Core.cpp

objects/Session.o: Session.cpp 
	g++ $(Comp) objects/Session.o Session.cpp

objects/Operation.o: Operation.cpp 
	g++ $(Comp) objects/Operation.o Operation.cpp

objects/Range.o: Range.cpp 
	g++ $(Comp) objects/Range.o Range.cpp

objects/Rule.o: Rule.cpp 
	g++ $(Comp) objects/Rule.o Rule.cpp

objects/geoip.o: geoip.cpp 
	g++ $(Comp) objects/geoip.o geoip.cpp

objects/Path.o: Path.cpp 
	g++ $(Comp) objects/Path.o Path.cpp

objects/ScoreData.o: ScoreData.cpp 
	g++ $(Comp) objects/ScoreData.o ScoreData.cpp


Engine_v3.a: objects/teletypes.o objects/teleobject.o objects/base64.o objects/Syslog.o objects/Node.o objects/Edge.o objects/Tree.o objects/Numeric.o objects/Enumeration.o objects/Parameter.o objects/Query.o objects/Coordinate.o objects/Cluster.o objects/DB-SCAN.o objects/NodeExtended.o objects/LinkExtended.o objects/Page.o objects/Link.o objects/Core.o objects/Session.o objects/Operation.o objects/Range.o objects/Rule.o objects/geoip.o objects/Path.o objects/ScoreData.o
	ar r Engine_v3.a objects/teletypes.o objects/teleobject.o objects/base64.o objects/Syslog.o objects/Node.o objects/Edge.o objects/Tree.o objects/Numeric.o objects/Enumeration.o objects/Parameter.o objects/Query.o objects/Coordinate.o objects/Cluster.o objects/DB-SCAN.o objects/NodeExtended.o objects/LinkExtended.o objects/Page.o objects/Link.o objects/Core.o objects/Session.o objects/Operation.o objects/Range.o objects/Rule.o objects/geoip.o objects/Path.o objects/ScoreData.o



engine_v3: objects/teleindex_v3.o Engine_v3.a objects/engine_v3.o json.a mXML.a
	g++ -o engine objects/engine_v3.o -L./ -pthread -static-libgcc $(LIBSTDC) $(CRYPTO) -ldl $(BOOST_SERIAL) objects/teleindex_v3.o Engine_v3.a json.a mXML.a $(PCRE) -lcurl -lrt $(GeoIP) $(Redis)

objects/engine_v3.o: engine.cpp 
	g++ $(GITFLAG) $(COUNTFLAG) $(TIME2) -g -Wall -O -c -o objects/engine_v3.o -DREDIS -DVERSION3 $(MSGPACK) engine.cpp $(Unordered) 





engine_trial: objects/teleindex_v3.o Engine_v3.a objects/engine_trial.o json.a mXML.a
	g++ -o engine objects/engine_trial.o -L./ -pthread -static-libgcc $(LIBSTDC) $(CRYPTO) -ldl $(BOOST_SERIAL) objects/teleindex_v3.o Engine_v3.a json.a mXML.a $(PCRE) -lcurl -lrt $(GeoIP) $(Redis)

objects/engine_trial.o: engine.cpp
	g++ $(GITFLAG) $(COUNTFLAG) $(TIME2) -g -O -c -o objects/engine_trial.o -DREDIS -DVERSION3 -DTRIAL $(MSGPACK) engine.cpp $(Unordered)





engine_debug: objects/teleindex_debug.o Engine_v3.a objects/engine_debug.o json.a mXML.a
	g++ -o engine objects/engine_debug.o json.a -L./ -pthread -static-libgcc $(LIBSTDC) $(CRYPTO) -ldl $(BOOST_SERIAL) objects/teleindex_debug.o Engine_v3.a mXML.a $(PCRE) -lcurl -lrt $(GeoIP) $(Redis)

objects/engine_debug.o: engine.cpp
	g++ $(GITFLAG) $(COUNTFLAG) $(TIME2) -g -O -c -o objects/engine_debug.o -DREDIS -DVERSION3 -DTRIAL -DDEBUG $(MSGPACK) engine.cpp $(Unordered)





engine_logger: objects/teleindex_v3.o Engine_v3.a objects/engine_logger json.a mXML.a
	g++ -o log_engine objects/engine_logger -L./ -pthread -static-libgcc $(LIBSTDC) $(CRYPTO) -ldl $(BOOST_SERIAL) objects/teleindex_v3.o Engine_v3.a json.a mXML.a $(PCRE) -lrt $(GeoIP) 

objects/engine_logger.o: engine.cpp 
	g++ $(GITFLAG) $(COUNTFLAG) $(TIME2) -g -Wall -O -c -o objects/engine_logger -DLOGGER $(MSGPACK) engine.cpp $(Unordered)





objects/json_value.o:
	g++ -o objects/json_value.o -g -c jsoncpp/json_value.cpp

objects/json_reader.o:
	g++ -o objects/json_reader.o -g -c jsoncpp/json_reader.cpp

objects/json_writer.o:
	g++ -o objects/json_writer.o -g -c jsoncpp/json_writer.cpp

json.a: objects/json_reader.o objects/json_value.o objects/json_writer.o
	ar r json.a objects/json_reader.o objects/json_value.o objects/json_writer.o

######

#
# Installing Geo IP.
# GeoIP-1.4.8.tar.gz - C API
# GeoLiteCity.dat    - GeoIP City binary database
# geo-inst           - this script 
#
# gunzip -c GeoIP-1.4.8.tar.gz > GeoIP-1.4.8.tar
# tar xf GeoIP-1.4.8.tar
# rm GeoIP-1.4.8.tar
# cd GeoIP-1.4.8
# ./configure
# make
# make check
# make install
# cd ..
# rm -r GeoIP-1.4.8
# 


clean:
	rm -r objects/*.o

