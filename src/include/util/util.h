/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>


namespace ctgfs {
namespace util {

int RemoveDirectoryRecursively(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];

    if (path == NULL) {
        return 0;
    }
    dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opendir()");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            if (entry->d_type == DT_DIR) {
                RemoveDirectoryRecursively(path);
            }

            /*
             * Here, the actual deletion must be done.  Beacuse this is
             * quite a dangerous thing to do, and this program is not very
             * well tested, we are just printing as if we are deleting.
             */
            /*
             * When you are finished testing this and feel you are ready to do the real
             * deleting, use this: remove*STUB*(path);
             * (see "man 3 remove")
             * Please note that I DONT TAKE RESPONSIBILITY for data you delete with this!
             */
        }

    }
    closedir(dir);

    /*
     * Now the directory is emtpy, finally delete the directory itself. (Just
     * printing here, see above) 
     */
    return 1;
}

}  // namespace util
}  // namespace ctgfs
