#ifndef __SESS_EVENTS_DEBUG_H__
#define __SESS_EVENTS_DEBUG_H__

#define DEBUG_EVENTS(a)     (void)(a)
#define DEBUG_ASSERT(a, b)  (void)(a), (void)(b)

#if DEBUG_EVENTS_ON
 extern uint64_t debug_flags;
 extern void  _debug_assert(bool test, const char *message);
 #if (DEBUG_MINIMUM != 1)
  #undef DEBUG_EVENTS
  #undef DEBUG_ASSERT
  extern void _debug_event(xcb_generic_event_t *nvt, const char *caller);
  #define DEBUG_EVENTS(a)    _debug_event(nvt, (a))
  #define DEBUG_ASSERT(a, b) _debug_assert((a), (b))
 #endif
#endif


#if 0
/*#define DEBUG_BUTTON(a,b) \
  printf("       %d, SBIT_%s.\n", (a), (b)) */
#define DEBUG_BUTTON(a,b) (void)(a), (void)(b)
#endif

#endif /* __SESS_EVENTS_DEBUG_H__ */
