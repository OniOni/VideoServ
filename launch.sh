#!/bin/bash

while getopts "vp:" name
do
    case $name in
	p)
	    echo "Starting server on " $OPTARG "..."
	    cp catalogue.default catalogue.txt
	    sed -i 's/serv_addr/'$OPTARG'/g' catalogue.txt
	    echo "Server running..."
	    ./serv.a 
	    echo "See you !!"
	    ;;
	v)
	    echo "Usage : "$0" -p address"
    esac
done
    