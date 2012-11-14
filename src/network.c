#include "network.h"

#include <string.h>
/*
 * Calculates log2 of number.
 * Can't compile with std=c99 in maemo. Thus we don't have log2 from math library.
 */
double log_2(double n) {
	// log(n)/log(2) is log2.
	return log(n) / log(2);
}


/*
 * Scales the graph. If the graph needs to be scaled, redraw is forced.
 *
 * Original source from gnome-system-monitor.
 */

void net_scale(Drawable *g, unsigned din, unsigned dout) {

	int i = 0;
	g->data[0][0] = 1.0f * din / g->net.max;
	g->data[0][1] = 1.0f * dout / g->net.max;

	unsigned dmax = MAX(din, dout);
	g->net.values[g->net.cur] = dmax;
	g->net.cur = (g->net.cur + 1) % NUM_POINTS;

	unsigned new_max;
	// both way, new_max is the greatest value
	if (dmax >= g->net.max)
		new_max = dmax;
	else {
		int maxtemp = 0;
		int i = 0;
		for (i = 0; i < NUM_POINTS; i++)
			if (g->net.values[i] > maxtemp)
				maxtemp = g->net.values[i];
		new_max = maxtemp;
	}

	//
	// Round network maximum
	//

	const unsigned bak_max = new_max;

	/*if (ProcData::get_instance()->config.network_in_bits) {
	 // TODO: fix logic to give a nice scale with bits

	 // round up to get some extra space
	 // yes, it can overflow
	 new_max = 1.1 * new_max;
	 // make sure max is not 0 to avoid / 0
	 // default to 125 bytes == 1kbit
	 new_max = std::max(new_max, 125U);

	 } else {*/
	// round up to get some extra space
	// yes, it can overflow
	new_max = 1.1 * new_max;
	// make sure max is not 0 to avoid / 0
	// default to 1 KiB
	new_max = MAX(new_max, 1024U);

	// decompose new_max = coef10 * 2**(base10 * 10)
	// where coef10 and base10 are integers and coef10 < 2**10
	//
	// e.g: ceil(100.5 KiB) = 101 KiB = 101 * 2**(1 * 10)
	//      where base10 = 1, coef10 = 101, pow2 = 16

	unsigned pow2 = floor(log_2(new_max));
	unsigned base10 = pow2 / 10;
	unsigned coef10 = ceil(new_max / (double) (1UL << (base10 * 10)));
	g_assert(new_max <= (coef10 * (1UL << (base10 * 10))));

	// then decompose coef10 = x * 10**factor10
	// where factor10 is integer and x < 10
	// so we new_max has only 1 significant digit

	unsigned factor10 = pow(10.0, floor(log10(coef10)));
	coef10 = ceil(coef10 / (double) (factor10)) * factor10;

	// then make coef10 divisible by num_bars
	if (coef10 % NUM_BARS_Y != 0)
		coef10 = coef10 + (NUM_BARS_Y - coef10 % NUM_BARS_Y);
	g_assert(coef10 % NUM_BARS_Y == 0);

	new_max = coef10 * (1UL << (base10 * 10));
	//printf("bak %u new_max %u pow2 %u coef10 %u\n", bak_max, new_max, pow2,
	//		coef10);
	//}

	if (bak_max > new_max) {
		printf("overflow detected: bak=%u new=%u\n", bak_max, new_max);
		new_max = bak_max;
	}
//for (i = NUM_POINTS-1; i >= 0; i--) {
//	printf("%.2f ", g->data[i][0]);
//}
//printf("\n");
	// if max is the same or has decreased but not so much, don't
	// do anything to avoid rescaling
	if ((0.8 * g->net.max) < new_max && new_max <= g->net.max)
		return;

	const float scale = 1.0f * g->net.max / new_max;

	for (i = 0; i < NUM_POINTS; i++) {
		if (g->data[i][0] >= 0.0f) {
			g->data[i][0] *= scale;
			g->data[i][1] *= scale;
		}
	}

	//printf("rescale dmax = %u max = %u new_max = %u\n", dmax, g->net.max,
	//		new_max);

	g->net.max = new_max;

	// force the graph background to be redrawn now that scale has changed
	clear_background(g);
}

