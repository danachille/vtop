#ifndef DISK_H
#define DISK_H

#include <cairo.h>

// Function to get actual disk usage in percentage
double get_disk_usage();

// Function to draw the disk usage chart with used and free memory in different colors
void draw_disk_chart(cairo_t *cr, int x, int y, int width, int height);

// Function to update the disk usage chart
gboolean update_disk_chart(gpointer user_data);

#endif // DISK_H
