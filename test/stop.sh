#!/usr/bin/env bash

killall yfs_client
killall extent_server
killall lock_server
killall master_main
YFSDIR1=$PWD/yfs1
YFSDIR2=$PWD/yfs2

export PATH=$PATH:/usr/local/bin
UMOUNT="umount"
if [ -f "/usr/local/bin/fusermount" -o -f "/usr/bin/fusermount" -o -f "/bin/fusermount" ]; then
    UMOUNT="fusermount -u";
fi
$UMOUNT $YFSDIR1
$UMOUNT $YFSDIR2
