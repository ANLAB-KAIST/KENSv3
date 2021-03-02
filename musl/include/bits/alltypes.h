#include <limits.h>
#include <stdint.h>

#define _Addr uintptr_t
#define _Int64 int64_t

#define __BYTE_ORDER 1234
#define __LONG_MAX LONG_MAX

#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2
#if !defined(__DEFINED_float_t)
typedef long double float_t;
#define __DEFINED_float_t
#endif

#if !defined(__DEFINED_double_t)
typedef long double double_t;
#define __DEFINED_double_t
#endif

#else
#if !defined(__DEFINED_float_t)
typedef float float_t;
#define __DEFINED_float_t
#endif

#if !defined(__DEFINED_double_t)
typedef double double_t;
#define __DEFINED_double_t
#endif

#endif

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __USE_TIME_BITS64 1

#if !defined(__DEFINED_ssize_t)
typedef _Addr ssize_t;
#define __DEFINED_ssize_t
#endif

#if !defined(__DEFINED_regoff_t)
typedef _Addr regoff_t;
#define __DEFINED_regoff_t
#endif

#if !defined(__DEFINED_time_t)
typedef _Int64 time_t;
#define __DEFINED_time_t
#endif

#if !defined(__DEFINED_suseconds_t)
typedef _Int64 suseconds_t;
#define __DEFINED_suseconds_t
#endif

#if !defined(__DEFINED_u_int64_t)
typedef uint64_t u_int64_t;
#define __DEFINED_u_int64_t
#endif

#if !defined(__DEFINED_uintmax_t)
typedef uint64_t uintmax_t;
#define __DEFINED_uintmax_t
#endif

#if !defined(__DEFINED_mode_t)
typedef unsigned mode_t;
#define __DEFINED_mode_t
#endif

#if !defined(__DEFINED_nlink_t)
typedef uintptr_t nlink_t;
#define __DEFINED_nlink_t
#endif

#if !defined(__DEFINED_blksize_t)
typedef long blksize_t;
#define __DEFINED_blksize_t
#endif

#if !defined(__DEFINED_blkcnt_t)
typedef _Int64 blkcnt_t;
#define __DEFINED_blkcnt_t
#endif

#if !defined(__DEFINED_fsblkcnt_t)
typedef uint64_t fsblkcnt_t;
#define __DEFINED_fsblkcnt_t
#endif

#if !defined(__DEFINED_fsfilcnt_t)
typedef uint64_t fsfilcnt_t;
#define __DEFINED_fsfilcnt_t
#endif

#if !defined(__DEFINED_timer_t)
typedef void *timer_t;
#define __DEFINED_timer_t
#endif

#if !defined(__DEFINED_clockid_t)
typedef int clockid_t;
#define __DEFINED_clockid_t
#endif

#if !defined(__DEFINED_clock_t)
typedef long clock_t;
#define __DEFINED_clock_t
#endif

#if !defined(__DEFINED_struct_timeval)
struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};
#define __DEFINED_struct_timeval
#endif

#if !defined(__DEFINED_pid_t)
typedef int pid_t;
#define __DEFINED_pid_t
#endif

#if !defined(__DEFINED_id_t)
typedef unsigned id_t;
#define __DEFINED_id_t
#endif

#if !defined(__DEFINED_uid_t)
typedef unsigned uid_t;
#define __DEFINED_uid_t
#endif

#if !defined(__DEFINED_gid_t)
typedef unsigned gid_t;
#define __DEFINED_gid_t
#endif

#if !defined(__DEFINED_key_t)
typedef int key_t;
#define __DEFINED_key_t
#endif

#if !defined(__DEFINED_useconds_t)
typedef unsigned useconds_t;
#define __DEFINED_useconds_t
#endif

#ifdef __cplusplus
#if !defined(__DEFINED_pthread_t)
typedef unsigned long pthread_t;
#define __DEFINED_pthread_t
#endif

#else
#if !defined(__DEFINED_pthread_t)
typedef struct __pthread *pthread_t;
#define __DEFINED_pthread_t
#endif

#endif
#if !defined(__DEFINED_pthread_once_t)
typedef int pthread_once_t;
#define __DEFINED_pthread_once_t
#endif

#if !defined(__DEFINED_pthread_key_t)
typedef unsigned pthread_key_t;
#define __DEFINED_pthread_key_t
#endif

#if !defined(__DEFINED_pthread_spinlock_t)
typedef int pthread_spinlock_t;
#define __DEFINED_pthread_spinlock_t
#endif

