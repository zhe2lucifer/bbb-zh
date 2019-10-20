#!/bin/sh

if [ -z "$1" ]; then
    echo "usage: $0 <linux directory full path>"
    exit
fi

./autogen.sh && \
./configure \
	CC=/opt/mipsel_4_4_hardfloat/bin/mipsel-linux-gnu-gcc \
	CXX=/opt/mipsel_4_4_hardfloat/bin/mipsel-linux-gnu-g++ \
	--host=mipsel-linux \
	--prefix=$1/images/fs/rootfs/usr/local/ \
	--enable-tests --enable-debug --enable-docs --enable-examples \
	CPPFLAGS="-I"$1"/kernel/linux/include/ali_common"
