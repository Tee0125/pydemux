#!/bin/bash


os=`uname | grep -i linux || echo nolinux`
if [ $os != "nolinux" ] ; then
    export CC=gcc-4.7
    export CXX=g++-4.7
fi

$PYTHON setup.py install

