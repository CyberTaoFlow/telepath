#!/bin/sh

valgrind --track-fds=yes --log-file=/tmp/atms-mem-log --num-callers=50 --leak-check=full --leak-resolution=high --show-reachable=yes ./snif

