#! /bin/bash

function listFiles()
{
  for file in `ls $1`;
  do
  	if [ -d "$1/$file" ]
    then
  		#echo "$2$file"
  		listFiles "$1/$file"
  	else 
      sufix="${file##*.}"
      if [ "$sufix" == "h" -o "$sufix" == "cc" -o "$sufix" == "cpp" ]; then
        echo "$1/$file"
        sudo clang-format -i "$1/$file" -style=Google
        sudo chmod a+rw "$1/$file"
      fi
  		#echo "$2$file"
  	fi
  done
}
listFiles "$1"
