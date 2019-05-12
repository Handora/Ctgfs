/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ctgfs {
namespace util {

// TODO why link error
static int MyRemoveDirectoryRecursively(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d)
     {
       struct dirent *p;

       r = 0;

       while (!r && (p=readdir(d)))
         {
           int r2 = -1;
           char *buf;
           size_t len;

           /* Skip the names "." and ".." as we don't want to recurse on them. */
           if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             {
               continue;
             }

           len = path_len + strlen(p->d_name) + 2;
           buf = (char *)malloc(len);

           if (buf)
             {
               struct stat statbuf;

               snprintf(buf, len, "%s/%s", path, p->d_name);

               if (!stat(buf, &statbuf))
                 {
                   if (S_ISDIR(statbuf.st_mode))
                     {
                       r2 = MyRemoveDirectoryRecursively(buf);
                     }
                   else
                     {
                       r2 = unlink(buf);
                     }
                 }

               free(buf);
             }

           r = r2;
         }

       closedir(d);
     }

   if (!r)
     {
       r = rmdir(path);
     }

   return r;
 }
 

}  // namespace util
}  // namespace ctgfs

#include <time.h>
#include <string.h>
#include <stdio.h>

/* CTGWARN: a simple log utility. by weifeng */

/* get the timestamp */
static inline char *timenow() {
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return buffer;
}

#define ERROR_TAG       "ERROR"
#define WARN_TAG        "WARN"
#define INFO_TAG        "INFO"
#define DEBUG_TAG       "DEBUG"

/* cut the directory like 'util/' */
#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

/* the format of the log. */
#define LOG_FMT             "%s:| %-6s | %-18s | %s:%d | "

#define LOG_ARGS(LOG_TAG)   timenow(), LOG_TAG, _FILE, __FUNCTION__, __LINE__

/* add \n for each log. */
#define NEWLINE     "\n"

/* the print function */
#define PRINTFUNCTION(format, ...)      fprintf(stderr, format, __VA_ARGS__)

/* the higher the log level is, the more details you will get. */
#define ERROR_LEVEL     0x01
#define WARN_LEVEL      0x02
#define INFO_LEVEL      0x03
#define DEBUG_LEVEL     0x04

#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

/* the log is DEBUG level, 0x04. */
#if LOG_LEVEL >= DEBUG_LEVEL
#define CTG_DEBUG(message, args...)    PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TAG), ## args)
#else
#define CTG_DEBUG(message, args...)
#endif

/* the log is INFO level, 0x03. */
#if LOG_LEVEL >= INFO_LEVEL
#define CTG_INFO(message, args...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TAG), ## args)
#else
#define CTG_INFO(message, args...)
#endif

/* the log is WARN level, 0x02. */
#if LOG_LEVEL >= WARN_LEVEL
#define CTG_WARN(message, args...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(WARN_TAG), ## args)
#else
#define CTG_WARN(message, args...)
#endif

/* the log ERROR level, 0x01. */
#if LOG_LEVEL >= ERROR_LEVEL
#define CTG_ERROR(message, args...)    PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## args)
#else
#define CTG_ERROR(message, args...)
#endif