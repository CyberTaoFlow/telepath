
SVNDEV := -D'SVN_REV="$(shell svnversion -n .)"'
SVNFLAG := $(SVNDEV)
MONTH := 2678400
T1 := -D'EPOCH=$(shell date +%s)+$(MONTH)'
TIME := $(T1)
TIME2 := -D'COMPILATION_TIME=$(shell date +%s)'



PCAP=/usr/lib64/libpcap.so
CRYPTO=/usr/lib64/libcrypto.so
ZLIB=/usr/lib64/libz.so
PCRE=/usr/lib/libpcre.a 
LIBSTDC=/usr/lib/gcc/x86_64-redhat-linux/4.8.2/libstdc++.so
Redis = /usr/lib/x86_64-linux-gnu/libhiredis.a

ifeq ($(wildcard $(LIBSTDC) ),)
        LIBSTDC = -static-libstdc++
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

ifeq ($(wildcard $(PCRE) ),) 
	PCAP = /usr/lib64/libpcap.so
endif

ifeq ($(wildcard $(PCAP) ),) 
	PCAP = /usr/lib/x86_64-linux-gnu/libpcap.a
endif

ifeq ($(wildcard $(PCAP) ),) 
	PCAP = /usr/lib64/libpcap.a
endif

ifeq ($(wildcard $(CRYPTO) ),) 
	CRYPTO = /usr/lib/x86_64-linux-gnu/libcrypto.a
endif

ifeq ($(wildcard $(CRYPTO) ),)
        CRYPTO = /usr/lib64/libcrypto.so
endif

ifeq ($(wildcard $(ZLIB) ),) 
	ZLIB = /usr/lib/x86_64-linux-gnu/libz.a
endif

ifeq ($(wildcard $(ZLIB) ),) 
	ZLIB = /usr/lib64/libz.a
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


../objects/json_value.o:
	g++ -o ../objects/json_value.o -g -c ../jsoncpp/json_value.cpp

../objects/json_reader.o:
	g++ -o ../objects/json_reader.o -g -c ../jsoncpp/json_reader.cpp

../objects/json_writer.o:
	g++ -o ../objects/json_writer.o -g -c ../jsoncpp/json_writer.cpp

json.a: ../objects/json_reader.o ../objects/json_value.o ../objects/json_writer.o
	ar r json.a ../objects/json_reader.o ../objects/json_value.o ../objects/json_writer.o


telewatchdog: telewatchdog.o  json.a
	g++ -g -o telewatchdog telewatchdog.o -L./ -pthread -static-libgcc $(LIBSTDC) $(PCAP) $(CRYPTO) $(ZLIB) $(PCRE) json.a -ldl -lcurl $(Redis)

telewatchdog.o: telewatchdog.cpp 
	g++ -g  $(SVNFLAG) $(TIME) $(TIME2) -c -o telewatchdog.o -DREDIS telewatchdog.cpp



clean:
	rm -r *.o

