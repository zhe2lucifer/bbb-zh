#!/bin/sh

# $1 $(prefix)
# $2 $(srcdir)
# $3 $(VERSION)

srcdir=`pwd $2`
echo "srcdir is $srcdir"

packagedir=`dirname $srcdir`
packagedir=`dirname $packagedir`
package=`basename $packagedir` 

mkdir -p $1/usr/share/doc > /dev/null 2>&1

export PACKAGE="$package"
echo "PACKAGE is $PACKAGE"

export VERSION="$3"
echo "VERSION is $VERSION"

export INPUT_SRC="$srcdir/../../include $srcdir/../../README $srcdir/../../src"
echo "INPUT_SRC is $INPUT_SRC"

export OUTPUT_DIR="$1/usr/share/doc"
echo "OUTPUT_DIR is $OUTPUT_DIR"

export OUTPUT_SUBMOD_DIR="$package"
echo "OUTPUT_SUBMOD_DIR is $OUTPUT_SUBMOD_DIR"

export EXAMPLES="$srcdir/../../examples"
echo "EXAMPLES is $EXAMPLES"
doxygen $srcdir/doxygen.cfg
