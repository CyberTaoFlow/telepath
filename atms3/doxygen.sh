#!/bin/bash

echo
echo "Run this script on 192.168.1.203 to rebuild doxygen configuration."
echo "/var/www/atmsdocs/ directory will be used to place generated docs."

echo
doxygen doxygen.conf > /dev/null 2>&1

echo "One you browser at http://192.168.1.203/atmsdocs/html/"
echo
