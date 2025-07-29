#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <unistd.h>
#include <poll.h>
#include <grp.h>
#include <sys/uio.h>
#include <sys/io.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/fsuid.h>
#include <sys/types.h>
#include <sys/swap.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/xattr.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#include "instr.h"

/*
typedef struct stat32 {
    uint32_t st_dev;
    uint16_t __pad1;
    uint32_t st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t st_rdev;
    uint16_t __pad2;
    int32_t  st_size;
    int32_t  st_blksize;
    int32_t  st_blocks;

    int32_t  st_atim;
    uint32_t st_atime_nsec;

    int32_t  st_mtim;
    uint32_t st_mtime_nsec;

    int32_t  st_ctim;
    uint32_t st_ctime_nsec;

    uint32_t __unused4;
    uint32_t __unused5;
};
*/


uint8_t * mem;


/**
 *  read - read from a file descriptor
 *
 *  Syscall number 0
 *
 *  ssize_t read(int fd, void *buf, size_t count);
 */
uint32_t do_read(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * buf = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_read, (int) *arg1, buf, (size_t) *arg3);
}

/**
 *  write - write to a file descriptor
 *
 *  Syscall number 1
 *
 *  ssize_t write(int fd, const void *buf, size_t count);
 */
uint32_t do_write(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void * buf = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_write, (int) *arg1, buf, (size_t)*arg3);
}


/**
 *  open, openat, creat - open and possibly create a file
 *
 *  Syscall number2
 *
 *  int open(const char *pathname, int flags);
 *  int open(const char *pathname, int flags, mode_t mode);
 */
uint32_t do_open(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (const char *)(mem + *arg1);  // arg1 = puntero al path (32-bit)
    int flags = (int)*arg2;                              // arg2 = flags (O_RDONLY, O_WRONLY, etc.)
    mode_t mode = (mode_t)*arg3;                         // arg3 = modo (si flags incluye O_CREAT)

    // Decide si llamar a open(2) con 2 o 3 argumentos
    if (flags & O_CREAT) {
        return (uint32_t)syscall(SYS_open, pathname, flags, mode); // open(path, flags, mode)
    } else {
        return (uint32_t)syscall(SYS_open, pathname, flags);       // open(path, flags)
    }
}

/**
 *  close - close a file descriptor
 *
 *  Syscall number 4
 *
 *  int close(int fd);
 */
uint32_t do_close(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_close, (int)*arg1);
}

/**
 *  stat, fstat, lstat, fstatat - get file status
 *
 *  Syscall number 4
 *
 *  int stat(const char *pathname, struct stat *statbuf);
 */
uint32_t do_stat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (const char *)(mem + *arg1);
    struct stat32 *statbuf32 = (struct stat32 *)(mem + *arg2);
    struct stat statbuf64;

    int result = syscall(SYS_stat, pathname, &statbuf64);
    if (result >= 0) {
        //stat64_to_stat32(&statbuf64, statbuf32); ???
    }
    return (uint32_t)result;
}

/**
 *  stat, fstat, lstat, fstatat - get file status
 *
 *  Syscall number 5
 *
 *  int fstat(int fd, struct stat *statbuf);
 */
uint32_t do_fstat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    int fd = (int)*arg1;
    struct stat32 *statbuf32 = (struct stat32 *)(mem + *arg2);
    struct stat statbuf64;

    int result = syscall(SYS_fstat, fd, &statbuf64);
    if (result >= 0) {
        //stat64_to_stat32(&statbuf64, statbuf32);
    }
    return (uint32_t)result;
}

/**
 *  stat, fstat, lstat, fstatat - get file status
 *
 *  Syscall number 6
 *
 *  int lstat(const char *pathname, struct stat *statbuf);
 */
uint32_t do_lstat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (const char *)(mem + *arg1);
    struct stat32 *statbuf32 = (struct stat32 *)(mem + *arg2);
    struct stat statbuf64;

    int result = syscall(SYS_lstat, pathname, &statbuf64);
    if (result >= 0) {
        //stat64_to_stat32(&statbuf64, statbuf32);
    }
    return (uint32_t)result;
}



/**
 * poll - wait for some event on a file descriptor
 *
 * Syscall number 7
 *
 * int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 */
uint32_t do_poll(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6) {
    struct pollfd *fds32 = (struct pollfd *)(mem + *arg1);
    nfds_t nfds = (nfds_t)*arg2;
    int timeout = (int)*arg3;

    struct pollfd *fds64 = malloc(nfds * sizeof(struct pollfd));
    if (!fds64) {
        return -ENOMEM;
    }

    for (nfds_t i = 0; i < nfds; i++) {
        fds64[i].fd = fds32[i].fd;
        fds64[i].events = fds32[i].events;

    }
    int result = syscall(SYS_poll, fds64, nfds, timeout);


    if (result >= 0) {
        for (nfds_t i = 0; i < nfds; i++) {
            fds32[i].revents = fds64[i].revents;
        }
    }

    free(fds64);
    return (uint32_t)result;
}

/**
 * lseek - reposition read/write file offset
 *
 * Syscall number 8
 *
 * off_t ofgfet must be 64 bits so we use arg3 as high part (edx) and arg2 as low part (ecx). edx||cx, where || is concatenate.
 *
 * off_t lseek(int fd, off_t offset, int whence);
 */
uint32_t do_lseek(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    int fd = (int)*arg1;
    off_t offset = ((uint64_t)(uint32_t)*arg3 << 32) | (uint32_t)*arg2;
    int whence = (int)*arg4;
    return (uint32_t)syscall(SYS_lseek, fd, offset, whence);
}

/**
 * lseek - reposition read/write file offset
 *
 * Syscall number 9
 *
 * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
 */
uint32_t do_mmap(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6, uint32_t){
    void * addr = (void *)(mem+*arg1);
    size_t length = (size_t)*arg2;
    int prot = (int)*arg3;
    int flags = (int)*arg4;
    int fd = (int)*arg5;
    off_t offset = (off_t)*arg6;
    void * dir = (void *)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
    return (uint32_t)(dir - (void *)mem); //return 32bit value that corresponds to a virtual dir within mem array.

}

/**
 * mprotect, pkey_mprotect - set protection on a region of memory
 *
 * Syscall number 10
 *
 * int mprotect(void *addr, size_t len, int prot);
 */
uint32_t do_mprotect(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+ *arg1);
    size_t len = (size_t)*arg2;
    int prot = (int)*arg3;

    return (uint32_t)syscall(SYS_mprotect, addr, len, prot);
}

/**
 *  mmap, munmap - map or unmap files or devices into memory
 *
 *  Syscall number 11
 *
 *  int munmap(void *addr, size_t length);
 */
uint32_t do_munmap(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    size_t length = (size_t)*arg2;

    return (uint32_t)syscall(SYS_munmap, addr, length);
}

/**
 * brk, sbrk - change data segment size
 *
 * Syscall number 12
 *
 * int brk(void *addr);
 */
uint32_t do_brk(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_brk, addr);
}



