/*
 * utils.c
 *
 *  Created on: Jul 17, 2010
 *      Author: jkorkean
 */

#include "utils.h"

/*
 * Clears the backround and forces redraw.
 *
 * Original source from gnome-system-monitor.
 */

void clear_background(Drawable *graph) {
	if (graph->background) {
		g_object_unref(graph->background);
		graph->background = NULL;
	}
}
