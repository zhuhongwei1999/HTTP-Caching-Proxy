#!/bin/bash
make clean
make
echo 'start unning http proxy ...'
./main &
while true ; do continue ; done