uint32_t do_rt_sigaction(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number13
uint32_t do_rt_sigprocmask(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number14
uint32_t do_rt_sigreturn(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number15
uint32_t do_ioctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number16

/**
 * pread, pwrite - read from or write to a file descriptor at a given offset
 *
 * Syscall number 17
 *
 * ssize_t pread(int fd, void buf[.count], size_t count, off_t offset);
 */
uint32_t do_pread64(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * buf = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_pread64, (int)*arg1, buf, (size_t)*arg3, (off_t)*arg4);
}

/**
 * pread, pwrite - read from or write to a file descriptor at a given offset
 *
 * Syscall number 18
 *
 * ssize_t pwrite(int fd, const void buf[.count], size_t count, off_t offset);
 */
uint32_t do_pwrite64(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * buf = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_pwrite64, (int)*arg1, buf, (size_t)*arg3, (off_t)*arg4);
}


uint32_t do_readv(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number19
uint32_t do_writev(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number20


/**
 * access, faccessat, faccessat2 - check user's permissions for a file
 *
 * Syscall number 21
 *
 * int access(const char *pathname, int mode);
 */
uint32_t do_access(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_access, pathname, (int)*arg2);
}

/**
 * pipe, pipe2 - create pipe
 *
 * Syscall number 22
 *
 * int pipe(int pipefd[2]);
 */
uint32_t do_pipe(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    int * pipefd = (int *)(mem+*arg1);
    return (uint32_t)syscall(SYS_pipe, pipefd);
}


uint32_t do_select(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number23

/**
 * sched_yield - yield the processor
 *
 * Syscall number 24
 *
 * int sched_yield(void);
 */
uint32_t do_sched_yield(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_sched_yield);
}

/**
 * sched_yield - yield the processor
 *
 * Syscall number 25
 *
 * void *mremap(void old_address[.old_size], size_t old_size, size_t new_size, int flags, ... /* void *new_address );
 */
uint32_t do_mremap(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * old_address = (void *)(mem+*arg1);
    void * new_address = (void *)(mem+*arg5);
    void * res = (void *) syscall(SYS_sched_yield, old_address, (size_t)*arg2, (size_t)*arg3, (int)*arg4, new_address);
    return (uint32_t)(res-(void *)mem); //return a 32bit virtual address
}

/**
 * msync - synchronize a file with a memory map
 *
 * Syscall number 26
 *
 * int msync(void addr[.length], size_t length, int flags);
 */
uint32_t do_msync(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_msync, addr, (size_t)*arg2, (int)*arg3);
}

/**
 * mincore - determine whether pages are resident in memory
 *
 * Syscall number 27
 *
 * int mincore(void addr[.length], size_t length, unsigned char *vec);
 */
uint32_t do_mincore(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    unsigned char * vec = (char*)(mem+*arg3);
    return (uint32_t)syscall(SYS_mincore, addr, (size_t)*arg2, vec);
}

/**
 * madvise - give advice about use of memory
 *
 * Syscall number 28
 *
 * int madvise(void addr[.length], size_t length, int advice);
 */
uint32_t do_madvise(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_madvise, addr, (size_t)*arg2, (int)*arg3);
}

/**
 * shmget - allocates a System V shared memory segment
 *
 * Syscall number 29
 *
 * int shmget(key_t key, size_t size, int shmflg);
 */
uint32_t do_shmget(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_shmget, (key_t)*arg1, (size_t)*arg2, (int)*arg3);
}

/**
 * shmat, shmdt - System V shared memory operations
 *
 * Syscall number 30
 *
 * void *shmat(int shmid, const void *_Nullable shmaddr, int shmflg);
 */
uint32_t do_shmat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void * shmaddr = (void *)(mem+*arg2);
    void * res = (void *) syscall(SYS_shmat, (int)*arg1, shmaddr, (int)*arg3);
    return (uint32_t)(res-(void*)mem);
}


uint32_t do_shmctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number31

/**
 * dup, dup2, dup3 - duplicate a file descriptor
 *
 * Syscall number 32
 *
 * int dup(int oldfd);
 */
uint32_t do_dup(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_dup, (int)*arg1);
}

/**
 * dup, dup2, dup3 - duplicate a file descriptor
 *
 * Syscall number 33
 *
 * int dup2(int oldfd, int newfd);
 */
uint32_t do_dup2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_dup2, (int)*arg1, (int)*arg2);
}

/**
 * pause - wait for signal
 *
 * Syscall number 34
 *
 * int pause(void);
 */
uint32_t do_pause(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_pause);
} 


uint32_t do_nanosleep(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number35
uint32_t do_getitimer(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number36

/**
 * alarm - set an alarm clock for delivery of a signal
 *
 * Syscall number 37
 *
 * unsigned int alarm(unsigned int seconds);
 */
uint32_t do_alarm(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_alarm, (unsigned int)*arg1);
}


uint32_t do_setitimer(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number38

/**
 * getpid, getppid - get process identification
 *
 * Syscall number 39
 *
 * pid_t getpid(void);
 */
uint32_t do_getpid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getpid);
}

/**
 * sendfile - transfer data between file descriptors
 *
 * Syscall number 40
 *
 * ssize_t sendfile(int out_fd, int in_fd, off_t *_Nullable offset, size_t count);
 */
uint32_t do_sendfile(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    off_t * offset = (*arg3)?(off_t *)(mem+*arg3):NULL; //Nullable
    return (uint32_t)syscall(SYS_sendfile, (int)*arg1, (int)*arg2, offset, (size_t)*arg4);
}

/**
 * socket - create an endpoint for communication
 *
 * Syscall number 41
 *
 * int socket(int domain, int type, int protocol);
 */
uint32_t do_socket(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return  (uint32_t)syscall(SYS_socket, (int)*arg1, (int)*arg2, (int)*arg3);
}


