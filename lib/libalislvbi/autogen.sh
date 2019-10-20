#!/bin/bash

if [ ! -d "m4" ]; then
	libtoolize --automake --copy --debug --force
fi

autoreconf --install
