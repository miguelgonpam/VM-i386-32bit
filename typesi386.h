#include <stdint.h>
//#include <signal.h>
//#include <types.h>

#ifndef TYPES_H
#define TYPES_H

#define SA_RESTORER 0x04000000

#if defined(sigset_t32)

#else

typedef uint64_t sigset_t32[2];

#endif



void convert_mask32_to_mask64(const sigset_t32 src, sigset_t *dst);
void convert_flags32_to_flags64(const uint32_t f32, uint32_t * f64);

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

struct sigaction64{
    union {
        void * sa_handler;
        void * sa_sigaction;
    } __sigaction_handler;

    //void * sa_handler;
    //void * sa_sigaction;

    sigset_t sa_mask; /*128 B */
    uint32_t sa_flags; /* 4B + 4B padding */
    uint32_t padding;
    void * sa_restorer;
};

struct sigaction32{
    union{
        uint32_t sa_handler;
        uint32_t sa_sigaction;
    }__sigaction_handler;
    uint32_t sa_flags;
    uint32_t sa_restorer;
    //uint32_t sa_mask; /* 4 bytes in i386 */
    sigset_t32 sa_mask;
    uint32_t padding;
};
/*
struct sigaction{
    union
      {
        __sighandler_t sa_handler;
        void (*sa_sigaction) (int, siginfo_t *, void *);
      }
    __sigaction_handler;
    __sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer) (void);
  };
*/
#endif
