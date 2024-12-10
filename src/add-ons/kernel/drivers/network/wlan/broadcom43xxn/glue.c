/*
 * Copyright 2009, Colin Günther, coling@gmx.de.
 * All Rights Reserved. Distributed under the terms of the MIT License.
 */


#include <sys/bus.h>
#include <sys/kernel.h>

#include <machine/bus.h>

#include <net/if.h>
#include <net/if_media.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_amrr.h>

#include <dev/bwn/if_bwnreg.h>
#include <dev/bwn/if_bwnvar.h>


HAIKU_FBSD_WLAN_DRIVER_GLUE(broadcom43xxn, bwn, pci)
NO_HAIKU_FBSD_MII_DRIVER();
NO_HAIKU_REENABLE_INTERRUPTS();
HAIKU_DRIVER_REQUIREMENTS(FBSD_WLAN);
HAIKU_FIRMWARE_VERSION(0);
NO_HAIKU_FIRMWARE_NAME_MAP();


int
HAIKU_CHECK_DISABLE_INTERRUPTS(device_t dev)
{
	struct bwn_softc* sc = (struct bwn_softc*)device_get_softc(dev);
	struct bwn_mac* mac = sc->sc_curmac;
	uint32 reason;

	if ((sc->sc_flags & BWN_FLAG_RUNNING)) {
		return 0;
	}

	if (mac->mac_status < BWN_MAC_STATUS_STARTED ||
	    (sc->sc_flags & BWN_FLAG_INVALID)) {
		return 0;
	}

	reason = BWN_READ_4(mac, BWN_INTR_REASON);
	if (reason == 0xffffffff) {
		// Not for us
		return 0;
	}

	reason &= mac->mac_intr_mask;
	if (reason == 0) {
		// nothing interesting
		return 0;
	}
	mac->mac_reason[0] = BWN_READ_4(mac, BWN_DMA0_REASON) & 0x0001dc00;
	mac->mac_reason[1] = BWN_READ_4(mac, BWN_DMA1_REASON) & 0x0000dc00;
	mac->mac_reason[2] = BWN_READ_4(mac, BWN_DMA2_REASON) & 0x0000dc00;
	mac->mac_reason[3] = BWN_READ_4(mac, BWN_DMA3_REASON) & 0x0001dc00;
	mac->mac_reason[4] = BWN_READ_4(mac, BWN_DMA4_REASON) & 0x0000dc00;
	BWN_WRITE_4(mac, BWN_INTR_REASON, reason);
	BWN_WRITE_4(mac, BWN_DMA0_REASON, mac->mac_reason[0]);
	BWN_WRITE_4(mac, BWN_DMA1_REASON, mac->mac_reason[1]);
	BWN_WRITE_4(mac, BWN_DMA2_REASON, mac->mac_reason[2]);
	BWN_WRITE_4(mac, BWN_DMA3_REASON, mac->mac_reason[3]);
	BWN_WRITE_4(mac, BWN_DMA4_REASON, mac->mac_reason[4]);

	/* Disable interrupts. */
	BWN_WRITE_4(mac, BWN_INTR_MASK, 0);

	mac->mac_reason_intr = reason;

	return 1;
}