/*
 * Formats the network load from bits or bytes to Mbits and Gbits.
 *
 * Original source from gnome-system-monitor.
 */

gchar*
format_size(guint64 size, guint64 max_size, int want_bits) {

	enum {
		K_INDEX, M_INDEX, G_INDEX
	};

	typedef struct {
		guint64 factor;
		const char* string;
	} Format;

	const Format all_formats[2][3] = { { { 1UL << 10, "%.1f KiB" }, {
			1UL << 20, "%.1f MiB" }, { 1UL << 30, "%.1f GiB" } }, { { 1000,
			"%.1f kbit" }, { 1000000, "%.1f Mbit" },
			{ 1000000000, "%.1f Gbit" } } };

	//Format formats[3];
	const Format *formats = all_formats[want_bits ? 1 : 0];

	if (want_bits) {
		size *= 8;
		max_size *= 8;
	}

	if (max_size == 0)
		max_size = size;

	guint64 factor;
	const char* format = NULL;

	if (max_size < formats[M_INDEX].factor) {
		factor = formats[K_INDEX].factor;
		format = formats[K_INDEX].string;
	} else if (max_size < formats[G_INDEX].factor) {
		factor = formats[M_INDEX].factor;
		format = formats[M_INDEX].string;
	} else {
		factor = formats[G_INDEX].factor;
		format = formats[G_INDEX].string;
	}

	return g_strdup_printf(format, size / (double) factor);
}

/*
 * Helper function for format_size.
 *
 * Original source from gnome-system-monitor.
 */

char *format_rate(guint64 rate, guint64 max_rate, int want_bits) {
	char *bytes = format_size(rate, max_rate, want_bits);
	// xgettext: rate, 10MiB/s or 10Mbit/s
	char *formatted_rate = g_strdup_printf("%s/s", bytes);
	g_free(bytes);
	return formatted_rate;
}

/*
 * Helper function for format_size.
 *
 * Original source from gnome-system-monitor.
 */

char *format_network(guint64 rate, guint64 max_rate) {
	return format_size(rate, max_rate, NETWORK_IN_BITS);
}

/*
 * Helper function for format_rate.
 *
 * Original source from gnome-system-monitor.
 */

char *format_network_rate(guint64 rate, guint64 max_rate) {
	return format_rate(rate, max_rate, NETWORK_IN_BITS);
}


Interface_list get_active_iflist() {

	int i, a;
	glibtop_netlist netlist;
	char **ifnames;
	Interface_list ret;

	ifnames = glibtop_get_netlist(&netlist);

	for (i = 0, a = 0; i < netlist.number && i < MAX_IFS; i++) {
	    glibtop_netload netload;
	    glibtop_get_netload(&netload, ifnames[i]);

	    if (netload.if_flags & (1 << GLIBTOP_IF_FLAGS_LOOPBACK))
        continue;

      if (!((netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS6)) && netload.scope6
        != GLIBTOP_IF_IN6_SCOPE_LINK) && !(netload.flags & (1
        << GLIBTOP_NETLOAD_ADDRESS)))
        continue;

	    strncpy(ret.interface[a].ifname, ifnames[i], MAX_IF_NAMELEN);

	    a++;
  }

	ret.len = a;

	return ret;
}

Interface_list get_full_iflist() {

	int i;
	glibtop_netlist netlist;
	char **ifnames;
	Interface_list ret;

	ifnames = glibtop_get_netlist(&netlist);

	for (i = 0; i < netlist.number && i < MAX_IFS; i++) {
		glibtop_netload netload;
		glibtop_get_netload(&netload, ifnames[i]);

		strncpy(ret.interface[i].ifname, ifnames[i], MAX_IF_NAMELEN);

		//printf("Active if no %d:%s, sizeof:%d\n", a+1, ifnames[i], sizeof(ifnames[i]));
	}

	ret.len = i;

	return ret;
}


