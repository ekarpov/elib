/*
    File manipulation functions
*/

#include <windows.h>

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "efile.h"

/*----------------------------------------------------------------------*/
/* open and close */
/*----------------------------------------------------------------------*/
int efile_open_impl(EFILE* efile, const void* file_name, int wide_char_version, unsigned long mode)
{
    HANDLE hfile = 0;
    DWORD access_flags = 0;
    DWORD share_mode = FILE_SHARE_READ;
    DWORD creation_flags = 0;

    EASSERT(efile);
    EASSERT(file_name);
    if(efile == 0 || file_name == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy flags */
    if(mode & EFILE_OPEN_READ) access_flags |= GENERIC_READ;
    if(mode & EFILE_OPEN_WRITE) access_flags |= GENERIC_WRITE;

    if( !(mode & EFILE_OPEN_WRITE) ) access_flags |= FILE_SHARE_WRITE;

    if(mode & EFILE_OPEN_ALWAYS) creation_flags |= OPEN_ALWAYS;
    if(mode & EFILE_OPEN_EXISTING) creation_flags |= OPEN_EXISTING;
    if(mode & EFILE_OPEN_CREATENEW) creation_flags |= CREATE_ALWAYS;

    /* open file */
    if(wide_char_version)
    {
        hfile = CreateFileW((const ewchar_t*)file_name, access_flags, share_mode, 0, 
            creation_flags, FILE_ATTRIBUTE_NORMAL, 0);
    } else 
    {
        hfile = CreateFileA((const char*)file_name, access_flags, share_mode, 0, 
            creation_flags, FILE_ATTRIBUTE_NORMAL, 0);
    }

    /* check for errors */
    if(hfile == INVALID_HANDLE_VALUE)
    {
        /* get error code */
        int err = win32_to_elibc_error(GetLastError());

        /* log error if needed */
        if(err != ELIBC_ERROR_FILE_NOT_FOUND)
        {
            ETRACE_WERR_LAST("efile_open: failed to open file");
        }

        return err;
    }

    /* copy handle */
    *efile = hfile;

    return ELIBC_SUCCESS;
}

int efile_open(EFILE* efile, const char* file_name, euint32_t mode)
{
    return efile_open_impl(efile, file_name, ELIBC_FALSE, mode);
}

int efile_openw(EFILE* efile, const ewchar_t* file_name, euint32_t mode)
{
    return efile_open_impl(efile, file_name, ELIBC_TRUE, mode);
}

int efile_close(EFILE efile)
{
    /* close handle if any */
    if(efile)
    {
        if(!CloseHandle((HANDLE)efile))
        {
            /* trace last error */
            ETRACE_WERR_LAST("efile_close: failed to close file");
            return win32_to_elibc_error(GetLastError());
        }
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* position */
/*----------------------------------------------------------------------*/
int efile_seek(EFILE efile, efilesize_t position)
{
    LARGE_INTEGER li;

    EASSERT(efile);
    if(efile == 0) return ELIBC_ERROR_ARGUMENT;

    /* set new postion */
    li.QuadPart = position;

    /* seek */
    if(SetFilePointer((HANDLE)efile, li.LowPart, &li.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        /* trace last error */
        ETRACE_WERR_LAST("efile_seek: failed to seek file");
        return win32_to_elibc_error(GetLastError());
    }

    return ELIBC_SUCCESS;
}

int efile_get_pos(EFILE efile, efilesize_t* position)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
/* read and write */
/*----------------------------------------------------------------------*/
int efile_read(EFILE efile, void* buffer, size_t buffer_size, size_t* data_read)
{
    DWORD dwBytesRead = 0;

    EASSERT(efile);
    EASSERT(buffer);
    if(efile == 0 || buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* read buffer */
    if(!ReadFile((HANDLE)efile, buffer, (DWORD)buffer_size, &dwBytesRead, 0))
    {
        /* trace last error */
        ETRACE_WERR_LAST("efile_read: failed to read file");
        return win32_to_elibc_error(GetLastError());
    }

    /* copy bytes read if needed */
    if(data_read) *data_read = dwBytesRead;

    return ELIBC_SUCCESS;
}

int efile_write(EFILE efile, const void* buffer, size_t buffer_size, size_t* data_written)
{
    DWORD dwBytesWritten = 0;

    EASSERT(efile);
    EASSERT(buffer);
    if(efile == 0 || buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* write buffer */
    if(!WriteFile((HANDLE)efile, buffer, (DWORD)buffer_size, &dwBytesWritten, 0))
    {
        /* trace last error */
        ETRACE_WERR_LAST("efile_write: failed to write file");
        return win32_to_elibc_error(GetLastError());
    }

    /* copy bytes written if needed */
    if(data_written) *data_written = dwBytesWritten;

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* file size */
/*----------------------------------------------------------------------*/
int efile_size(EFILE efile, efilesize_t* file_size)
{
    LARGE_INTEGER fileSize;

    EASSERT(efile);
    EASSERT(file_size);
    if(efile == 0 || file_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* get file size */
    if(!GetFileSizeEx(efile, &fileSize))
    {
        ETRACE_WERR_LAST("efile_size: failed to read file size");
        return ELIBC_ERROR_FILE_NOT_FOUND;
    }

    /* copy size */
    *file_size = fileSize.QuadPart;

    return ELIBC_SUCCESS;
}

int efile_size_impl(const void* file_name, efilesize_t* file_size, int wide_char_version)
{
    EFILE efile;
    int err;
    
    /* open file */
    if(wide_char_version)
    {
        err = efile_openw(&efile, (const ewchar_t*)file_name, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
    } else
    {
        err = efile_open(&efile, (const char*)file_name, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
    }

    if(err != ELIBC_SUCCESS) return err;

    /* get size */
    err = efile_size(efile, file_size);

    /* close file */
    efile_close(efile);

    return err;
}

int efile_size_name(const char* file_name, efilesize_t* file_size)
{
    return efile_size_impl(file_name, file_size, ELIBC_FALSE);
}

int efile_size_namew(const ewchar_t* file_name, efilesize_t* file_size)
{
    return efile_size_impl(file_name, file_size, ELIBC_TRUE);
}

/*----------------------------------------------------------------------*/
/* test if file exists */
/*----------------------------------------------------------------------*/
int efile_exists_impl(const void* file_name, int wide_char_version)
{
    DWORD attr;
     
    EASSERT(file_name);
    if(file_name == 0) return ELIBC_FALSE;

    /* get file attributes */
    if(wide_char_version)
    {
        attr = GetFileAttributesW((const ewchar_t*)file_name);

    } else
    {
        attr = GetFileAttributesA((const char*)file_name);
    }

    /* check attributes */
    return (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) ? ELIBC_FALSE : ELIBC_TRUE;
}

int efile_exists(const char* file_name)
{
    return efile_exists_impl(file_name, ELIBC_FALSE);
}

int efile_existsw(const ewchar_t* file_name)
{
    return efile_exists_impl(file_name, ELIBC_TRUE);
}

/*----------------------------------------------------------------------*/
/* delete file */
/*----------------------------------------------------------------------*/
int efile_delete_impl(const void* file_name, int wide_char_version)
{
    BOOL ret;

    EASSERT(file_name);
    if(file_name == 0) return ELIBC_ERROR_ARGUMENT;

    /* delete file */
    if(wide_char_version)
    {
        ret = DeleteFileW((const ewchar_t*)file_name);
    } else 
    {
        ret = DeleteFileA((const char*)file_name);
    }

    /* check for errors */
    if(!ret)
    {
        /* get error code */
        int err = win32_to_elibc_error(GetLastError());

        /* log error if needed */
        if(err != ELIBC_ERROR_FILE_NOT_FOUND)
        {
            ETRACE_WERR_LAST("efile_open: failed to delete file");
        }

        return err;
    }

    return ELIBC_SUCCESS;
}

int efile_delete(const char* file_name)
{
    return efile_delete_impl(file_name, ELIBC_FALSE);
}

int efile_deletew(const ewchar_t* file_name)
{
    return efile_delete_impl(file_name, ELIBC_TRUE);
}

/*----------------------------------------------------------------------*/
/* rename file */
/*----------------------------------------------------------------------*/
int efile_rename_impl(const void* file_name, const void* new_name, int wide_char_version, ebool_t replace)
{
    BOOL ret;
    DWORD flags = 0;

    EASSERT(file_name);
    EASSERT(new_name);
    if(file_name == 0 || new_name == 0) return ELIBC_ERROR_ARGUMENT;

    /* replace if required */
    if(replace)
    {
        flags |= MOVEFILE_REPLACE_EXISTING;
    }

    /* rename file */
    if(wide_char_version)
    {
        ret = MoveFileExW((const ewchar_t*)file_name, (const ewchar_t*)new_name, flags);
    } else 
    {
        ret = MoveFileExA((const char*)file_name, (const char*)new_name, flags);
    }

    /* check for errors */
    if(!ret)
    {
        /* get error code */
        int err = win32_to_elibc_error(GetLastError());

        /* log error if needed */
        if(err != ELIBC_ERROR_FILE_NOT_FOUND)
        {
            ETRACE_WERR_LAST("efile_open: failed to rename file");
        }

        return err;
    }

    return ELIBC_SUCCESS;
}

int efile_rename(const char* file_name, const char* new_name, ebool_t replace)
{
    return efile_rename_impl(file_name, new_name, ELIBC_FALSE, replace);
}

int efile_renamew(const ewchar_t* file_name, const ewchar_t* new_name, ebool_t replace)
{
    return efile_rename_impl(file_name, new_name, ELIBC_TRUE, replace);
}

/*----------------------------------------------------------------------*/
