/*
Title: sfind.c
Author: Mane Galstyan
Created on: April 18, 2024
Description: print list of pathnames based on test
Usage: sfind [dir1 dir2 ..] [test]
Build with: gcc -o sfind sfind.c
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <dirent.h>
#include <sys/types.h>   /* type definitions used by many programs    */
#include <sys/stat.h>   /* type definitions used by many programs    */
#include <stdlib.h>      /* prototypes of many C functions and macros */
#include <stdio.h>       /* C standard I/O library                    */
#include <string.h>      /* string functions                          */
#include <limits.h>      /* system limit constants                    */
#include <unistd.h>      /* prototypes of most system calls           */
#include <time.h>        /* time related functions                    */
#ifndef LINUX_FCNTL
    #include <fcntl.h>       /* file I/O related functions            */
#endif
#include <errno.h>       /* errno and error constants and functions   */
#include <paths.h>       /* For definition of _PATH_LASTLOG           */
#include <locale.h>      /* For localization                          */
#include <utmpx.h>       /* For utmp and wtmp processing              */
#include <signal.h>      /* For all signal-related functions          */
#include <ftw.h>
#include <fnmatch.h>
#include <stdint.h>

#define  STRING_MAX  1024
#define MAXLEN  STRING_MAX        /* Maximum size of message string         */
#define  MAXDEPTH  100

/* globally declared so that the function for nftw can use them*/
char *target = NULL; /* the option argument*/
struct stat target_stat; /* the option stat structure*/

/* error_message()
   This prints an error message associated with errnum  on standard error
   if errnum > 0. If errnum <= 0, it prints the msg passed to it.
   It does not terminate the calling program.
   This is used when there is a way  to recover from the error.               */
void error_mssge(int errornum, const char * msg) {
    errornum > 0 ? fprintf(stderr, "%s\n", msg) : fprintf(stderr, "%s\n", msg);
}

/* fatal_error()
   This prints an error message associated with errnum  on standard error
   before terminating   the calling program, if errnum > 0.
   If errnum <= 0, it prints the msg passed to it.
   fatal_error() should be called for a nonrecoverable error.                 */
void fatal_error(int errornum, const char * msg) {
    error_mssge(errornum, msg);
    exit(EXIT_FAILURE);
}

/* usage_error()
   This prints a usage error message on standard output, advising the
   user of the correct way to call the program, and terminates the program.   */
void usage_error(const char * msg) {
    fprintf(stderr, "usage: %s\n", msg);
    exit(EXIT_FAILURE);
}

/* function to search for target name in file name, used in nftw*/
/* uses fnmatch to test if the target matches the basename of the file or symbolic link is the same*/
/* print if fnmatch returns 0*/
int file_match(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    if (tflag == FTW_F || tflag == FTW_SL) {
        const char *basename = fpath + ftwbuf->base;
        if (fnmatch(target, basename, 0) == 0) {
            printf("%s\n", fpath);
        }
    }
    return 0;
}

/* function to search for the files with links to target, used in nftw*/
/* uses stat structures to compare the inode and dev number of the target and fpath*/
/* print if inode and dev match*/
int link_match(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    if(tflag == FTW_SL) {
        const char *basename = fpath + ftwbuf->base;
        struct stat entry;
        if(-1 == stat(fpath,&entry))
            fatal_error(errno, "Failed to stat file");
        if((entry.st_ino == target_stat.st_ino) && (entry.st_dev == target_stat.st_dev)) {
            printf("%s\n", fpath);
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    char usage_msg[512];
    int ms;
    char options[] = "m:s:";
    int m_option = 0;
    int s_option = 0;
    char *m_arg;
    int m_arg_length;
    char *s_arg;
    int s_arg_length;
    
    char *directories[STRING_MAX];
    int dir_count = 0;
    errno = 0;
    
    /* get cwd*/
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fatal_error(errno, "getcwd() error");
    }
    
    /*parses command line for list of directories*/
    int i = 1;
    while (i < argc && argv[i][0] != '-') {
        if (strcmp(argv[i], ".") == 0) {
            directories[dir_count] = strdup(cwd);
        } else {
            directories[dir_count] = argv[i];
        }
        dir_count++;
        i++;
    }
    
    /* default directory is non are specified*/
    if (dir_count == 0) {
        directories[dir_count++] = strdup(cwd);
    }
    
    /* parse options looking for s or m with required arg*/
    while((ms = getopt(argc, argv, "s:m:")) != -1) {
        switch(ms) {
            case 's':
                s_option = 1;
                s_arg_length = strlen(optarg);
                s_arg = malloc((s_arg_length+1) * sizeof(char));
                if (NULL == s_arg )
                    fatal_error(EXIT_FAILURE, "calloc could not allocate memory\n");
                strcpy(s_arg, optarg);
                break;
            case 'm':
                m_option = 1;
                m_arg_length = strlen(optarg);
                m_arg = malloc((m_arg_length+1) * sizeof(char));
                if (NULL == m_arg )
                    fatal_error(EXIT_FAILURE, "calloc could not allocate memory\n");
                strcpy(m_arg, optarg);
                break;
            case ':':
                sprintf(usage_msg, "%s  [dir1 dir2 ...] [test ]", argv[0]);
                usage_error(usage_msg);
            case '?':
                printf("invalid argument found %c\n",optopt);
                sprintf(usage_msg, "%s  [dir1 dir2 ...] [test ]", argv[0]);
                usage_error(usage_msg);
        }
    }

    /* if both options are provides return usage error*/
    if(m_option && s_option) {
        sprintf(usage_msg, "%s [dir1 dir2 ...] [test ]", argv[0]);
        usage_error(usage_msg);
    }
    
    /* if no options are provides return usage error*/
    if (!s_option && !m_option) {
           usage_error("Must specify either -s or -m option.");
   }
    
    /* set target based on the option selected and stat target if option s selected*/
    if(s_option) {
        target = s_arg;
        if(-1 == stat(target,&target_stat))
            fatal_error(errno, "Failed to stat file");
    }
    else target = m_arg;
    
    /* call tree walk for option m for s option with respective functions for each option*/
    for(int j = 0; j < dir_count; j++) {
        if(m_option) {
            if(-1 == nftw(directories[j], file_match, MAXDEPTH, FTW_PHYS))
                fatal_error(errno, "nftw error");
        }
        if(s_option) {
            if(-1 == nftw(directories[j], link_match, MAXDEPTH, FTW_PHYS |FTW_MOUNT))
                fatal_error(errno, "nftw error");
        }
    }

    free(s_arg);
    free(m_arg);
    return 0;
}