uint32_t do_connect(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number42
uint32_t do_accept(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number43
uint32_t do_sendto(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number44
uint32_t do_recvfrom(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number45
uint32_t do_sendmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number46
uint32_t do_recvmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number47
uint32_t do_shutdown(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number48
uint32_t do_bind(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number49

/**
 * listen - listen for connections on a socket
 *
 * Syscall number 50
 *
 * int listen(int sockfd, int backlog);
 */
uint32_t do_listen(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_listen, (int)*arg1, (int)*arg2);
}


uint32_t do_getsockname(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number51
uint32_t do_getpeername(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number52
uint32_t do_socketpair(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number53
uint32_t do_setsockopt(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number54
uint32_t do_getsockopt(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number55
uint32_t do_clone(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number56

/**
 * fork - create a child process
 *
 * Syscall number 57
 *
 * pid_t fork(void);
 */
uint32_t do_fork(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fork);
}

/**
 * vfork - create a child process and block parent
 *
 * Syscall number 58
 *
 * pid_t vfork(void);
 */
uint32_t do_vfork(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_vfork);
}

/**
 * execve - execute program
 *
 * Syscall number 59
 *
 * int execve(const char *pathname, char *const _Nullable argv[], char *const _Nullable envp[]);
 */
uint32_t do_execve(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    //POINTER TO VARIABLE WORKS, BUT IT POINTS TO 32bit INTEGERS; NOT 64bit POINTERS, it must do mem+off 
    // for every offser within pointer to pointers. ???
    const char * pathname = (char *)(mem+*arg1);
    const char** argv = (const char **)(mem+*arg2);
    const char** envp = (const char **)(mem+*arg3);
    return (uint32_t)syscall(SYS_execve, pathname, argv, envp);
}

/**
 * _exit, _Exit - terminate the calling process
 *
 * Syscall number 60
 *
 * [[noreturn]] void _exit(int status);
 */
uint32_t do_exit(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_exit);
}


uint32_t do_wait4(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number61

/**
 * kill - send signal to a process
 *
 * Syscall number 62
 *
 * int kill(pid_t pid, int sig);
 */
uint32_t do_kill(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_kill, (pid_t)*arg1, (int)*arg2);
}


uint32_t do_uname(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number63

/**
 * semget - get a System V semaphore set identifier
 *
 * Syscall number 64
 *
 * int semget(key_t key, int nsems, int semflg);
 */
uint32_t do_semget(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_semget, (key_t)*arg1, (int)*arg2, (int)*arg3);
}


uint32_t do_semop(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number65
uint32_t do_semctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number66

/**
 * shmat, shmdt - System V shared memory operations
 *
 * Syscall number 67
 *
 * int shmdt(const void *shmaddr);
 */
uint32_t do_shmdt(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void * shmaddr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_shmdt, shmaddr);
}

/**
 * msgget - get a System V message queue identifier
 *
 * Syscall number 68
 *
 * int msgget(key_t key, int msgflg);
 */
uint32_t do_msgget(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_msgget, (key_t)*arg1, (int)*arg2);
}

/**
 * msgrcv, msgsnd - System V message queue operations
 *
 * Syscall number 69
 *
 * int msgsnd(int msqid, const void msgp[.msgsz], size_t msgsz, int msgflg);
 */
uint32_t do_msgsnd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void * msgp = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_msgsnd, (int)*arg1, msgp, (size_t)*arg3, (int)*arg4);
}

/**
 * msgrcv, msgsnd - System V message queue operations
 *
 * Syscall number 70
 *
 * ssize_t msgrcv(int msqid, void msgp[.msgsz], size_t msgsz, long msgtyp, int msgflg);
 */
uint32_t do_msgrcv(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void * msgp = (void *)(mem+*arg2);
    return (uint32_t)syscall(SYS_msgrcv, (int)*arg1, msgp, (size_t)*arg3, (long)*arg4, (int)*arg5);
}

uint32_t do_msgctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number71
uint32_t do_fcntl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number72

/**
 * flock - apply or remove an advisory lock on an open file
 *
 * Syscall number 73
 *
 * int flock(int fd, int op);
 */
uint32_t do_flock(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_flock, (int)*arg1, (int)*arg2);
}

/**
 * fsync, fdatasync - synchronize a file's in-core state with storage device
 *
 * Syscall number 74
 *
 * int fsync(int fd);
 */
uint32_t do_fsync(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fsync, (int)*arg1);
}

/**
 * fsync, fdatasync - synchronize a file's in-core state with storage device
 *
 * Syscall number 75
 *
 * int fdatasync(int fd);
 */
uint32_t do_fdatasync(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fdatasync, (int)*arg1);
}

/**
 * truncate, ftruncate - truncate a file to a specified length
 *
 * Syscall number 76
 *
 * int truncate(const char *path, off_t length);
 */
uint32_t do_truncate(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_truncate, path, (off_t)*arg2);
}

/**
 * truncate, ftruncate - truncate a file to a specified length
 *
 * Syscall number 77
 *
 * int ftruncate(int fd, off_t length);
 */
uint32_t do_ftruncate(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_ftruncate, (int)*arg1, (off_t)*arg2);
}


uint32_t do_getdents(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number78

/**
 * getcwd, getwd, get_current_dir_name - get current working directory
 *
 * Syscall number 79
 *
 * char *getcwd(char buf[.size], size_t size);
 */
uint32_t do_getcwd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    char * buf = (char *)(mem+*arg1);
    char * res = (char *) syscall(SYS_getcwd, buf, (size_t)*arg2);
    return (uint32_t)(res-(char*)mem);
}

/**
 * chdir, fchdir - change working directory
 *
 * Syscall number 80
 *
 * int chdir(const char *path);
 */
uint32_t do_chdir(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *path = (char*)(mem+*arg1);
    return (uint32_t)syscall(SYS_chdir, path);
}

/**
 * chdir, fchdir - change working directory
 *
 * Syscall number 81
 *
 * int fchdir(int fd);
 */
uint32_t do_fchdir(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fchdir, (int)*arg1);
}

/**
 * rename, renameat, renameat2 - change the name or location of a file
 *
 * Syscall number 82
 *
 * int rename(const char *oldpath, const char *newpath);
 */
uint32_t do_rename(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * oldpath = (char *)(mem+*arg1);
    const char * newpath = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_rename, oldpath, newpath);
}

/**
 * mkdir, mkdirat - create a directory
 *
 * Syscall number 83
 *
 * int mkdir(const char *pathname, mode_t mode);
 */
uint32_t do_mkdir(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (char*)(mem+*arg1);
    return (uint32_t)syscall(SYS_mkdir, pathname, (mode_t)*arg2);
}

/**
 * mkdir, mkdirat - create a directory
 *
 * Syscall number 84
 *
 * int rmdir(const char *pathname);
 */
uint32_t do_rmdir(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (char*)(mem+*arg1);
    return (uint32_t)syscall(SYS_rmdir, pathname);
}

/**
 * open, openat, creat - open and possibly create a file
 *
 * Syscall number 85
 *
 * int creat(const char *pathname, mode_t mode);
 */
uint32_t do_creat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg1);
    mode_t mode = (mode_t)*arg2;

    return (uint32_t)syscall(SYS_creat, pathname, mode);
}

/**
 * link, linkat - make a new name for a file
 *
 * Syscall number 86
 *
 * int link(const char *oldpath, const char *newpath);
 */
uint32_t do_link(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *oldpath = (char *)(mem+*arg1);
    const char *newpath = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_link, oldpath, newpath);
}

/**
 * unlink, unlinkat - delete a name and possibly the file it refers to
 *
 * Syscall number 87
 *
 * int unlink(const char *pathname);
 */
uint32_t do_unlink(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *pathname = (char*)(mem+*arg1);
    return (uint32_t)syscall(SYS_unlink, pathname);
}

/**
 * symlink, symlinkat - make a new name for a file
 *
 * Syscall number 88
 *
 * int symlink(const char *target, const char *linkpath);
 */
uint32_t do_symlink(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *target = (char *)(mem+*arg1);
    const char * linkpath = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_symlink, target, linkpath);

}

/**
 * readlink, readlinkat - read value of a symbolic link
 *
 * Syscall number 89
 *
 * ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz);
 */
uint32_t do_readlink(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *restrict pathname = (char *)(mem+*arg1);
    char *restrict buf = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_readlink, pathname, buf, (size_t)*arg3);
}

/**
 * chmod, fchmod, fchmodat - change permissions of a file
 *
 * Syscall number 90
 *
 * int chmod(const char *pathname, mode_t mode);
 */
uint32_t do_chmod(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_chmod, pathname, (mode_t)*arg2);
}

/**
 * chmod, fchmod, fchmodat - change permissions of a file
 *
 * Syscall number 91
 *
 * int fchmod(int fd, mode_t mode);
 */
uint32_t do_fchmod(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fchmod, (int)*arg1, (mode_t)*arg2);
}

/**
 * chown, fchown, lchown, fchownat - change ownership of a file
 *
 * Syscall number 92
 *
 * int chown(const char *pathname, uid_t owner, gid_t group);
 */
uint32_t do_chown(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem + *arg1);
    return (uint32_t)syscall(SYS_chown, pathname, (uid_t)*arg2, (gid_t)*arg3);
}

/**
 * chown, fchown, lchown, fchownat - change ownership of a file
 *
 * Syscall number 93
 *
 * int fchown(int fd, uid_t owner, gid_t group);
 */
uint32_t do_fchown(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_chown, (int)*arg1, (uid_t)*arg2, (gid_t)*arg3);
}

