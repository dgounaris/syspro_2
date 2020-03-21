#!/bin/bash

myrandom() {
	echo $(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $1 | head -n 1)
}

if [ "$#" -ne 4 ];
	then echo Wrong number of parameters
	return
fi
dirname=$1
filesnum=$2
dirsnum=$3
levels=$4
if [ -e $1 ]
	then echo $1 exists
else
	echo $1 will be created
	mkdir "$1"
fi
curpath="$1/"
tfreal=$(($filesnum / ($dirsnum + 1)))
tfmod=$(($filesnum % ($dirsnum + 1)))
if [ $tfreal -ge 0 ]
	then echo $(myrandom $(( (RANDOM % 128000) + 1)) ) >> $curpath$(myrandom $(( (RANDOM % 8) + 1 )) )
fi
curfile=1
while [ $curfile -le $tfreal ]; do
	if [ $tfmod -eq 0 ]
		then break
	fi
	echo $(myrandom 2048) >> $curpath$(myrandom $(( (RANDOM % 8) + 1)) )
	curfile=$((curfile+1))
done
curdir=1
curlvl=1
while [ $curdir -le $dirsnum ]; do
	curfile=1
	newpath=$curpath$(myrandom 6)
	echo $newpath
	mkdir $newpath
	tfreal=$((($filesnum - $curdir) / ($dirsnum + 1)))
	tfmod=$((($filesnum - $curdir) % ($dirsnum + 1)))
	if [ $tfreal -ge 0 ]
		then echo $(myrandom $(( (RANDOM % 128000) + 1)) ) >> $newpath"/"$(myrandom $(( (RANDOM % 8) + 1)) )
	fi
	while [ $curfile -le $tfreal ]; do
		if [ $tfmod -eq 0 ]
			then break
		fi
		echo $(myrandom $(( (RANDOM % 128000) + 1)) ) >> $newpath"/"$(myrandom $(( (RANDOM % 8) + 1)) )
		curfile=$((curfile+1))
	done
	curdir=$((curdir+1))
	if [ $curlvl -lt $4 ]
		then curpath=$newpath"/"
		curlvl=$((curlvl+1))
	else
		curpath="$1/"
		curlvl=1
	fi
done
