#ifndef __SESS_CONFIGURE_H__
#define __SESS_CONFIGURE_H__

#include "session.h"
#include "nexus.h"
#include "draw.h"

/* private */
typedef struct _image_data {
 PhxNexus      *nexus;
 PhxRectangle  mete_box;        /* allocate box */
 PhxRectangle  draw_box;        /* need copy */
 uint16_t      state;           /* use as flags seperate from PhxObjects */
} IData_s;

typedef struct _config_image {
 IData_s *idata;
 uint16_t  icount;
} Image_s;

/* public */
extern void              _interface_configure(PhxInterface *iface,
                                              int16_t hD,
                                              int16_t vD);
extern void              ui_nexus_resize(PhxNexus *nexus,
                                         PhxRectangle *rD);

#endif /* __SESS_CONFIGURE_H__ */