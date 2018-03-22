#!/bin/bash

export CFLAGS="${CFLAGS} -I${PREFIX}/include"
export LDFLAGS="${LDFLAGS} -I${PREFIX}/lib"

$PYTHON setup.py install