/**
 * chown, fchown, lchown, fchownat - change ownership of a file
 *
 * Syscall number 94
 *
 * int lchown(const char *pathname, uid_t owner, gid_t group);
 */
uint32_t do_lchown(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem + *arg1);
    return (uint32_t)syscall(SYS_chown, pathname, (uid_t)*arg2, (gid_t)*arg3);
}

/**
 * umask - set file mode creation mask
 *
 * Syscall number 95
 *
 * mode_t umask(mode_t mask);
 */
uint32_t do_umask(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_umask, (mode_t)*arg1);
}


uint32_t do_gettimeofday(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number96
uint32_t do_getrlimit(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number97
uint32_t do_getrusage(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number98
uint32_t do_sysinfo(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number99
uint32_t do_times(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number100

/**
 * ptrace - process trace
 *
 * Syscall number 101
 *
 * long ptrace(enum __ptrace_request op, pid_t pid, void *addr, void *data); ???long
 */
uint32_t do_ptrace(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg3);
    void * data = (void *)(mem+*arg4);
    return (uint32_t)syscall(SYS_ptrace, (enum __ptrace_request)*arg1, (pid_t)*arg2, addr, data);
}

/**
 *  getuid, geteuid - get user identity
 *
 *  Syscall number 102
 *
 *  uid_t getuid(void);
 */
uint32_t do_getuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getuid);
} 

/**
 *  syslog, klogctl - read and/or clear kernel message ring buffer; set console_loglevel
 *
 *  Syscall number 103
 *
 *  int syslog(int type, char *bufp, int len);
 */
uint32_t do_syslog(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    char * bufp = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_syslog, (int)*arg1, bufp, (int)*arg3);
}

/**
 *  getgid, getegid - get group identity
 *
 *  Syscall number 104
 *
 *  gid_t getgid(void);
 */
uint32_t do_getgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getgid);
}

/**
 *  setuid - set user identity
 *
 *  Syscall number 105
 *
 *  int setuid(uid_t uid);
 */
uint32_t do_setuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setuid, (uid_t)*arg1);
}

/**
 *  setgid - set group identity
 *
 *  Syscall number 106
 *
 *  int setgid(gid_t gid);
 */
uint32_t do_setgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setgid, (gid_t)*arg1);
}

/**
 *  getuid, geteuid - get user identity
 *
 *  Syscall number 107
 *
 *  uid_t geteuid(void);
 */
uint32_t do_geteuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_geteuid);
}

/**
 *  getgid, getegid - get group identity
 *
 *  Syscall number 108
 *
 *  gid_t getegid(void);
 */
uint32_t do_getegid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getegid);
}

/**
 *  getgid, getegid - get group identity
 *
 *  Syscall number 109
 *
 *  int setpgid(pid_t pid, pid_t pgid);
 */
uint32_t do_setpgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setpgid, (pid_t)*arg1, (pid_t)*arg2);
}

/**
 *  getpid, getppid - get process identification
 *
 *  Syscall number 110
 *
 *  pid_t getppid(void);
 */
uint32_t do_getppid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getppid);
}


/**
 *  setpgid, getpgid, setpgrp, getpgrp - set/get process group
 *
 *  Syscall number 111
 *
 *  pid_t getpgrp(void);                  POSIX.1 version <--
 *  pid_t getpgrp(pid_t pid);             BSD version 
 */
uint32_t do_getpgrp(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getpgrp);
}

/**
 *  setsid - creates a session and sets the process group ID
 *
 *  Syscall number 112
 *
 *  pid_t setsid(void);
 */
uint32_t do_setsid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setsid);
}

/**
 *  setreuid, setregid - set real and/or effective user or group ID
 *
 *  Syscall number 113
 *
 *  int setreuid(uid_t ruid, uid_t euid);
 */
uint32_t do_setreuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setreuid, (uid_t)*arg1, (uid_t)*arg2);
}

/**
 *  setreuid, setregid - set real and/or effective user or group ID
 *
 *  Syscall number 114
 *
 *  int setregid(gid_t rgid, gid_t egid);
 */
uint32_t do_setregid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setregid, (gid_t)*arg1, (gid_t)*arg2);
}

/**
 *  getgroups, setgroups - get/set list of supplementary group IDs
 *
 *  Syscall number 115
 *
 *  int getgroups(int size, gid_t list[]); ???
 */
uint32_t do_getgroups(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    gid_t *list = (gid_t *)(mem+*arg2);
    return (uint32_t)syscall(SYS_getgroups, (int)*arg1, list);
}

/**
 *  getgroups, setgroups - get/set list of supplementary group IDs
 *
 *  Syscall number 116
 *
 *  int setgroups(size_t size, const gid_t *list);
 */
uint32_t do_setgroups(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const gid_t *list = (gid_t*)(mem+*arg2);
    return (uint32_t)syscall(SYS_setgroups, (size_t)*arg1, list);
}

/**
 *  setresuid, setresgid - set real, effective and saved user or group ID
 *
 *  Syscall number 117
 *
 *  int setresuid(uid_t ruid, uid_t euid, uid_t suid);
 */
uint32_t do_setresuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setresuid, (uid_t)*arg1, (uid_t)*arg2, (uid_t)*arg3);
}

/**
 *  getresuid, getresgid - get real, effective and saved user/group IDs
 *
 *  Syscall number 118
 *
 *  int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
 */
uint32_t do_getresuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    uid_t * ruid = (uid_t *)(mem+*arg1);
    uid_t * euid = (uid_t *)(mem+*arg2);
    uid_t * suid = (uid_t *)(mem+*arg3);
    return (uint32_t)syscall(SYS_getresuid, ruid, euid, suid);
}

/**
 *  setresuid, setresgid - set real, effective and saved user or group ID
 *
 *  Syscall number 119
 *
 *  int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
 */
uint32_t do_setresgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setresgid, (gid_t)*arg1, (gid_t)*arg2, (gid_t)*arg3);
}

/**
 *  getresuid, getresgid - get real, effective and saved user/group IDs
 *
 *  Syscall number 120
 *
 *  int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
 */
uint32_t do_getresgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    gid_t * rgid = (gid_t *)(mem+*arg1);
    gid_t * egid = (gid_t *)(mem+*arg2);
    gid_t * sgid = (gid_t *)(mem+*arg3);
    return (uint32_t)syscall(SYS_getresgid, rgid, egid, sgid);
}

/**
 *  setpgid, getpgid, setpgrp, getpgrp - set/get process group
 *
 *  Syscall number 121
 *
 *  pid_t getpgid(pid_t pid);
 */
uint32_t do_getpgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getpgid, (pid_t)*arg1);
}

/**
 *  setfsuid - set user identity used for filesystem checks
 *
 *  Syscall number 122
 *
 *  int setfsuid(uid_t fsuid);
 */
uint32_t do_setfsuid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setfsuid, (uid_t)*arg1);
}

/**
 *  setfsgid - set group identity used for filesystem checks
 *
 *  Syscall number 123
 *
 *  int setfsgid(uid_t fsgid);
 */
uint32_t do_setfsgid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setfsgid, (uid_t)*arg1);
}

/**
 *  getsid - get session ID
 *
 *  Syscall number 124
 *
 *   pid_t getsid(pid_t pid);
 */
uint32_t do_getsid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getsid, (pid_t)*arg1);
}

