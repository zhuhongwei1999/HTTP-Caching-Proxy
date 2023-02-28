#!/bin/bash
make clean
make
echo 'start running http proxy ...'
./main &
while true ; do continue ; done