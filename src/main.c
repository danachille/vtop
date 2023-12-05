#include <gtk/gtk.h>
#include "cpu.h"
#include "disk.h"
#include "process.h"
#include "ram_swap.h"

// Forward declaration of the on_draw_event function
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);

// Function to create a drawing area and set up the draw event handler
GtkWidget* create_drawing_area(const char* user_data) {
    GtkWidget *drawing_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), (gpointer)user_data);
    return drawing_area;
}

void create_four_areas(GtkWidget *window, int window_width, int window_height) {
    // Create the main vertical pane
    GtkWidget *main_vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(window), main_vpaned);

    // Create the top and bottom horizontal panes
    GtkWidget *top_hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *bottom_hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    // Add the top and bottom panes to the main vertical pane
    gtk_paned_pack1(GTK_PANED(main_vpaned), top_hpaned, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(main_vpaned), bottom_hpaned, TRUE, TRUE);

    // Create and setup drawing areas for different functionalities
    GtkWidget *top_left_area = create_drawing_area("cpu_graph");
    GtkWidget *top_right_area = create_drawing_area("disk_graph");
    GtkWidget *bottom_right_area = create_drawing_area("ram_swap_graph");
    GtkWidget *bottom_left_area = create_process_view(); // Functionality not explicitly shown

    // Add these areas to their respective panes
    gtk_paned_pack1(GTK_PANED(top_hpaned), top_left_area, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(top_hpaned), top_right_area, TRUE, FALSE);
    gtk_paned_pack1(GTK_PANED(bottom_hpaned), bottom_left_area, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(bottom_hpaned), bottom_right_area, TRUE, FALSE);

    // Set the position of the panes
    gtk_paned_set_position(GTK_PANED(main_vpaned), window_height / 2);
    gtk_paned_set_position(GTK_PANED(top_hpaned), window_width / 2);
    gtk_paned_set_position(GTK_PANED(bottom_hpaned), window_width / 2);

    // Update functions for different areas
    g_timeout_add(500, (GSourceFunc)update_cpu_graph, top_left_area);
    g_timeout_add(1000, (GSourceFunc)update_ram_swap_graph, bottom_right_area);
    g_timeout_add(5000, (GSourceFunc)update_disk_chart, top_right_area);
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;

    cairo_set_source_rgb(cr, 1, 1, 1); // White background
    cairo_paint(cr);

    // Check the user_data string to determine which chart to draw
    const char *chart_type = (const char *)user_data;
    if (strcmp(chart_type, "cpu_graph") == 0) {
        draw_cpu_graph(cr, 0, 0, width, height);
    } else if (strcmp(chart_type, "disk_graph") == 0) {
        draw_disk_chart(cr, 0, 0, width, height);
    } else if (strcmp(chart_type, "ram_swap_graph") == 0) {
        draw_ram_swap_graph(cr, 0, 0, width, height);
    }

    return FALSE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Get the default screen dimensions
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    GdkRectangle workarea;
    gdk_monitor_get_workarea(monitor, &workarea);
    gint screen_width = workarea.width;
    gint screen_height = workarea.height;

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);

    // Signal to quit the GTK main loop when the window is closed
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Call the function to create four areas with window dimensions
    create_four_areas(window, screen_width, screen_height);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
