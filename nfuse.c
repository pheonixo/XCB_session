#include "nfuse.h"

PhxNFuse *
ui_nfuse_create(PhxInterface *iface, PhxRectangle configure) {

    /* attaches to active_iface */
  PhxNexus *nexus = ui_nexus_create(iface, configure);
  if (nexus == NULL)  return NULL;
    /* redefine its purpose */
  nexus->type = PHX_NFUSE;
    /* default: XPD static. Code here only as example.
      Must always make sure not to affect other bits.
      ui_nexus_create() has this as default, so unnecessary code. */
  nexus->state &= ~(HXPD_MSK | VXPD_MSK);

  return (PhxNFuse*)nexus;
}
