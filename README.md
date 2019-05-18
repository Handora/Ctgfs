# Ctgfs

## build
```
git submodule update --init --recursive
./build.sh
```
## format
```
./format.sh <path>
// example: format.sh src
```

## test yfs client
```
1. sudo umount -l yfs1 yfs2 ctgfs
2. ./test/start.sh
3. run the tests under ./test/
4. ./test/stop.sh
```
