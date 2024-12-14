#!/bin/sh

if [ $# -lt 2 ]
then
        echo "Not enough arguments specified!"
        exit 1
else
	if ! [ -d "$1" ]
	then
		echo "the directory $1 is not part of the filesystem!"
		exit 1
	fi
fi

NUMFILES=$(find "$1" -type f | wc -l)
NUMSTR=$(find "$1" -type f | xargs grep "$2" | wc -l)

echo "The number of files are ${NUMFILES} and the number of matching lines are ${NUMSTR}"

exit 0
