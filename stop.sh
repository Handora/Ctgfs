#!/bin/bash

ps | grep lock_server | awk '{print $1}' | xargs kill -9
ps | grep extent_server | awk '{print $1}' | xargs kill -9
echo "stop ok"
