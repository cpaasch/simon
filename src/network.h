#ifndef _NETWORK_H
#define _NETWORK_H

#include <glibtop/netload.h>
#include <glibtop/netlist.h>
#include <math.h>
#include "utils.h"

#define NETWORK_IN_BITS 1


double log_2( double n );
void net_scale(Drawable *g, unsigned din, unsigned dout);

gchar*
format_size(guint64 size, guint64 max_size, int want_bits);

char *format_rate(guint64 rate, guint64 max_rate, int want_bits);
char *format_network(guint64 rate, guint64 max_rate);
char *format_network_rate(guint64 rate, guint64 max_rate);

Interface_list get_active_iflist();
Interface_list get_full_iflist();
void update_net(Drawable *g);

void update_if_status(int status, Drawable *graph);

#endif /* _NETWORK_H */
