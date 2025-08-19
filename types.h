#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

/**
 * Struct user_desc adapted to fit 32 bit arch.
 * 
 * 4 bytes long;
 */
struct user_desc32{
    uint32_t entry_number;
    uint32_t base_addr;
    uint32_t limit;
    uint32_t seg_32bit:1;
    uint32_t contents:2;
    uint32_t read_exec_only:1;
    uint32_t limit_in_pages:1;
    uint32_t seg_not_present:1;
    uint32_t useable:1;
};

/**
 * Struct iovec adapted to fit 32 bit arch.
 * 
 * 8 bytes long;
 */
struct iovec32{
    uint32_t iov_base;     /* Pointer to data.  */
    uint32_t iov_len;     /* Length of data.  */
};

/**
 * Struct iovec adapted to fit 64 bit arch.
 * 
 * 16 bytes long;
 */
struct iovec64{
    void *iov_base;     /* Pointer to data.  8 bytes */
    size_t iov_len;     /* Length of data.   8 bytes */
};

#endif