#if !defined(__DEFINED_pthread_mutexattr_t)
typedef struct {
  unsigned __attr;
} pthread_mutexattr_t;
#define __DEFINED_pthread_mutexattr_t
#endif

#if !defined(__DEFINED_pthread_condattr_t)
typedef struct {
  unsigned __attr;
} pthread_condattr_t;
#define __DEFINED_pthread_condattr_t
#endif

#if !defined(__DEFINED_pthread_barrierattr_t)
typedef struct {
  unsigned __attr;
} pthread_barrierattr_t;
#define __DEFINED_pthread_barrierattr_t
#endif

#if !defined(__DEFINED_pthread_rwlockattr_t)
typedef struct {
  unsigned __attr[2];
} pthread_rwlockattr_t;
#define __DEFINED_pthread_rwlockattr_t
#endif

#if !defined(__DEFINED_struct__IO_FILE)
struct _IO_FILE {
  char __x;
};
#define __DEFINED_struct__IO_FILE
#endif

#if !defined(__DEFINED_locale_t)
typedef struct __locale_struct *locale_t;
#define __DEFINED_locale_t
#endif

#if !defined(__DEFINED_sigset_t)
typedef struct __sigset_t {
  unsigned long __bits[128 / sizeof(long)];
} sigset_t;
#define __DEFINED_sigset_t
#endif

#if !defined(__DEFINED_struct_iovec)
struct iovec {
  void *iov_base;
  size_t iov_len;
};
#define __DEFINED_struct_iovec
#endif

#if !defined(__DEFINED_struct_winsize)
struct winsize {
  unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel;
};
#define __DEFINED_struct_winsize
#endif

#if !defined(__DEFINED_socklen_t)
typedef unsigned socklen_t;
#define __DEFINED_socklen_t
#endif

#if !defined(__DEFINED_sa_family_t)
typedef unsigned short sa_family_t;
#define __DEFINED_sa_family_t
#endif

#if !defined(__DEFINED_pthread_attr_t)
typedef struct {
  union {
    int __i[sizeof(long) == 8 ? 14 : 9];
    volatile int __vi[sizeof(long) == 8 ? 14 : 9];
    unsigned long __s[sizeof(long) == 8 ? 7 : 9];
  } __u;
} pthread_attr_t;
#define __DEFINED_pthread_attr_t
#endif

#if !defined(__DEFINED_pthread_mutex_t)
typedef struct {
  union {
    int __i[sizeof(long) == 8 ? 10 : 6];
    volatile int __vi[sizeof(long) == 8 ? 10 : 6];
    volatile void *volatile __p[sizeof(long) == 8 ? 5 : 6];
  } __u;
} pthread_mutex_t;
#define __DEFINED_pthread_mutex_t
#endif

#if !defined(__DEFINED_mtx_t)
typedef struct {
  union {
    int __i[sizeof(long) == 8 ? 10 : 6];
    volatile int __vi[sizeof(long) == 8 ? 10 : 6];
    volatile void *volatile __p[sizeof(long) == 8 ? 5 : 6];
  } __u;
} mtx_t;
#define __DEFINED_mtx_t
#endif

#if !defined(__DEFINED_pthread_cond_t)
typedef struct {
  union {
    int __i[12];
    volatile int __vi[12];
    void *__p[12 * sizeof(int) / sizeof(void *)];
  } __u;
} pthread_cond_t;
#define __DEFINED_pthread_cond_t
#endif

#if !defined(__DEFINED_cnd_t)
typedef struct {
  union {
    int __i[12];
    volatile int __vi[12];
    void *__p[12 * sizeof(int) / sizeof(void *)];
  } __u;
} cnd_t;
#define __DEFINED_cnd_t
#endif

#if !defined(__DEFINED_pthread_rwlock_t)
typedef struct {
  union {
    int __i[sizeof(long) == 8 ? 14 : 8];
    volatile int __vi[sizeof(long) == 8 ? 14 : 8];
    void *__p[sizeof(long) == 8 ? 7 : 8];
  } __u;
} pthread_rwlock_t;
#define __DEFINED_pthread_rwlock_t
#endif

#if !defined(__DEFINED_pthread_barrier_t)
typedef struct {
  union {
    int __i[sizeof(long) == 8 ? 8 : 5];
    volatile int __vi[sizeof(long) == 8 ? 8 : 5];
    void *__p[sizeof(long) == 8 ? 4 : 5];
  } __u;
} pthread_barrier_t;
#define __DEFINED_pthread_barrier_t
#endif

#undef _Addr
#undef _Int64

#define weak_alias(A, B)
