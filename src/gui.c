#include "gui.h"

static void close_simon( GtkWidget *widget,
                   gpointer   data )
{
	Drawable *graph = (Drawable *) data;

	graph->stop = 1;
}

/*
 * This method initializes a Drawable structure. The drawable structure is
 * defined in common.h.
 *
 * Parameters:
 * graph: the graph to initialize
 * ifname: the name of the interface in the system.
 * dispname: the name of the interface to display next to the graph.
 */

void drawableInit(Drawable *graph, char *ifname, char *dispname)
{
	int i;

	graph->gc = NULL;

	graph->stop = 0;

	/* Initializin some :/ values... */
	memset(&graph->net, 0, sizeof(graph->net));
	graph->net.max = 1;
	graph->net.time.tv_sec = 0;
	graph->net.time.tv_usec = 0;

	graph->labels.net_in = gtk_label_new(NULL);
	graph->labels.net_in_total = gtk_label_new(NULL);
	graph->labels.net_out = gtk_label_new(NULL);
	graph->labels.net_out_total = gtk_label_new(NULL);
	graph->labels.dispname = gtk_label_new(dispname);
	gtk_label_set_angle(GTK_LABEL (graph->labels.dispname), 90);
	gtk_widget_modify_font (graph->labels.net_in,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_widget_modify_font (graph->labels.net_in_total,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_widget_modify_font (graph->labels.net_out,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_widget_modify_font (graph->labels.net_out_total,
	pango_font_description_from_string (TABLE_FONT)
	);


	graph->mainvbox = gtk_vbox_new (FALSE, 6);

	graph->hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_widget_set_size_request(graph->hbox, -1, GRAPH_MIN_HEIGHT);

	gdk_color_parse(IN_COLOR, &graph->net_in_color);
	gdk_color_parse(OUT_COLOR, &graph->net_out_color);

	graph->render_counter = 0;
	graph->timer_index = 0;

	for (i = 0; i < NUM_POINTS*2; i++)
		graph->data_block[i] = -1.0f;

	for (i = 0; i < NUM_POINTS; ++i)
		graph->data[i] = &graph->data_block[0] + i * 2;

	memset(graph->net.ifname, '\0', MAX_IF_NAMELEN);

	if(ifname != NULL)
		strncpy(graph->net.ifname, ifname, MAX_IF_NAMELEN);

	/* Create the drawing area */

	graph->drawing_area = gtk_drawing_area_new();
	//gtk_widget_set_size_request(GTK_WIDGET(graph->drawing_area), GRAPH_MIN_WIDTH, -1);
	gtk_box_pack_start(GTK_BOX(graph->hbox), graph->labels.dispname, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(graph->hbox), graph->drawing_area, TRUE, TRUE, 0);

	graph->background = NULL;

	/* Showing all */

	gtk_widget_show(graph->drawing_area);
}

/*
 * This method draws the backround of the graph.
 *
 * Original source from gnome-system-monitor.
 */

void draw_backround(Drawable *graph)
{

	int i = 0;
	double dash[2] = { 1.0, 2.0 };
	cairo_text_extents_t extents;
	char *caption;


//	Drawable *graph = (Drawable *)data;
	if(graph->gc == NULL) {
		graph->gc = gdk_gc_new (GDK_DRAWABLE (graph->drawing_area->window));
	}

	/* Playing with cairo */
	graph->draw_width = graph->drawing_area->allocation.width - 2 * FRAME_WIDTH;
	graph->draw_height = graph->drawing_area->allocation.height - 2 * FRAME_WIDTH;
	graph->graph_dely = (graph->draw_height - 15) / NUM_BARS; /* round to int to avoid AA blur */
	graph->real_draw_height = graph->graph_dely * NUM_BARS;
	graph->graph_delx = (graph->draw_width - 2.0 - RMARGIN - INDENT) / (NUM_POINTS - 3);
	graph->graph_buffer_offset = (int) (1.5 * graph->graph_delx) + FRAME_WIDTH;

	graph->background = gdk_pixmap_new(GDK_DRAWABLE(graph->drawing_area->window),
			graph->drawing_area->allocation.width, graph->drawing_area->allocation.height, -1);

	//gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);
	cairo_t *cr = gdk_cairo_create(graph->background);

	GtkStyle *style = gtk_widget_get_style(graph->drawing_area);
	gdk_cairo_set_source_color(cr, &style->bg[GTK_STATE_NORMAL]);
	cairo_paint(cr);

	/* draw frame */
	cairo_translate(cr, FRAME_WIDTH, FRAME_WIDTH);

	/* Draw background rectangle */
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_rectangle (cr, RMARGIN + INDENT, 0,
			graph->draw_width - RMARGIN - INDENT, graph->real_draw_height);

	cairo_fill(cr);

	cairo_set_line_width (cr, 1.0);
	cairo_set_dash (cr, dash, 2, 0);
	cairo_set_font_size (cr, FONTSIZE);

	for (i = 0; i <= NUM_BARS; ++i) {
		double y;

		if (i == 0)
			y = 0.5 + FONTSIZE / 2.0;
		else if (i == NUM_BARS)
			y = i * graph->graph_dely + 0.5;
		else
			y = i * graph->graph_dely + FONTSIZE / 2.0;

		gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_NORMAL]);
		// operation orders matters so it's 0 if i == num_bars
		unsigned rate = graph->net.max - (i * graph->net.max / NUM_BARS);
		const char *caption = format_network_rate(rate, graph->net.max);
		cairo_text_extents (cr, caption, &extents);
		cairo_move_to (cr, INDENT - extents.width + 20, y);
		cairo_show_text (cr, caption);


		cairo_set_source_rgba (cr, 0, 0, 0, 0.75);
		cairo_move_to (cr, RMARGIN + INDENT - 3, i * graph->graph_dely + 0.5);
		cairo_line_to (cr, graph->draw_width - 0.5, i * graph->graph_dely + 0.5);
	}
	cairo_stroke (cr);

	cairo_set_dash(cr, dash, 2, 1.5);

	const unsigned total_seconds = SPEED * (NUM_POINTS - 2) / SPEED;

	for (i = 0; i < 7; i++) {
		double x = (i) * (graph->draw_width - RMARGIN - INDENT) / 6;
		cairo_set_source_rgba(cr, 0, 0, 0, 0.75);
		cairo_move_to(cr, (ceil(x) + 0.5) + RMARGIN + INDENT, 0.5);
		cairo_line_to(cr, (ceil(x) + 0.5) + RMARGIN + INDENT,
				graph->real_draw_height + 4.5);
		cairo_stroke(cr);
		unsigned seconds = total_seconds - i * total_seconds / 6;
		const char* format;
		//if (i == 0)
		//	format = dngettext(GETTEXT_PACKAGE, "%u second", "%u seconds", seconds);
		//else
		format = "%u";
		caption = g_strdup_printf(format, seconds);
		cairo_text_extents(cr, caption, &extents);
		cairo_move_to(cr, ((ceil(x) + 0.5) + RMARGIN + INDENT)
				- (extents.width / 2), graph->draw_height);
		gdk_cairo_set_source_color(cr, &style->fg[GTK_STATE_NORMAL]);
		cairo_show_text(cr, caption);
		g_free(caption);
	}

	cairo_stroke(cr);



	cairo_destroy(cr);



//	gtk_widget_queue_draw (graph->drawing_area);

}

/*
 * A function that draws the graph of the network load.
 *
 * Original source from gnome-system-monitor.
 */

gboolean load_graph_expose(GtkWidget *widget, GdkEventExpose *event,
		gpointer data_ptr) {
	Drawable * const g = (Drawable *) data_ptr;

	guint i, j;
	gdouble sample_width, x_offset;

	//clear_background(g);
	if (g->background == NULL) {
		draw_backround(g);
	}
	gdk_draw_drawable(g->drawing_area->window, g->gc, g->background, 0, 0, 0, 0,
			g->drawing_area->allocation.width, g->drawing_area->allocation.height);

	/* Number of pixels wide for one graph point */
	sample_width = (float) (g->draw_width - RMARGIN - INDENT)
			/ (float) NUM_POINTS;
	/* General offset */
	x_offset = g->draw_width - RMARGIN + (sample_width * 2);

	/* Subframe offset */
	x_offset += RMARGIN - ((sample_width / FRAMES_PER_UNIT)
			* g->render_counter);

	/* draw the graph */
	cairo_t* cr;

	cr = gdk_cairo_create(g->drawing_area->window);

	cairo_set_line_width(cr, LINE_WIDTH);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_rectangle(cr, RMARGIN + INDENT + FRAME_WIDTH + 1, FRAME_WIDTH
			- 1, g->draw_width - RMARGIN - INDENT - 1,
			g->real_draw_height + FRAME_WIDTH - 1);
	cairo_clip(cr);

	for (j = 0; j < 2; ++j) {
		cairo_move_to(cr, x_offset, (1.0f - g->data[0][j])
				* g->real_draw_height);
		if(j == 0)
			gdk_cairo_set_source_color(cr, &(g->net_in_color));
		else
			gdk_cairo_set_source_color(cr, &(g->net_out_color));

		for (i = 1; i < NUM_POINTS; ++i) {

			if (g->data[i][j] == -1.0f)
				continue;
			if(j == 0) {
							//printf("graph_delx %G, data[%d][0] %G, real_draw_height %d\n", g->graph_delx, i-1, g->data[i - 1][0], g->real_draw_height);
						}
			cairo_curve_to(cr, x_offset - ((i - 0.5f) * g->graph_delx), (1.0f
					- g->data[i - 1][j]) * g->real_draw_height + 3.5f, x_offset
					- ((i - 0.5f) * g->graph_delx), (1.0f - g->data[i][j])
					* g->real_draw_height + 3.5f, x_offset
					- (i * g->graph_delx), (1.0f - g->data[i][j])
					* g->real_draw_height + 3.5f);
		}
		cairo_stroke(cr);

	}

	cairo_destroy(cr);

	return TRUE;
}

/* Redraws the backing buffer for the load graph and updates the window */
void
load_graph_draw (Drawable *g)
{
	/* repaint */
	gtk_widget_queue_draw (g->drawing_area);
}

/*
 * Updates the load graph when the timeout expires.
 *
 * Original source from gnome-system-monitor.
 */

gboolean
load_graph_update (gpointer user_data)
{
	Drawable * const g = (Drawable *)user_data;

	/* Stop flag set > The widget will be destroyed ad the timer canceled */
	if(g->stop == 1) {
		gtk_widget_destroy(g->mainvbox);
		return FALSE;
	}

	if (g->render_counter == FRAMES_PER_UNIT - 1) {
		int i;
		for (i = NUM_POINTS -1; i > 0; i--) {
			g->data[i][0] = g->data[i-1][0];
			g->data[i][1] = g->data[i-1][1];
		}

		update_net(g);
	}

	load_graph_draw(g);

	g->render_counter++;

	if (g->render_counter >= FRAMES_PER_UNIT)
		g->render_counter = 0;

	return TRUE;
}

/*
 * Updates the size of the graph.
 *
 * Original source from gnome-system-monitor.
 */

static gboolean
load_graph_configure (GtkWidget *widget,
		      GdkEventConfigure *event,
		      gpointer data_ptr)
{
/*	Drawable * const g = (Drawable *)(data_ptr);

	//GtkAllocation allocation;
	//gtk_widget_get_allocation (graph->hbox, &allocation);
	//gtk_widget_set_size_request(GTK_WIDGET(graph->drawing_area), allocation.width - 20, -1);


	g->draw_width = widget->allocation.width - 2 * FRAME_WIDTH;
	g->draw_height = widget->allocation.height - 2 * FRAME_WIDTH;

	clear_background(g);

	if (g->gc == NULL) {
		g->gc = gdk_gc_new (GDK_DRAWABLE (widget->window));
	}

	load_graph_draw (g);
*/
	return TRUE;
}

/*
 * Creates the widget with the graph, header and the net legend box.
 *
 * Original source from gnome-system-monitor.
 */

GtkWidget *get_and_start_network_load_widget(Drawable *graph) {
	//GtkWidget *button;
	GtkWidget *table;
	GtkWidget *net_legend_box;
	GtkWidget *spacer, *label;
	GtkWidget *closebutton;
	GtkAlignment *halign;
	char buf[100];
	GdkColor color;

	gtk_box_pack_start(GTK_BOX(graph->mainvbox), graph->hbox, TRUE, TRUE, 0);
	gtk_widget_show(graph->hbox);

	/* Net legend */
	net_legend_box = gtk_hbox_new(FALSE, 10);
	halign = (GtkAlignment *)gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding (halign,
			 0,
			 0,
			 80,
			 0);
	gtk_container_add(GTK_CONTAINER(halign), net_legend_box);


	gtk_box_pack_start (GTK_BOX (graph->mainvbox), (GtkWidget *)halign,
				FALSE, FALSE, 0);

	//spacer = gtk_label_new ("");

	//if(show_in_out_color_label)
	//	sprintf(buf, "<span color=\"%s\">in</span> / <span color=\"%s\">out</span>", IN_COLOR, OUT_COLOR);
	//		gtk_label_set_markup (GTK_LABEL (spacer), buf);
	//gtk_box_pack_start(GTK_BOX(net_legend_box), spacer, FALSE, FALSE, 0);


	table = gtk_table_new (2, 4, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_box_pack_start (GTK_BOX (net_legend_box), table,
				TRUE, TRUE, 0);

	label = gtk_label_new ("Receiving");
	gtk_widget_modify_font (label,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	gtk_misc_set_alignment (GTK_MISC (graph->labels.net_in),
				1.0,
				0.5);

	gtk_widget_set_size_request(GTK_WIDGET(graph->labels.net_in), 65, -1);
	gtk_table_attach (GTK_TABLE (table), graph->labels.net_in, 2, 3, 0, 1,
			(GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);

	label = gtk_label_new ("Total Received");
	gtk_widget_modify_font (label,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	gtk_misc_set_alignment (GTK_MISC (graph->labels.net_in_total),
				1.0,
				0.5);
	gtk_table_attach (GTK_TABLE (table),
			graph->labels.net_in_total,
			  2,
			  3,
			  1,
			  2,
			  GTK_FILL,
			  GTK_FILL,
			  0,
			  0);


	spacer = gtk_label_new ("");
	gtk_widget_modify_font (spacer,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_widget_set_size_request(GTK_WIDGET(spacer), 38, -1);
	gtk_table_attach (GTK_TABLE (table), spacer, 3, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_box_pack_start (GTK_BOX (net_legend_box), table,
				TRUE, TRUE, 0);

	label = gtk_label_new ("Sending");
	gtk_widget_modify_font (label,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	gtk_misc_set_alignment (GTK_MISC (graph->labels.net_out),
				1.0,
				0.5);
	gtk_widget_set_size_request(GTK_WIDGET(graph->labels.net_out), 65, -1);
	gtk_table_attach (GTK_TABLE (table), graph->labels.net_out, 2, 3, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);

	label = gtk_label_new ("Total Sent");
	gtk_widget_modify_font (label,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	gtk_misc_set_alignment (GTK_MISC (graph->labels.net_out_total),
				1.0,
				0.5);
	gtk_table_attach (GTK_TABLE (table),
			graph->labels.net_out_total,
			  2,
			  3,
			  1,
			  2,
			  GTK_FILL,
			  GTK_FILL,
			  0,
			  0);

	spacer = gtk_label_new ("");
	gtk_widget_modify_font (spacer,
	pango_font_description_from_string (TABLE_FONT)
	);
	gtk_widget_set_size_request(GTK_WIDGET(spacer), 38, -1);


	/* Setting colors for in and out */
	sprintf(buf, "%s", IN_COLOR);
	gdk_color_parse (buf, &color);
	gtk_widget_modify_fg (graph->labels.net_in, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg (graph->labels.net_in_total, GTK_STATE_NORMAL, &color);

	sprintf(buf, "%s", OUT_COLOR);
	gdk_color_parse (buf, &color);
	gtk_widget_modify_fg (graph->labels.net_out, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg (graph->labels.net_out_total, GTK_STATE_NORMAL, &color);

	gtk_table_attach (GTK_TABLE (table), spacer, 3, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	closebutton = gtk_button_new_with_label ("Close");

	gtk_box_pack_start (GTK_BOX (net_legend_box), closebutton,
					TRUE, TRUE, 0);

	g_signal_connect (closebutton, "clicked",
			  G_CALLBACK (close_simon), graph);

	g_signal_connect (G_OBJECT(graph->drawing_area), "configure_event",
				  G_CALLBACK (load_graph_configure), graph);
	g_signal_connect (G_OBJECT (graph->drawing_area), "expose_event",
						  G_CALLBACK (load_graph_expose), (gpointer) graph);
	gtk_widget_set_events (graph->drawing_area, GDK_EXPOSURE_MASK);

	load_graph_update(graph);
	printf("Timeout: %d\n", SPEED / FRAMES_PER_UNIT);
	graph->timer_index = g_timeout_add (SPEED / FRAMES_PER_UNIT,
							load_graph_update,
							graph);



	return graph->mainvbox;

}