/*
 * A function that updates the current load info. Uses glibtop.
 *
 * Original source from gnome-system-monitor. Modified to show load for the
 * specified interface only.
 */

void update_net(Drawable *g) {
	glibtop_netlist netlist;
	char **ifnames;
	guint32 i;
	guint64 in = 0, out = 0;
	GTimeVal time;
	unsigned din, dout;

	ifnames = glibtop_get_netlist(&netlist);

	for (i = 0; i < netlist.number; i++) {
		glibtop_netload netload;
		glibtop_get_netload(&netload, ifnames[i]);

#ifdef ALL_TRAFFIC
		if (netload.if_flags & (1 << GLIBTOP_IF_FLAGS_LOOPBACK))
			continue;
#endif

		/* Skip interfaces without any IPv4/IPv6 address (or
		 those with only a LINK ipv6 addr) However we need to
		 be able to exclude these while still keeping the
		 value so when they get online (with NetworkManager
		 for example) we don't get a suddent peak.  Once we're
		 able to get this, ignoring down interfaces will be
		 possible too.  */
		//if (!(netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS6) && netload.scope6
		//		!= GLIBTOP_IF_IN6_SCOPE_LINK) && !(netload.flags & (1
		//		<< GLIBTOP_NETLOAD_ADDRESS)))
		//	continue;

		/* Don't skip interfaces that are down (GLIBTOP_IF_FLAGS_UP)
		 to avoid spikes when they are brought up */

		/* Modifying rate only for this graphs interface */

		if (strncmp(ifnames[i], g->net.ifname, MAX_IF_NAMELEN) == 0) {
#ifndef ALL_TRAFFIC
			in += netload.bytes_in;
			out += netload.bytes_out;
#endif
			if (netload.flags & (1 << GLIBTOP_IF_FLAGS_UP)) {
				update_if_status(1, g);		// wlan on
			}
			else {
				update_if_status(0, g);		// wlan off
			}
			break;
		}

#ifndef ALL_TRAFFIC
		if(strlen(g->net.ifname) == 0) {
#endif
			in += netload.bytes_in;
			out += netload.bytes_out;
#ifndef ALL_TRAFFIC
		}
#endif
	}

	if (i == netlist.number) { // Radio disabled.
		update_if_status(0, g);
	}

	g_strfreev(ifnames);

	g_get_current_time(&time);

	if (in >= g->net.last_in && out >= g->net.last_out && g->net.time.tv_sec
			!= 0) {
		float dtime;
		dtime = time.tv_sec - g->net.time.tv_sec + (float) (time.tv_usec
				- g->net.time.tv_usec) / G_USEC_PER_SEC;
		din = (in - g->net.last_in) / dtime;
		dout = (out - g->net.last_out) / dtime;
	} else {
		/* Don't calc anything if new data is less than old (interface
		 removed, counters reset, ...) or if it is the first time */
		din = 0;
		dout = 0;
	}

	g->net.last_in = in;
	g->net.last_out = out;
	g->net.time = time;

	net_scale(g, din, dout);

	gtk_label_set_text(GTK_LABEL (g->labels.net_in),
			format_network_rate(din, 0));
	gtk_label_set_text(GTK_LABEL (g->labels.net_in_total),
			format_network(in, 0));

	gtk_label_set_text(GTK_LABEL (g->labels.net_out), format_network_rate(dout,
			0));
	gtk_label_set_text(GTK_LABEL (g->labels.net_out_total), format_network(out,
			0));
}



/*
 * A function used to update the interface status information of the structure.
 *
 * Original source from gnome-system-monitor.
 */

void update_if_status(int status, Drawable *graph) {

	graph->net.status = status;

}


