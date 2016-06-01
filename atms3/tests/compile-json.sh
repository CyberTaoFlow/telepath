#COMP COMP="-g -std=gnu++0x -c -Wall"
COMP="-g -c -Wall"
cd ..
echo "building json"
g++ -o json/json_value.o  $COMP -Isrc json/json_value.cpp
g++ -o json/json_reader.o $COMP -Isrc json/json_reader.cpp
g++ -o json/json_writer.o $COMP -Isrc json/json_writer.cpp
cd tests

echo "final"
g++ -g -Wall -I src/ -I/usr/include/mysql/ -L/usr/include/ -o"json"  ./json.cpp ../json/json_value.o ../json/json_reader.o ../json/json_writer.o
