#!/bin/bash

echo "Running aclocal -I m4 ${ACLOCAL_FLAGS:+$ACLOCAL_FLAGS }..."

if [ ! -d "m4" -o ! -f "ltmain.sh" ]; then
	libtoolize --automake --copy --force
fi

aclocal -I m4 $ACLOCAL_FLAGS
echo "Running autoheader..."
autoheader
echo "Running automake ..."
automake --add-missing -a -c
echo "Running autoconf ..."
autoconf 

echo "You may now run:
	./configure --enable-maintainer-mode && make
	for cross-compile, you can run
	./configure --help
	to get the parameters list
 "
