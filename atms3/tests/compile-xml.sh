
#COMP COMP="-g -std=gnu++0x -c -Wall"
COMP="-g -c -Wall"
echo "building mxml"
cd ..
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
cd tests

echo "final"
g++ -g -Wall -I src/ -I/usr/include/mysql/ -L/usr/include/ -o"xml"  ./xml.cpp ../mxml/mxml-attr.o ../mxml/mxml-entity.o ../mxml/mxml-file.o ../mxml/mxml-get.o ../mxml/mxml-index.o ../mxml/mxml-node.o ../mxml/mxml-search.o ../mxml/mxml-set.o ../mxml/mxml-private.o ../mxml/mxml-string.o -lrt 

#chmod +s ./atms
