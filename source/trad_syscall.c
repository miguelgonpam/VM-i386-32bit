#include "../lib/trad_syscall.h"
#include "../lib/syscall.h"
#include <stdint.h>

Syscall32bits syscalls[]= {
    do_restart_syscall, do_exit, do_fork, do_read, do_write, do_open, do_close, unimpl, do_creat, do_link, do_unlink, /*0-10*/
    do_execve, do_chdir, do_time, do_mknod, do_chmod, do_lchown, do_break, unimpl, unimpl, do_getpid, /*11-20*/
    do_mount, unimpl, do_setuid, do_getuid, unimpl, do_ptrace, do_alarm, unimpl, do_pause, do_utime, /*21-30*/
    unimpl, unimpl, do_access, unimpl, unimpl, do_sync, do_kill, do_rename, do_mkdir, do_rmdir, /*31-40*/
    do_dup, do_pipe, do_times, unimpl, do_brk, do_setgid, do_getgid, unimpl, do_geteuid, do_getegid, /*41-50*/
    do_acct, do_umount2, unimpl, do_ioctl, unimpl, unimpl, do_setpgid, unimpl, unimpl, do_umask, /*51-60*/
    do_chroot, do_ustat, do_dup2, do_getppid, do_getpgrp, do_setsid, do_rt_sigaction, unimpl, unimpl, do_setreuid, /* 61-70*/
    do_setregid, do_rt_sigsuspend, do_rt_sigpending, do_sethostname, do_setrlimit, do_getrlimit, do_getrusage, do_gettimeofday, do_settimeofday, do_getgroups, /*71-80*/
    do_setgroups, do_select, do_symlink, unimpl, do_readlink, do_uselib, do_swapon, do_reboot, unimpl, do_mmap, /*81-90*/
    do_munmap, do_truncate, do_ftruncate, do_fchmod, do_fchown, do_getpriority, do_setpriority, unimpl, do_statfs, do_fstatfs, /*91-100*/
    do_ioperm, unimpl, do_syslog, do_setitimer, do_getitimer, do_stat, do_lstat, do_fstat, unimpl, do_iopl, /*101-110*/
    do_vhangup, unimpl, unimpl, do_wait4, do_swapoff, do_sysinfo, unimpl, do_fsync, do_rt_sigreturn, do_clone, /*111-120*/
    do_setdomainname, do_uname, do_modify_ldt, do_adjtimex, do_mprotect, do_rt_sigprocmask, do_create_module, do_init_module, do_delete_module, do_get_kernel_syms, /*121-130*/
    do_quotactl, do_getpgid, do_fchdir, unimpl, do_sysfs, do_personality, do_afs_syscall, do_setfsuid, do_setfsgid, unimpl, /*131-140*/
    unimpl, unimpl, unimpl, do_msync, do_readv, do_writev, do_getsid, do_fdatasync, do__sysctl, do_mlock, /*141-150*/
    do_munlock, do_mlockall, do_munlockall, do_sched_setparam, do_sched_getparam, do_sched_setscheduler, do_sched_getscheduler, do_sched_yield, do_sched_get_priority_max, do_sched_get_priority_min, /*151-160*/
    do_sched_rr_get_interval, do_nanosleep, do_mremap, do_setresuid, do_getresuid, unimpl, do_query_module, do_poll, do_nfsservctl, do_setresgid, /* 161-170*/
    do_getresgid, do_prctl, do_rt_sigreturn, do_rt_sigaction, do_rt_sigprocmask, do_rt_sigpending, do_rt_sigtimedwait, do_rt_sigqueueinfo, do_rt_sigsuspend, unimpl, /*171-180*/
    unimpl, do_chown, do_getcwd, do_capget, do_capset, do_sigaltstack, do_sendfile, do_getpmsg, do_putpmsg, do_vfork, /* 181-190*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*191-200*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*201-210*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*211-220*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*221-230*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*231-240*/
    unimpl, unimpl, do_set_thread_area, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*241-250*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, do_set_tid_address, unimpl, unimpl, /*251-260*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*261-270*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*271-280*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*281-290*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*291-300*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*301-310*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*311-320*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*321-330*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*331-340*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*341-350*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*351-360*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*361-370*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*371-380*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*381-390*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*391-400*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*401-410*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*411-420*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*421-430*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*431-440*/
    unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, unimpl, /*441-450*/
    unimpl                                                                          /*451*/
    
}; 