uint32_t do_capget(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number125
uint32_t do_capset(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number126
uint32_t do_rt_sigpending(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number127
uint32_t do_rt_sigtimedwait(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number128
uint32_t do_rt_sigqueueinfo(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number129
uint32_t do_rt_sigsuspend(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number130
uint32_t do_sigaltstack(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number131
uint32_t do_utime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number132

/**
 *  mknod, mknodat - create a special or ordinary file
 *
 *  Syscall number 133
 *
 *  int mknod(const char *pathname, mode_t mode, dev_t dev);
 */
uint32_t do_mknod(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_mknod, pathname, (mode_t)*arg2, (dev_t)*arg3);
}



/**
 *  uselib - load shared library
 *
 *  Syscall number 134
 *
 *  [[deprecated]] int uselib(const char *library);
 */
uint32_t do_uselib(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * library = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_uselib, library);
}

/**
 *  personality - set the process execution domain
 *
 *  Syscall number 135
 *
 *  int personality(unsigned long persona);
 */
uint32_t do_personality(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_personality, (unsigned long)*arg1);
} 


uint32_t do_ustat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number136
uint32_t do_statfs(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number137
uint32_t do_fstatfs(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number138

/**
 *  sysfs - get filesystem type information
 *
 *  Syscall number 139
 *
 *  [[deprecated]] int sysfs(int option, const char *fsname);
 *  [[deprecated]] int sysfs(int option, unsigned int fs_index, char *buf);
 *  [[deprecated]] int sysfs(int option);
 */
uint32_t do_sysfs(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    switch(*arg1){ /*option*/
        case 1:
            const char * fsname = (char *)(mem+*arg2);
            return (uint32_t)syscall(SYS_sysfs, (int)*arg1, fsname);
        case 2:
            char * buf = (char *)(mem+*arg3);
            return (uint32_t)syscall(SYS_sysfs, (int)*arg1, (unsigned int)*arg2, buf);
        case 3:
        default:
            return (uint32_t)syscall(SYS_sysfs, (int)*arg1);
    }
}

/**
 *  getpriority, setpriority - get/set program scheduling priority
 *
 *  Syscall number 140
 *
 *  int getpriority(int which, id_t who);
 */
uint32_t do_getpriority(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_getpriority, (int)*arg1, (id_t)*arg2);
}

/**
 *  getpriority, setpriority - get/set program scheduling priority
 *
 *  Syscall number 141
 *
 *  int setpriority(int which, id_t who, int prio);
 */
uint32_t do_setpriority(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_setpriority, (int)*arg1, (id_t)*arg2, (int)*arg3);
}


uint32_t do_sched_setparam(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number142
uint32_t do_sched_getparam(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number143
uint32_t do_sched_setscheduler(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number144
uint32_t do_sched_getscheduler(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number145

/**
 *  sched_get_priority_max, sched_get_priority_min  - get static priority range
 *
 *  Syscall number 146
 *
 *  int sched_get_priority_max(int policy);
 */
uint32_t do_sched_get_priority_max(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_sched_get_priority_max, (int)*arg1);
}

/**
 *  sched_get_priority_max, sched_get_priority_min  - get static priority range
 *
 *  Syscall number 147
 *
 *  int sched_get_priority_min(int policy);
 */
uint32_t do_sched_get_priority_min(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_sched_get_priority_min, (int)*arg1);
}

uint32_t do_sched_rr_get_interval(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number148

/**
 *  mlock, mlock2, munlock, mlockall, munlockall - lock and unlock memory
 *
 *  Syscall number 149
 *
 *  int mlock(const void addr[.len], size_t len);
 */
uint32_t do_mlock(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void *addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_mlock, addr, (size_t)*arg2);
}

/**
 *  mlock, mlock2, munlock, mlockall, munlockall - lock and unlock memory
 *
 *  Syscall number 150
 *
 *  int munlock(const void addr[.len], size_t len);
 */
uint32_t do_munlock(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const void *addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_munlock, addr, (size_t)*arg2);
}

/**
 *  mlock, mlock2, munlock, mlockall, munlockall - lock and unlock memory
 *
 *  Syscall number 151
 *
 *  int mlockall(int flags);
 */
uint32_t do_mlockall(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_mlockall, (int)*arg1);
}

/**
 *  mlock, mlock2, munlock, mlockall, munlockall - lock and unlock memory
 *
 *  Syscall number 152
 *
 *  int munlockall(void);
 */
uint32_t do_munlockall(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_munlockall);
}

/**
 *  vhangup - virtually hangup the current terminal
 *
 *  Syscall number 153
 *
 *  int vhangup(void);
 */
uint32_t do_vhangup(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_vhangup);
}

/**
 *  modify_ldt - get or set a per-process LDT entry
 *
 *  Syscall number 154
 *
 *  int syscall(SYS_modify_ldt, int func, void ptr[.bytecount], unsigned long bytecount); ???
 */
uint32_t do_modify_ldt(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * ptr = (void *)(mem + *arg2);
    return (uint32_t)syscall(SYS_modify_ldt, (int)*arg1, ptr, (unsigned long)*arg3);
}

/**
 *  pivot_root - change the root mount
 *
 *  Syscall number 155
 *
 *  int syscall(SYS_pivot_root, const char *new_root, const char *put_old);
 */
uint32_t do_pivot_root(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char *new_root = (void *)(mem + *arg1);
    const char *put_old = (void *)(mem + *arg2);
    return (uint32_t)syscall(SYS_pivot_root, new_root, put_old);
}

uint32_t do__sysctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number156
uint32_t do_prctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number157
uint32_t do_arch_prctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number158
uint32_t do_adjtimex(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number159
uint32_t do_setrlimit(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number160

/**
 *  chroot - change root directory
 *
 *  Syscall number 161
 *
 *  int chroot(const char *path);
 */
uint32_t do_chroot(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem + *arg1);
    return (uint32_t)syscall(SYS_chroot, path);
}

/**
 *  sync, syncfs - commit filesystem caches to disk
 *
 *  Syscall number 162
 *
 *  void sync(void);
 */
uint32_t do_sync(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_sync);
} 

/**
 *  acct - switch process accounting on or off
 *
 *  Syscall number 163
 *
 *  int acct(const char *_Nullable filename);
 */
uint32_t do_acct(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * filename = (*arg1)?(char *)(mem+*arg1):NULL;
    return (uint32_t)syscall(SYS_acct, filename);
}

uint32_t do_settimeofday(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number164

/**
 *  mount - mount filesystem
 *
 *  Syscall number 165
 *
 *  int mount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *_Nullable data);
 */
uint32_t do_mount(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * source = (char *)(mem+*arg1);
    const char * target = (char *)(mem+*arg2);
    const char * filesystemtype = (char *)(mem+*arg3);
    const char * data = (*arg5)?(char *)(mem+*arg5):NULL;

    return (uint32_t)syscall(SYS_mount, source, target, filesystemtype, (unsigned long)*arg4, data);
}

/**
 *  umount, umount2 - unmount filesystem
 *
 *  Syscall number 166
 *
 *  int umount2(const char *target, int flags);
 */
uint32_t do_umount2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * target = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_umount2, target, (int)*arg2);
}

/**
 *  swapon, swapoff - start/stop swapping to file/device
 *
 *  Syscall number 167
 *
 *  int swapon(const char *path, int swapflags);
 */
uint32_t do_swapon(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_swapon, path, (int)*arg2);
}

/**
 *  swapon, swapoff - start/stop swapping to file/device
 *
 *  Syscall number 168
 *
 *  int swapoff(const char *path);
 */
uint32_t do_swapoff(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_swapoff, path);
}

