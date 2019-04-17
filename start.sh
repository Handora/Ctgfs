#!/bin/bash

echo "opening lock server on port 55555..."
build/src/master/lock_server 55555 > ctgfs_lock_server.log 2>&1 &
echo "open ok"
echo "opening extent server on port 55556..."
build/src/fs/extent_server 55556 > ctgfs_server.log 2>&1 &
echo "open ok"
echo "mount to ctgfs dir"
mkdir -p ctgfs
build/src/client/yfs_client ctgfs 55556 55555 > ctgfs_client.log 2>&1 &
