/*
    File manipulation functions
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "efile.h"

// TODO: http://stackoverflow.com/questions/13322299/file-read-using-posix-apis

/*----------------------------------------------------------------------*/
/* open and close */
/*----------------------------------------------------------------------*/
int efile_open(EFILE* efile, const char* file_name, euint32_t mode)
{
    int hfile = -1;
    int open_flags = 0;
    mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    EASSERT(efile);
    EASSERT(file_name);

    /* init flags */
    if(mode & EFILE_OPEN_READWRITE)
        open_flags = O_RDWR;
    else if(mode & EFILE_OPEN_WRITE)
        open_flags = O_WRONLY;
    else
        open_flags = O_RDONLY;

    if(mode & EFILE_OPEN_ALWAYS) open_flags |= O_CREAT;
    if(mode & EFILE_OPEN_CREATENEW) open_flags |= (O_CREAT | O_EXCL);

    /* open */
    hfile = open(file_name, open_flags, open_mode);

    /* check for errors */
    if(hfile == -1)
    {
        /* check error type */
        if(errno == ENOENT)
        {
            /* report last error */
            ETRACE("efile_open failed to open file, file not found");
            return ELIBC_ERROR_FILE_NOT_FOUND;

        } else
        {
            ETRACE_ERRNO("efile_open failed to open file");
            return errno_to_elibc_error(errno);
        }
    }

    /* copy handle */
    *efile = hfile;

    return ELIBC_SUCCESS;
}

int efile_openw(EFILE* efile, const ewchar_t* file_name, euint32_t mode)
{
    EUNUSED(efile);
    EUNUSED(file_name);
    EUNUSED(mode);

    /* NOTE: conversion required, not a part of elibc */
    EASSERT1(0, "efile_openw not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int efile_close(EFILE efile)
{
    /* close handle if any */
    if(efile)
    {
        if(close(efile) == -1)
        {
            /* trace last error */
            ETRACE_ERRNO("efile_close failed to close file");
            return errno_to_elibc_error(errno);
        }
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* position */
/*----------------------------------------------------------------------*/
int efile_seek(EFILE efile, efilesize_t position)
{
    EUNUSED(efile);
    EUNUSED(position);

    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int efile_get_pos(EFILE efile, efilesize_t* position)
{
    EUNUSED(efile);
    EUNUSED(position);

    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
/* read and write */
/*----------------------------------------------------------------------*/
int efile_read(EFILE efile, void* buffer, size_t buffer_size, size_t* data_read)
{
    ssize_t bytes_read = 0;

    EASSERT(buffer);
    if(buffer == 0 || buffer_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* read buffer */
    bytes_read = read(efile, buffer, buffer_size);
    if(bytes_read == -1)
    {
        /* trace last error */
        ETRACE_ERRNO("efile_read failed to read file");
        return errno_to_elibc_error(errno);
    }

    /* copy bytes read if needed */
    if(data_read) *data_read = bytes_read;

    return ELIBC_SUCCESS;
}

int efile_write(EFILE efile, const void* buffer, size_t buffer_size, size_t* data_written)
{
    ssize_t bytes_written = 0;

    EASSERT(buffer);
    if(buffer == 0 || buffer_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* write buffer */
    bytes_written = write(efile, buffer, buffer_size);
    if(bytes_written == -1)
    {
        /* trace last error */
        ETRACE_ERRNO("efile_read failed to write file");
        return errno_to_elibc_error(errno);
    }

    /* copy bytes written if needed */
    if(data_written) *data_written = bytes_written;

    return ELIBC_SUCCESS;
}

/* file size */
int efile_size(EFILE efile, efilesize_t* file_size)
{
    struct stat st;

    EASSERT(file_size);
    if(efile == -1 || file_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* get size */
    if (fstat(efile, &st) != 0)
    {
        /* trace last error */
        ETRACE_ERRNO("efile_size failed to get file size");
        return errno_to_elibc_error(errno);
    }

    /* copy file size */
    *file_size = st.st_size;

    return ELIBC_SUCCESS;
}

int efile_size_name(const char* file_name, efilesize_t* file_size)
{
    struct stat st;

    EASSERT(file_name);
    EASSERT(file_size);
    if(file_name == 0 || file_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* get size */
    if (stat(file_name, &st) != 0)
    {
        /* trace last error */
        ETRACE_ERRNO("efile_size_name failed to get file size");
        return errno_to_elibc_error(errno);
    }

    /* copy file size */
    *file_size = st.st_size;

    return ELIBC_SUCCESS;
}

int efile_size_namew(const ewchar_t* file_name, efilesize_t* file_size)
{
    EUNUSED(file_name);
    EUNUSED(file_size);

    /* NOTE: conversion required, not a part of elibc */
    EASSERT1(0, "efile_size_namew not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
/* test if file exists */
/*----------------------------------------------------------------------*/
int efile_exists(const char* file_name)
{
    EASSERT(file_name);
    if(file_name == 0) return ELIBC_FALSE;

    EASSERT1(0, "efile_exists not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int efile_existsw(const ewchar_t* file_name)
{
    EASSERT(file_name);
    if(file_name == 0) return ELIBC_FALSE;

    EASSERT1(0, "efile_existsw not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
/* delete file */
/*----------------------------------------------------------------------*/
int efile_delete(const char* file_name)
{
    EASSERT(file_name);
    if(file_name == 0) return ELIBC_ERROR_ARGUMENT;

    EASSERT1(0, "efile_delete not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int efile_deletew(const ewchar_t* file_name)
{
    EASSERT(file_name);
    if(file_name == 0) return ELIBC_ERROR_ARGUMENT;

    EASSERT1(0, "efile_deletew not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
/* rename file */
/*----------------------------------------------------------------------*/
int efile_rename(const char* file_name, const char* new_name, ebool_t replace)
{
    EASSERT(file_name);
    EASSERT(new_name);
    if(file_name == 0 || new_name == 0) return ELIBC_ERROR_ARGUMENT;

    EASSERT1(0, "efile_rename not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int efile_renamew(const ewchar_t* file_name, const ewchar_t* new_name, ebool_t replace)
{
    EASSERT(file_name);
    EASSERT(new_name);
    if(file_name == 0 || new_name == 0) return ELIBC_ERROR_ARGUMENT;

    EASSERT1(0, "efile_renamew not implemented on Unix systems");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
