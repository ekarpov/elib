/*
    File manipulation functions
*/

#ifndef _EFILE_H_
#define _EFILE_H_

/*----------------------------------------------------------------------*/

/* file open modes */
#define EFILE_OPEN_READ             0x0001
#define EFILE_OPEN_WRITE            0x0002
#define EFILE_OPEN_READWRITE        (EFILE_OPEN_READ | EFILE_OPEN_WRITE)

#define EFILE_OPEN_ALWAYS           0x0010
#define EFILE_OPEN_EXISTING         0x0020
#define EFILE_OPEN_CREATENEW        0x0040

/* file attributes */
#define EFILE_ATTR_NORMAL           0x0001
#define EFILE_ATTR_SYSTEM           0x0002
#define EFILE_ATTR_HIDDEN           0x0003

/*----------------------------------------------------------------------*/

/* open and close */
int efile_open(EFILE* efile, const char* file_name, euint32_t mode);
int efile_openw(EFILE* efile, const ewchar_t* file_name, euint32_t mode);
int efile_close(EFILE efile);

/* position */
int efile_seek(EFILE efile, efilesize_t position);
int efile_get_pos(EFILE efile, efilesize_t* position);

/* read and write */
int efile_read(EFILE efile, void* buffer, size_t buffer_size, size_t* data_read);
int efile_write(EFILE efile, const void* buffer, size_t buffer_size, size_t* data_written);

/* file size */
int efile_size(EFILE efile, efilesize_t* file_size);
int efile_size_name(const char* file_name, efilesize_t* file_size);
int efile_size_namew(const ewchar_t* file_name, efilesize_t* file_size);

/* file attributes */
/* TODO: */

/* test if file exists */
int efile_exists(const char* file_name);
int efile_existsw(const ewchar_t* file_name);

/* delete file */
int efile_delete(const char* file_name);
int efile_deletew(const ewchar_t* file_name);

/* rename file */
int efile_rename(const char* file_name, const char* new_name, ebool_t replace);
int efile_renamew(const ewchar_t* file_name, const ewchar_t* new_name, ebool_t replace);

/*----------------------------------------------------------------------*/

#endif /* _EFILE_H_ */

