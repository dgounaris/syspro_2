#!/bin/bash

echo "$(cat)" >> templog.txt

echo "Total connected clients:" $(cat templog.txt | grep "LOG: CONNECTED ID" | awk '{ SUM += 1 } END { print SUM }')

echo "Max client id:" $(cat templog.txt | grep "LOG: CONNECTED ID" | sort -nrk4,4 | head -1 | awk '{ print $4 }')

echo "Min client id:" $(cat templog.txt | grep "LOG: CONNECTED ID" | sort -nk4,4 | head -1 | awk '{ print $4 }')

echo "Total exited clients:" $(cat templog.txt | grep "LOG: EXITED ID" | awk '{ SUM += 1 } END { print SUM }')

echo "Total read files:" $(cat templog.txt | grep "LOG: READ" | awk '{ SUM += 1 } END { print SUM }')

echo "Total read bytes:" $(cat templog.txt | grep "LOG: READ" | awk '{ SUM += $4 } END { print SUM }')

echo "Total written files:" $(cat templog.txt | grep "LOG: WRITE" | awk '{ SUM += 1 } END { print SUM }')

echo "Total written bytes:" $(cat templog.txt | grep "LOG: WRITE" | awk '{ SUM += $4 } END { print SUM }')

rm -rf templog.txt
