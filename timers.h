#ifndef __SESS_TIMERS_H__
#define __SESS_TIMERS_H__

#include "windows.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#if (__STDC_VERSION__ <= 199901L)
 extern int clock_gettime(clockid_t clock_id, struct timespec *tp);
 #define CLOCK_MONOTONIC 4
#endif

 /* vvp = tvp + uvp */
#define	timeradd(tvp, uvp, vvp) \
do { \
(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
(vvp)->tv_nsec = (tvp)->tv_nsec + (uvp)->tv_nsec; \
if ((vvp)->tv_nsec >= 1000000000) { \
(vvp)->tv_sec++; \
(vvp)->tv_nsec -= 1000000000; \
} \
} while (0)
 /* vvp = tvp - uvp */
#define	timersub(tvp, uvp, vvp) \
do {								\
(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
(vvp)->tv_nsec = (tvp)->tv_nsec - (uvp)->tv_nsec; \
if ((vvp)->tv_nsec < 0) { \
(vvp)->tv_sec--; \
(vvp)->tv_nsec += 1000000000; \
} \
} while (0)

struct _sigtimer {
  const char       *id;
  PhxInterface     *iface;
  struct timespec  request;
  struct timespec  next_send;
  void             (*_sig_cb)(struct _sigtimer *);
  uint64_t         data;
  uint16_t         ref_count;
};
struct _timers {
  struct _sigtimer *timer;
  uint16_t         tcount;
};

extern struct _timers *timers;

extern int32_t          _process_timers(void);
extern struct _sigtimer *
                        ui_timer_create(PhxInterface *,
                                        const char *,
                                        struct timespec *,
                                        void (*handler)(struct _sigtimer *));
extern struct _sigtimer *
                        ui_timer_get(PhxInterface *, const char *);
extern void             ui_timer_delete(struct _sigtimer *);

#endif /* __SESS_TIMERS_H__ */
