#!/bin/sh

if [ "$1" = "-h" -o "$1" = "--help" ]; then
	echo "Usage: processes=processnum threads=threads_of_each_process" \
		 "count=count_of_each_series trngkit.sh"
	exit 0
fi

if test -z "$processes" -a test -z "$threads"; then
	echo "TRNG kit will be run in single test mode"
	echo "For multi processes and multi threads test, please"
	echo "run \"processes= threads= trngkit.sh\""
else
	if test ! -z "$processes"; then
		echo "TRNG kit will be run with $processes processes"
	fi

	if test ! -z "$threads"; then
		echo "Each TRNG kit will be run with $threads threads"
	fi
fi

if test -z "$count"; then
	echo "Series random test will be run with only 1 series"
	echo "For more than 1 series random test, please run"
	echo "\" count= trngkit.sh \""
fi

if test -z "$processes"; then
	processnum=0
else
	processnum=$processes
fi

if test -z "$count"; then
	seriescount=2
else
	seriescount=$count
fi

processid=0
while [ $processid -le $processnum ]; do
	trngkit --length=1 &
	trngkit --length=8 &
	trngkit --length=1 --multi=$processnum &
	trngkit --length=8 --multi=$processnum &
	trngkit --series --count=$seriescount &
	trngkit --series --count=$seriescount --multi=$processnum &
	processid=$(($processid + 1))
done

wait

exit 0
