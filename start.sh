#!/bin/bash

echo "opening lock server on port 55555..."
build/src/master/lock_server 55555 &
echo "open ok"
echo "opening extent server on port 55556..."
build/src/fs/extent_server 55556 &
echo "open ok"
