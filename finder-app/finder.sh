#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Please check parameter"
    exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d $filesdir ]; then
    echo "Directory $filesdir does not exist!"
    exit 1 
fi

file_count=0
matched_line_count=0

matched_line_count=$( grep -ir "$searchstr" $filesdir | wc -l )
file_count=$( grep -irl "$searchstr" $filesdir | wc -l )

echo "The number of files are $file_count and the number of matching lines are $matched_line_count"
