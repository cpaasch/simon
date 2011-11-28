#ifndef _MODEL_H
#define _MODEL_H

#include <gtk/gtk.h>
#include <glibtop.h>
#include <stdio.h>
#include <stdlib.h>



#define NUM_POINTS 62
#define TABLE_WIDTH 300
#define GRAPH_MIN_HEIGHT 100
#define GRAPH_MIN_WIDTH 350
#define LINE_WIDTH 1
#define IN_COLOR "#2C42FF"
#define OUT_COLOR "#BE001A"

#define FRAME_WIDTH 4
#define FRAMES_PER_UNIT 10
#define FONTSIZE 10.0
#define TABLE_FONT "Sans 10"
#define RMARGIN 3.5 * FONTSIZE
#define INDENT 32.0
#define NUM_BARS 4
#define SPEED 1000
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

#define MAX_IF_NAMELEN 100
#define MAX_IFS 50



typedef struct {
	GtkWidget *dispname;
	GtkWidget *net_in;
	GtkWidget *net_in_total;
	GtkWidget *net_out;
	GtkWidget *net_out_total;
} LoadGraphLabels;

typedef struct {

	GtkWidget *drawing_area;
	GdkDrawable *background;
	GtkWidget *hbox;
	GtkWidget *mainvbox;
	GdkGC *gc;

	int stop;	/* Set to 1 to kill this widget */

	LoadGraphLabels labels;

	GdkColor net_in_color, net_out_color;

	size_t cur;
	gfloat *data[NUM_POINTS];
	float data_block[124];
	guint graph_dely;
	guint real_draw_height;
	double graph_delx;
	guint graph_buffer_offset;
	guint draw_width, draw_height;
	guint render_counter;
	guint timer_index;

	struct {
		guint64 last_in, last_out;
		GTimeVal time;
		unsigned int max;
		unsigned values[62];
		size_t cur;
		char ifname[MAX_IF_NAMELEN];
		int status;
	} net;

} Drawable;

typedef struct {
	char ifname[MAX_IF_NAMELEN];
	Drawable data;
	GtkWidget *ifwidget;
	GtkWidget *select_widget;
} Interface;

typedef struct {
    GtkWidget *window;
    GtkWidget *dialog;
    Interface interface[MAX_IFS];
    int len;
} Interface_list;


void clear_background(Drawable *graph);

#endif /* _MODEL_H */
