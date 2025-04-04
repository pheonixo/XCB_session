#include "timers.h"

 /* global */
struct _timers *timers = NULL;

struct _sigtimer *
ui_timer_create(PhxInterface *iface, const char *tid,
                struct timespec *ts,
                void (*handler)(struct _sigtimer *)) {

  struct _sigtimer *tmr;
  uint16_t tdx;

  if (!IS_WINDOW_TYPE(iface)) {
    fprintf(stderr, "Could not create timer on iface.\n");
    return NULL;
  }

  if (timers == NULL) {
    size_t aSz;
    timers = malloc(sizeof(struct _timers));
    if (timers == NULL)  return NULL;
    aSz = OBJS_ALLOC * sizeof(struct _sigtimer);
    timers->timer = malloc(aSz);
    if (timers->timer == NULL) {
      free(timers);
      timers = NULL;
      return NULL;
    }
    memset(timers->timer, 0, aSz);
    timers->tcount = 0;
  }

  tdx = 0;
  do {
    tmr = &timers->timer[tdx];
    if (tmr->id == NULL)  continue;
    if (strcmp(tmr->id, tid) == 0) {
      if (iface == tmr->iface) {
        tmr->ref_count++;
        return tmr;
      }
    }
  } while ((++tdx) < timers->tcount);

  if (_obj_alloc_test((void**)&timers,
                        sizeof(struct _timers), timers->tcount))
    return NULL;

  tdx = 0;
  do {
    tmr = &timers->timer[tdx];
    if (tmr->id == NULL) {
      struct timespec now;
      tmr->id = strdup(tid);
      if (!IS_WINDOW_TYPE(iface))
        do  iface = iface->i_mount;
        while (!IS_WINDOW_TYPE(iface));
      tmr->iface = iface;
      tmr->request = *ts;
      clock_gettime(CLOCK_MONOTONIC, &now);
      timeradd(&now, ts, &tmr->next_send);
      tmr->_sig_cb = handler;
      tmr->ref_count++;
      timers->tcount++;
      return tmr;
    }
  } while ((++tdx) <= timers->tcount);
  return tmr;
}

struct _sigtimer *
ui_timer_get(PhxInterface *iface, const char *tid) {

  uint16_t tdx;

  if ( (timers == NULL) || (timers->tcount == 0) )  return NULL;

  tdx = 0;
  do {
    struct _sigtimer *tmr;
    tmr = &timers->timer[tdx];
    if (tmr->id == NULL)  continue;
    if ( (strcmp(tmr->id, tid) == 0)
        && (iface == tmr->iface) )
      return tmr;
  } while ((++tdx) < timers->tcount);
  return NULL;
}

/* private function, part of xcb_main() event loop */
int32_t
_process_timers(void) {

  struct timespec now, elapsed, ptts;
  uint16_t tdx, idx;
  int32_t timeout = INT_MAX;
  if ( (timers == NULL) || (timers->tcount == 0) )  return timeout;

  tdx = 0, idx = 0;
    /* time started processing timers */
  clock_gettime(CLOCK_MONOTONIC, &ptts);
  do {
    struct _sigtimer *tmr = &timers->timer[idx];  idx++;
    if (tmr->id != NULL) {
      int32_t tout;
      tdx++;
      clock_gettime(CLOCK_MONOTONIC, &now);
      timersub(&tmr->next_send, &now, &elapsed);
      if (elapsed.tv_sec < 0) {
        tmr->_sig_cb(tmr); /* may delete timer */
        if (tmr->id == NULL)  continue;
        timeradd(&tmr->next_send, &tmr->request, &tmr->next_send);
        timersub(&tmr->next_send, &now, &elapsed);
      }
if (elapsed.tv_sec != 0)
puts("here timers.c:122");
      tout = elapsed.tv_nsec / 1000000;
      timeout = minof(timeout, tout);
    }
  } while (tdx < timers->tcount);

  if (timeout != INT_MAX) {
    clock_gettime(CLOCK_MONOTONIC, &now);
    timersub(&now, &ptts, &elapsed);
    if ((elapsed.tv_nsec + 500000) > 1000000) {
      timeout -= (elapsed.tv_nsec + 500000) / 1000000;
      if (timeout < 0)  timeout = 1;
    }
  }
  return timeout;
}

void
ui_timer_delete(struct _sigtimer *tmr) {

  uint16_t tdx, idx;
  if (timers->tcount == 0) {
    fprintf(stderr, "Could not delete timer\n");
    return;
  }
  tdx = 0, idx = 0;
  do {
    struct _sigtimer *inspect = &timers->timer[idx];  idx++;
    if (inspect != tmr) continue;
    if (tmr->ref_count != 0) {
      if ((--tmr->ref_count) == 0) {
        free((void*)tmr->id);
        tmr->iface = NULL;
        tmr->id = NULL;
        --timers->tcount;
      }
    }
    break;
  } while (tdx < timers->tcount);
}

