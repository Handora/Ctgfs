#! /bin/bash

function listFiles()
{
  for file in `ls $1`;
  do
  	if [ -d "$1/$file" ]
    then
  		listFiles "$1/$file"
  	else
      sufix="${file##*.}"
      if [ "$sufix" == "h" -o "$sufix" == "cc" -o "$sufix" == "cpp" ]; then
        echo "$1/$file"
        clang-format -i "$1/$file" -style=Google
        chmod a+rw "$1/$file"
      fi
  	fi
  done
}

listFiles "$1"
