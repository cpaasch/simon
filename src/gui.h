#ifndef _GUI_H
#define _GUI_H



#include <gtk/gtk.h>
#include <glibtop.h>
#include "utils.h"
#include "network.h"








void drawableInit(Drawable *graph, char *ifname, char *dispname);
void draw_backround(Drawable *graph);
gboolean load_graph_expose(GtkWidget *widget, GdkEventExpose *event,
		gpointer data_ptr);
gboolean
load_graph_update (gpointer user_data);
void quit();
GtkWidget *get_and_start_network_load_widget(Drawable *graph);


#endif /* _GUI_H */