/**
 *  reboot - reboot or enable/disable Ctrl-Alt-Del
 *
 *  Syscall number 169
 *
 *  int reboot(int op);
 */
uint32_t do_reboot(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_reboot, (int)*arg1);
}

/**
 *  gethostname, sethostname - get/set hostname
 *
 *  Syscall number 170
 *
 *  int sethostname(const char *name, size_t len);
 */
uint32_t do_sethostname(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * name = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_sethostname, name, (size_t)*arg2);
}

/**
 *  getdomainname, setdomainname - get/set NIS domain name
 *
 *  Syscall number 171
 *
 *  int setdomainname(const char *name, size_t len);
 */
uint32_t do_setdomainname(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * name = (char *)(mem+*arg1);
    return (uint32_t)syscall(SYS_setdomainname, name, (size_t)*arg2);
}

/**
 *  iopl - change I/O privilege level
 *
 *  Syscall number 172
 *
 *  [[deprecated]] int iopl(int level);
 */
uint32_t do_iopl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_iopl, (int)*arg1);
}

/**
 *  ioperm - set port input/output permissions
 *
 *  Syscall number 173
 *
 *  int ioperm(unsigned long from, unsigned long num, int turn_on);
 */
uint32_t do_ioperm(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_ioperm, (unsigned long)*arg1, (unsigned long)*arg2, (int)*arg3);
}


uint32_t do_create_module(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number174
uint32_t do_init_module(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number175
uint32_t do_delete_module(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number176
uint32_t do_get_kernel_syms(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number177
uint32_t do_query_module(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number178
uint32_t do_quotactl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number179
uint32_t do_nfsservctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number180

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 181
 */
uint32_t do_getpmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){}

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 182
 */
uint32_t do_putpmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){}

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 183
 */
uint32_t do_afs_syscall(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){}

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 184
 */
uint32_t do_tuxcall(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} 

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 185
 */
uint32_t do_security(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){}

/**
 *  gettid - get thread identification
 *
 *  Syscall number 186
 *
 *  pid_t gettid(void);
 */
uint32_t do_gettid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_gettid);
}

/**
 *  readahead - initiate file readahead into page cache
 *
 *  Syscall number 187
 *
 *  ssize_t readahead(int fd, off_t offset, size_t count);
 */
uint32_t do_readahead(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_readahead, (int)*arg1, (off_t)*arg2, (size_t)*arg3);
}

/**
 *  readahead - initiate file readahead into page cache
 *
 *  Syscall number 188
 *
 *  int setxattr(const char *path, const char *name, const void value[.size], size_t size, int flags);
 */
uint32_t do_setxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    const void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_setxattr, path, name, value, (size_t)*arg4, (int)*arg5);
}

/**
 *  readahead - initiate file readahead into page cache
 *
 *  Syscall number 189
 *
 *  int lsetxattr(const char *path, const char *name, const void value[.size], size_t size, int flags);
 */
uint32_t do_lsetxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    const void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_lsetxattr, path, name, value, (size_t)*arg4, (int)*arg5);
}

/**
 *  readahead - initiate file readahead into page cache
 *
 *  Syscall number 190
 *
 *  int fsetxattr(int fd, const char *name, const void value[.size], size_t size, int flags);
 */
uint32_t do_fsetxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * name = (char *)(mem+*arg2);
    const void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_fsetxattr, (int)*arg1, name, value, (size_t)*arg4, (int)*arg5);
}

/**
 *  getxattr, lgetxattr, fgetxattr - retrieve an extended attribute value
 *
 *  Syscall number 191
 *
 *  ssize_t getxattr(const char *path, const char *name, void value[.size], size_t size);
 */
uint32_t do_getxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_getxattr, path, name, value, (size_t)*arg4);
}

/**
 *  getxattr, lgetxattr, fgetxattr - retrieve an extended attribute value
 *
 *  Syscall number 192
 *
 *  ssize_t lgetxattr(const char *path, const char *name, void value[.size], size_t size);
 */
uint32_t do_lgetxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_lgetxattr, path, name, value, (size_t)*arg4);
}

/**
 *  getxattr, lgetxattr, fgetxattr - retrieve an extended attribute value
 *
 *  Syscall number 193
 *
 *  ssize_t fgetxattr(int fd, const char *name, void value[.size], size_t size);
 */
uint32_t do_fgetxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * name = (char *)(mem+*arg2);
    void * value = (void *)(mem+*arg3);

    return (uint32_t)syscall(SYS_fgetxattr, (int)*arg1, name, value, (size_t)*arg4);
}

/**
 *  listxattr, llistxattr, flistxattr - list extended attribute names
 *
 *  Syscall number 194
 *
 *  ssize_t listxattr(const char *path, char *_Nullable list, size_t size);
 */
uint32_t do_listxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    char * list = (*arg2)?(char *)(mem+*arg2):NULL;
    return (uint32_t)syscall(SYS_listxattr, path, list, (size_t)*arg3);
}

/**
 *  listxattr, llistxattr, flistxattr - list extended attribute names
 *
 *  Syscall number 195
 *
 *  ssize_t llistxattr(const char *path, char *_Nullable list, size_t size);
 */
uint32_t do_llistxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    char * list = (*arg2)?(char *)(mem+*arg2):NULL;
    return (uint32_t)syscall(SYS_llistxattr, path, list, (size_t)*arg3);
}

/**
 *  listxattr, llistxattr, flistxattr - list extended attribute names
 *
 *  Syscall number 196
 *
 *  ssize_t flistxattr(int fd, char *_Nullable list, size_t size);
 */
uint32_t do_flistxattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    char * list = (*arg2)?(char *)(mem+*arg2):NULL;
    return (uint32_t)syscall(SYS_llistxattr, (int)*arg1, list, (size_t)*arg3);
}

/**
 *  removexattr, lremovexattr, fremovexattr - remove an extended attribute
 *
 *  Syscall number 197
 *
 *  int removexattr(const char *path, const char *name);
 */
uint32_t do_removexattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_removexattr, path, name);
}

/**
 *  removexattr, lremovexattr, fremovexattr - remove an extended attribute
 *
 *  Syscall number 198
 *
 *  int lremovexattr(const char *path, const char *name);
 */
uint32_t do_lremovexattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * path = (char *)(mem+*arg1);
    const char * name = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_lremovexattr, path, name);
}

/**
 *  removexattr, lremovexattr, fremovexattr - remove an extended attribute
 *
 *  Syscall number 199
 *
 *  int fremovexattr(int fd, const char *name);
 */
uint32_t do_fremovexattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * name = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_lremovexattr, (int)*arg1, name);
}

/**
 *  tkill, tgkill - send a signal to a thread
 *
 *  Syscall number 200
 *
 *  [[deprecated]] int syscall(SYS_tkill, pid_t tid, int sig);
 */
uint32_t do_tkill(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_tkill, (pid_t)*arg1, (int) *arg2);
}

/**
 *  time - get time in seconds
 *
 *  Syscall number 201
 *
 *  time_t time(time_t *_Nullable tloc);
 */
uint32_t do_time(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    if (*arg1)
    {return (uint32_t)syscall(SYS_time, (time_t)*arg1);}
    else
    {return (uint32_t)syscall(SYS_time, NULL);}
    
}

