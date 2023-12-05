#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/statvfs.h>

struct statvfs buf;

// Function to get actual disk usage in percentage
double get_disk_usage() {
    if (statvfs("/", &buf) == 0) {
        unsigned long long total = buf.f_frsize * buf.f_blocks;
        unsigned long long free_space = buf.f_frsize * buf.f_bfree;

        // Calculate disk usage percentage
        double usage_percentage = ((double)(total - free_space) / (double)total) * 100.0;

        return usage_percentage;
    } else {
        // Error occurred while retrieving disk usage
        g_warning("Error retrieving disk usage");
        return -1.0; // Return a negative value to indicate an error
    }
}


// Function to draw the disk usage chart with used and free memory in different colors
void draw_disk_chart(cairo_t *cr, int x, int y, int width, int height) {
    double disk_usage = get_disk_usage(); // Get the disk usage percentage

    // Calculate the angles for the used and free disk space sectors
    double used_angle = 2 * M_PI * (disk_usage / 100.0);
    double free_angle = 2 * M_PI - used_angle;

    // Calculate the radius and center of the chart
    double radius = MIN(width, height - 40) / 2.0; // Reduced height for legend
    double center_x = x + width / 2.0;
    double center_y = y + (height - 40) / 2.0; // Adjusted center position

    // Calculate the total disk size in gigabytes
    double total_gb = (double)buf.f_frsize * buf.f_blocks / (1024 * 1024 * 1024);

    // Calculate positions for percentage labels
    char used_percentage_text[50];
    char free_percentage_text[50];
    snprintf(used_percentage_text, sizeof(used_percentage_text), "Used: %.2f%%", disk_usage);
    snprintf(free_percentage_text, sizeof(free_percentage_text), "Free: %.2f%%", 100.0 - disk_usage);

    // Set up color for the percentage labels
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color for text
    cairo_set_font_size(cr, 12.0);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, used_percentage_text, &extents);
    double percentage_text_width = extents.width;
    double percentage_text_height = extents.height;

    // Display used percentage label
    cairo_move_to(cr, center_x - percentage_text_width / 2.0, center_y + radius + percentage_text_height + 5); // Adjusted position
    cairo_show_text(cr, used_percentage_text);

    // Calculate positions for free percentage label
    cairo_text_extents(cr, free_percentage_text, &extents);
    percentage_text_width = extents.width;

    // Display free percentage label
    cairo_move_to(cr, center_x - percentage_text_width / 2.0, center_y + radius + 2 * (percentage_text_height + 5)); // Adjusted position
    cairo_show_text(cr, free_percentage_text);

    // Set up colors for used and free sectors
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // Red color for used space

    // Draw the used sector
    cairo_move_to(cr, center_x, center_y);
    cairo_arc(cr, center_x, center_y, radius, -M_PI_2, -M_PI_2 + used_angle);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Set up color for free sector
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green color for free space

    // Draw the free sector
    cairo_move_to(cr, center_x, center_y);
    cairo_arc(cr, center_x, center_y, radius, -M_PI_2 + used_angle, -M_PI_2 + used_angle + free_angle);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Draw a border around the chart
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color
    cairo_set_line_width(cr, 2);
    cairo_arc(cr, center_x, center_y, radius, 0, 2 * M_PI);
    cairo_stroke(cr);

    // Display used and free memory in GB near the chart sectors
    char used_gb_text[50];
    char free_gb_text[50];
    snprintf(used_gb_text, sizeof(used_gb_text), "%.2f GB", (used_angle / (2 * M_PI)) * total_gb);
    snprintf(free_gb_text, sizeof(free_gb_text), "%.2f GB", (free_angle / (2 * M_PI)) * total_gb);

    // Calculate positions for GB labels
    double used_gb_x = center_x + radius * cos(-M_PI_2 + used_angle / 2.0) - percentage_text_width / 2.0;
    double used_gb_y = center_y + radius * sin(-M_PI_2 + used_angle / 2.0) + percentage_text_height + 5; // Adjusted position

    double free_gb_x = center_x + radius * cos(-M_PI_2 + used_angle + free_angle / 2.0) - percentage_text_width / 2.0;
    double free_gb_y = center_y + radius * sin(-M_PI_2 + used_angle + free_angle / 2.0) + percentage_text_height + 5; // Adjusted position

    // Display used memory in GB
    cairo_move_to(cr, used_gb_x, used_gb_y);
    cairo_show_text(cr, used_gb_text);

    // Display free memory in GB
    cairo_move_to(cr, free_gb_x, free_gb_y);
    cairo_show_text(cr, free_gb_text);
}


// Function to update the disk usage chart
gboolean update_disk_chart(gpointer user_data) {
    GtkWidget *area = GTK_WIDGET(user_data);
    if (gtk_widget_get_realized(area)) {
        GdkWindow *window = gtk_widget_get_window(area);
        cairo_t *cr = gdk_cairo_create(window);

        GtkAllocation allocation;
        gtk_widget_get_allocation(area, &allocation);
        int width = allocation.width;
        int height = allocation.height;

        // Get the updated disk usage percentage
        double disk_usage = get_disk_usage();

        // Clear the area before redrawing
        cairo_set_source_rgb(cr, 1, 1, 1); // White background
        cairo_paint(cr);

        // Draw the disk usage chart with the updated data
        draw_disk_chart(cr, 0, 0, width, height);

        cairo_destroy(cr);
    }
    return TRUE; // Ensures that the function will be called again; for one-time execution, return FALSE
}
