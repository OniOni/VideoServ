#!/bin/bash

if [ $1 = "-p" ]; then
    cp catalogue.default catalogue.txt
    sed -i 's/serv_addr/'$2'/g' catalogue.txt
    ./serv.a
else
    echo $1 is not a recognized command
fi
    