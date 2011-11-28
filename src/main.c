/*
 ============================================================================
 Name        : simon.c
 Author      : Jaakko Korkeaniemi
 Version     :
 Copyright   :
 Description : A simple program to visualize traffic trough a set of interfaces
 ============================================================================
 */


#include "network.h"
#include "gui.h"

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
  gtk_main_quit ();
}



static GtkWidget *start_window(Interface_list *interfaces) {

  GtkWidget *window;
  GtkWidget *wrapper;       /*Contains the graphs */

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);

  wrapper = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER (window), wrapper);

  int i;
  for(i = 0; i < interfaces->len; i++) {
    if(gtk_toggle_button_get_active((GtkToggleButton *)interfaces->interface[i].select_widget) == TRUE) {
      drawableInit(&interfaces->interface[i].data, interfaces->interface[i].ifname, interfaces->interface[i].ifname);
      interfaces->interface[i].ifwidget = get_and_start_network_load_widget(&interfaces->interface[i].data);
      gtk_box_pack_start(GTK_BOX(wrapper), interfaces->interface[i].ifwidget, FALSE, FALSE, 0);
    }
  }

  g_signal_connect (window, "destroy",
            G_CALLBACK (destroy), NULL);

  gtk_widget_show_all(wrapper);
  gtk_widget_show(GTK_WIDGET(window));

  return window;

}

static void interfaces_selected( GtkWidget *widget,
                     gpointer   data )
{

  Interface_list *interfaces = (Interface_list *)data;

  if(interfaces->window == NULL)
    interfaces->window = start_window(interfaces);

  gtk_widget_hide(interfaces->dialog);
}

int main(int argc, char *argv[]) {


	GtkWidget *dialog;
	GtkWidget *dialog_wrapper;
	GtkWidget *select_interfaces_label;
  GtkWidget *ready_button;
  Interface_list interfaces;
  int i;

	gtk_init(&argc, &argv);
	g_set_application_name("Traffic Monitor");


	/* Asking for interfaces to display */

	dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	dialog_wrapper = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER (dialog), dialog_wrapper);

  select_interfaces_label = gtk_label_new("Select interfaces to display:");
  gtk_box_pack_start(GTK_BOX(dialog_wrapper), select_interfaces_label, FALSE, FALSE, 0);

  interfaces = get_full_iflist();
  interfaces.dialog = dialog;

  for(i = 0; i < interfaces.len; i++) {
    GtkWidget *select = gtk_check_button_new_with_label(interfaces.interface[i].ifname);
    interfaces.interface[i].select_widget = select;
    gtk_box_pack_start(GTK_BOX(dialog_wrapper), select, FALSE, FALSE, 0);
  }

  ready_button = gtk_button_new_with_label("Draw!");
  gtk_box_pack_start(GTK_BOX(dialog_wrapper), ready_button, FALSE, FALSE, 0);


  g_signal_connect (ready_button, "clicked",
          G_CALLBACK (interfaces_selected), (gpointer) &interfaces);
  g_signal_connect (dialog, "destroy",
              G_CALLBACK (destroy), NULL);


  gtk_widget_show_all(dialog_wrapper);
  gtk_widget_show(GTK_WIDGET(dialog));

	//printf("Active interfaces: %d\n", nif);


	gtk_main();

	gtk_exit(0);

	return EXIT_SUCCESS;
}
