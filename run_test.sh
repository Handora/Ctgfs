#!/bin/bash

RED="\033[1;31m"
GREEN="\033[1;32m"
NOCOLOR="\033[0m"

check_or_exit()
{
  ret="$?"
  if [ $ret -ne 0 ]; then
    echo -e "${RED}$1" "fail" "${NOCOLOR}"
    test/stop.sh
    echo -e "${RED}stop all" "${NOCOLOR}"
    exit -1
  else
    echo -e "${GREEN}$1" "success" "${NOCOLOR}"
  fi
}

init()
{
  rm -rf test_log
  mkdir test_log
}

init
echo "start all"
test/start.sh
check_or_exit "start all"

# run .pl test
for file in test/*.pl; do
  if [[ $file = test/single-*  ]] ; then
    echo "start test " $file
    base=`basename $file`
    $file yfs1 > test_log/"$base".log 2>&1
    check_or_exit $file
  fi
  if [[ $file = test/double-*  ]] ; then
    echo "start test " $file
    base=`basename $file`
    $file yfs1 yfs2 > test_log/"$base".log 2>&1
    check_or_exit $file
  fi
done

# run .cc test
build/test/test-dir yfs1 yfs2
check_or_exit "test/test-dir"

build/test/test-dirs yfs1 yfs2
check_or_exit "test/test-dirs"

echo "stop all"
test/stop.sh
echo -e "${GREEN}stop all success${NOCOLOR}"

echo -e "${GREEN}done${NOCOLOR}"
