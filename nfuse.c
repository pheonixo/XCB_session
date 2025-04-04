#include "nfuse.h"

PhxNFuse *
ui_nfuse_create(PhxInterface *iface, PhxRectangle configure) {

    /* attaches to active_iface */
  PhxNexus *nexus = ui_nexus_create(iface, configure);
  if (nexus == NULL)  return NULL;
    /* redefine its purpose */
  nexus->type = PHX_NFUSE;
    /* nfuse is constant width/height. */
  nexus->state |= HXPD_LFT | VXPD_TOP;

  return (PhxNFuse*)nexus;
}
