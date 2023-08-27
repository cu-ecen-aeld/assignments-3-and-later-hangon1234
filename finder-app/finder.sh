#!/bin/bash

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

lines=$( grep -rc $searchstr $filesdir )
file_count=0
matched_line_count=0

while IFS= read -r line; do
    file_count=$(( file_count + 1 ))
    count=$( echo $line | rev | cut -d ":" -f 1 )
    matched_line_count=$(( matched_line_count + count ))
done <<< "$lines"

echo "The number of files are $file_count and the number of matching lines are $matched_line_count"