uint32_t do_futex(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number202
uint32_t do_sched_setaffinity(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number203
uint32_t do_sched_getaffinity(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number204
uint32_t do_set_thread_area(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number205
uint32_t do_io_setup(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number206
uint32_t do_io_destroy(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number207
uint32_t do_io_getevents(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number208
uint32_t do_io_submit(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number209
uint32_t do_io_cancel(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number210
uint32_t do_get_thread_area(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number211
uint32_t do_lookup_dcookie(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number212

/**
 *  epoll_create, epoll_create1 - open an epoll file descriptor
 *
 *  Syscall number 213
 *
 *  int epoll_create(int size);
 */
uint32_t do_epoll_create(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_epoll_create, (int)*arg1); 
}

uint32_t do_epoll_ctl_old(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number214
uint32_t do_epoll_wait_old(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number215

/**
 *  remap_file_pages - create a nonlinear file mapping
 *
 *  Syscall number 216
 *
 *  [[deprecated]] int remap_file_pages(void addr[.size], size_t size, int prot, size_t pgoff, int flags);
 */
uint32_t do_remap_file_pages(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    void * addr = (void *)(mem+*arg1);
    return (uint32_t)syscall(SYS_remap_file_pages, addr, (size_t)*arg2, (int)*arg3, (size_t)*arg4, (int)*arg5);
}
uint32_t do_getdents64(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number217

/**
 *  set_tid_address - set pointer to thread ID
 *
 *  Syscall number 218
 *
 *  pid_t syscall(SYS_set_tid_address, int *tidptr);
 */
uint32_t do_set_tid_address(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    int * tidptr = (int *)(mem+*arg1);
    return (uint32_t)syscall(SYS_set_tid_address, tidptr);
}

/**
 *  restart_syscall - restart a system call after interruption by a stop signal
 *
 *  Syscall number 219
 *
 *  long restart_syscall(void);
 */
uint32_t do_restart_syscall(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_restart_syscall);
}


uint32_t do_semtimedop(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number220

/**
 *  posix_fadvise - predeclare an access pattern for file data
 *
 *  Syscall number 221
 *
 *  int posix_fadvise(int fd, off_t offset, off_t len, int advice);
 */
uint32_t do_fadvise64(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_fadvise64, (int)*arg1, (off_t)*arg2, (off_t)*arg3, (int)*arg4);
}
uint32_t do_timer_create(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number222
uint32_t do_timer_settime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number223
uint32_t do_timer_gettime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number224
uint32_t do_timer_getoverrun(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number225
uint32_t do_timer_delete(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number226
uint32_t do_clock_settime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number227
uint32_t do_clock_gettime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number228
uint32_t do_clock_getres(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number229
uint32_t do_clock_nanosleep(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number230

/**
 *  exit_group - exit all threads in a process
 *
 *  Syscall number 231
 *
 *  [[noreturn]] void syscall(SYS_exit_group, int status);
 */
uint32_t do_exit_group(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_exit_group, (int)*arg1);
}


uint32_t do_epoll_wait(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number232
uint32_t do_epoll_ctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number233

/**
 *  tkill, tgkill - send a signal to a thread
 *
 *  Syscall number 234
 *
 *  int tgkill(pid_t tgid, pid_t tid, int sig);
 */
uint32_t do_tgkill(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_tgkill, (pid_t)*arg1, (pid_t)*arg2, (int)*arg3);
}

uint32_t do_utimes(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number235

/* 
 * UNIMPLEMENTED 
 *
 * Syscall number 236
 */
uint32_t do_vserver(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){}
uint32_t do_mbind(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number237
uint32_t do_set_mempolicy(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number238
uint32_t do_get_mempolicy(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number239
uint32_t do_mq_open(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number240
uint32_t do_mq_unlink(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number241
uint32_t do_mq_timedsend(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number242
uint32_t do_mq_timedreceive(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number243
uint32_t do_mq_notify(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number244
uint32_t do_mq_getsetattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number245
uint32_t do_kexec_load(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number246
uint32_t do_waitid(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number247
uint32_t do_add_key(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number248
uint32_t do_request_key(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number249
uint32_t do_keyctl(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number250
uint32_t do_ioprio_set(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number251
uint32_t do_ioprio_get(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number252
uint32_t do_inotify_init(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number253
uint32_t do_inotify_add_watch(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number254
uint32_t do_inotify_rm_watch(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number255
uint32_t do_migrate_pages(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number256

/**
 *  open, openat, creat - open and possibly create a file
 *
 *  Syscall number 257
 *
 *  int openat(int dirfd, const char *pathname, int flags);
 *  int openat(int dirfd, const char *pathname, int flags, mode_t mode);
 */
uint32_t do_openat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    int flags = (int)*arg3;
    mode_t mode = (mode_t)*arg4;


    // Decide si llamar a open(2) con 2 o 3 argumentos
    if (flags & O_CREAT) {
        return (uint32_t)syscall(SYS_openat, (int)*arg1, pathname, flags, mode); // open(path, flags, mode)
    } else {
        return (uint32_t)syscall(SYS_openat, (int)*arg1, pathname, flags);       // open(path, flags)
    }

}


/**
 *  mkdir, mkdirat - create a directory
 *
 *  Syscall number 258
 *
 *  int mkdirat(int dirfd, const char *pathname, mode_t mode);
 */
uint32_t do_mkdirat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_mkdirat, (int)*arg1, pathname, (mode_t)*arg3);
}

/**
 *  mknod, mknodat - create a special or ordinary file
 *
 *  Syscall number 259
 *
 *  int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);
 */
uint32_t do_mknodat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_mknodat, (int)*arg1, pathname, (mode_t)*arg3, (dev_t)*arg4);
}

/**
 *  chown, fchown, lchown, fchownat - change ownership of a file
 *
 *  Syscall number 260
 *
 *  int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
 */
uint32_t do_fchownat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_fchownat, (int)*arg1, pathname, (uid_t)*arg3, (gid_t)*arg4, (int)*arg5);
}

uint32_t do_futimesat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number261
uint32_t do_newfstatat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number262

/**
 *  unlink, unlinkat - delete a name and possibly the file it refers to
 *
 *  Syscall number 263
 *
 *  int unlinkat(int dirfd, const char *pathname, int flags);
 */
uint32_t do_unlinkat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_unlinkat, (int)*arg1, pathname, (int)*arg3);
}

/**
 *  rename, renameat, renameat2 - change the name or location of a file
 *
 *  Syscall number 264
 *
 *  int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
 */
uint32_t do_renameat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * oldpath = (char *)(mem+*arg2);
    const char * newpath = (char *)(mem+*arg4);
    return (uint32_t)syscall(SYS_renameat, (int)*arg1, oldpath, (int)*arg3, newpath);
}

/**
 *  link, linkat - make a new name for a file
 *
 *  Syscall number 265
 *
 *  int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
 */
uint32_t do_linkat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * oldpath = (char *)(mem+*arg2);
    const char * newpath = (char *)(mem+*arg4);
    return (uint32_t)syscall(SYS_linkat, (int)*arg1, oldpath, (int)*arg3, newpath, (int)*arg5);
}

/**
 *  symlink, symlinkat - make a new name for a file
 *
 *  Syscall number 266
 *
 *  int symlinkat(const char *target, int newdirfd, const char *linkpath);
 */
uint32_t do_symlinkat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * target = (char *)(mem+*arg1);
    const char * linkpath = (char *)(mem+*arg3);
    return (uint32_t)syscall(SYS_symlinkat, target, (int)*arg2, linkpath);
}

/**
 *  readlink, readlinkat - read value of a symbolic link
 *
 *  Syscall number 267
 *
 *  ssize_t readlinkat(int dirfd, const char *restrict pathname, char *restrict buf, size_t bufsiz);
 */
uint32_t do_readlinkat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    char * restrict buf = (char *)(mem+*arg3);
    return (uint32_t)syscall(SYS_readlinkat, (int)*arg1, pathname, buf, (size_t)*arg4);
}

/**
 *  chmod, fchmod, fchmodat - change permissions of a file
 *
 *  Syscall number 268
 *
 *  int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
 */
uint32_t do_fchmodat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_fchmodat, (int)*arg1, pathname, (mode_t)*arg3, (int)*arg4);
}

/**
 *  access, faccessat, faccessat2 - check user's permissions for a file
 *
 *  Syscall number 269
 *
 *  int faccessat(int dirfd, const char *pathname, int mode, int flags);
 */
uint32_t do_faccessat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    const char * pathname = (char *)(mem+*arg2);
    return (uint32_t)syscall(SYS_faccessat, (int)*arg1, pathname, (int)*arg3, (int)*arg4);
}

uint32_t do_pselect6(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number270

/**
 * poll, ppoll - wait for some event on a file descriptor
 *
 * Syscall number 271
 *
 * int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *_Nullable tmo_p, const sigset_t *_Nullable sigmask);
 */
uint32_t do_ppoll(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    struct pollfd *fds32 = (struct pollfd *)(mem + *arg1);
    nfds_t nfds = (nfds_t)*arg2;
    const struct timespec32 {
        int32_t tv_sec;
        int32_t tv_nsec;
    } *tmo_p32 = (const struct timespec32 *)(mem + *arg3);
    const sigset_t *sigmask = (const sigset_t *)(mem + *arg4);

    struct pollfd *fds64 = malloc(nfds * sizeof(struct pollfd));
    if (!fds64) {
        return -ENOMEM;
    }

    for (nfds_t i = 0; i < nfds; i++) {
        fds64[i].fd = fds32[i].fd;
        fds64[i].events = fds32[i].events;
    }


    struct timespec tmo_p64;
    struct timespec *tmo_p64_ptr = NULL;

    if (tmo_p32) {
        tmo_p64.tv_sec = tmo_p32->tv_sec;
        tmo_p64.tv_nsec = tmo_p32->tv_nsec;
        tmo_p64_ptr = &tmo_p64;
    }


    int result = syscall(SYS_ppoll, fds64, nfds, tmo_p64_ptr, sigmask, NSIG/8);


    if (result >= 0) {
        for (nfds_t i = 0; i < nfds; i++) {
            fds32[i].revents = fds64[i].revents;
        }
    }

    free(fds64);
    return (uint32_t)result;
}

/**
 * unshare - disassociate parts of the process execution context
 *
 * Syscall number 272
 *
 * int unshare(int flags);
 */
uint32_t do_unshare(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){
    return (uint32_t)syscall(SYS_unshare, (int)*arg1);
}

uint32_t do_set_robust_list(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number273
uint32_t do_get_robust_list(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number274
uint32_t do_splice(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number275
uint32_t do_tee(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number276
uint32_t do_sync_file_range(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number277
uint32_t do_vmsplice(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number278
uint32_t do_move_pages(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number279
uint32_t do_utimensat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number280
uint32_t do_epoll_pwait(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number281
uint32_t do_signalfd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number282
uint32_t do_timerfd_create(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number283
uint32_t do_eventfd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number284
uint32_t do_fallocate(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number285
uint32_t do_timerfd_settime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number286
uint32_t do_timerfd_gettime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number287
uint32_t do_accept4(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number288
uint32_t do_signalfd4(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number289
uint32_t do_eventfd2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number290
uint32_t do_epoll_create1(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number291
uint32_t do_dup3(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number292
uint32_t do_pipe2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number293
uint32_t do_inotify_init1(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number294
uint32_t do_preadv(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number295
uint32_t do_pwritev(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number296
uint32_t do_rt_tgsigqueueinfo(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number297
uint32_t do_perf_event_open(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number298
uint32_t do_recvmmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number299
uint32_t do_fanotify_init(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number300
uint32_t do_fanotify_mark(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number301
uint32_t do_prlimit64(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number302
uint32_t do_name_to_handle_at(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number303
uint32_t do_open_by_handle_at(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number304
uint32_t do_clock_adjtime(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number305
uint32_t do_syncfs(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number306
uint32_t do_sendmmsg(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number307
uint32_t do_setns(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number308
uint32_t do_getcpu(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number309
uint32_t do_process_vm_readv(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number310
uint32_t do_process_vm_writev(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number311
uint32_t do_kcmp(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number312
uint32_t do_finit_module(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number313
uint32_t do_sched_setattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number314
uint32_t do_sched_getattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number315
uint32_t do_renameat2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number316
uint32_t do_seccomp(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number317
uint32_t do_getrandom(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number318
uint32_t do_memfd_create(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number319
uint32_t do_kexec_file_load(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number320
uint32_t do_bpf(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number321
uint32_t do_execveat(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number322
uint32_t do_userfaultfd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number323
uint32_t do_membarrier(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number324
uint32_t do_mlock2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number325
uint32_t do_copy_file_range(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number326
uint32_t do_preadv2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number327
uint32_t do_pwritev2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number328
uint32_t do_pkey_mprotect(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number329
uint32_t do_pkey_alloc(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number330
uint32_t do_pkey_free(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number331
uint32_t do_statx(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number332
uint32_t do_io_pgetevents(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number333
uint32_t do_rseq(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number334
uint32_t do_pidfd_send_signal(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number424
uint32_t do_io_uring_setup(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number425
uint32_t do_io_uring_enter(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number426
uint32_t do_io_uring_register(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number427
uint32_t do_open_tree(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number428
uint32_t do_move_mount(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number429
uint32_t do_fsopen(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number430
uint32_t do_fsconfig(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number431
uint32_t do_fsmount(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number432
uint32_t do_fspick(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number433
uint32_t do_pidfd_open(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number434
uint32_t do_clone3(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number435
uint32_t do_close_range(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number436
uint32_t do_openat2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number437
uint32_t do_pidfd_getfd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number438
uint32_t do_faccessat2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number439
uint32_t do_process_madvise(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number440
uint32_t do_epoll_pwait2(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number441
uint32_t do_mount_setattr(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number442
uint32_t do_quotactl_fd(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number443
uint32_t do_landlock_create_ruleset(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number444
uint32_t do_landlock_add_rule(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number445
uint32_t do_landlock_restrict_self(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number446
uint32_t do_memfd_secret(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number447
uint32_t do_process_mrelease(uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6){} // Syscall number448

/*
void stat64_to_stat32(const struct stat *st, struct stat32 *st32) {
    st32->st_dev = st->st_dev;
    st32->st_ino = st->st_ino;
    st32->st_mode = st->st_mode;
    st32->st_nlink = st->st_nlink;
    st32->st_uid = st->st_uid;
    st32->st_gid = st->st_gid;
    st32->st_rdev = st->st_rdev;
    st32->st_size = st->st_size;
    st32->st_blksize = st->st_blksize;
    st32->st_blocks = st->st_blocks;

    //st32->st_atime = st64->st_atim;
    //st32->st_atime_nsec = st64->st_atim.tv_nsec;

    //st32->st_mtime = st64->st_mtim;
    //st32->st_mtime_nsec = st64->st_mtim.tv_nsec;

    //st32->st_ctime = st64->st_ctim;
    //st32->st_ctime_nsec = st64->st_ctim.tv_nsec;
}*/