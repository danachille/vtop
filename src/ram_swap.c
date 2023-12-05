#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define HISTORY_SIZE 60
#define HORIZONTAL_LINES 5

typedef struct {
    long unsigned int total_ram, free_ram, total_swap, free_swap, last_total_ram, last_free_ram, last_total_swap, last_free_swap;
    float ram_usage_history[HISTORY_SIZE];
    float swap_usage_history[HISTORY_SIZE];
} RAMSwapUsage;

RAMSwapUsage ram_swap_data;

void read_ram_swap_usage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Error opening /proc/meminfo");
        return;
    }

    char buffer[1024];
    long total_mem = 0, available_mem = 0, total_swap = 0, free_swap = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        long value;
        if (sscanf(buffer, "MemTotal: %ld kB", &value) == 1) {
            total_mem = value;
        } else if (sscanf(buffer, "MemAvailable: %ld kB", &value) == 1) {
            available_mem = value;
        } else if (sscanf(buffer, "SwapTotal: %ld kB", &value) == 1) {
            total_swap = value;
        } else if (sscanf(buffer, "SwapFree: %ld kB", &value) == 1) {
            free_swap = value;
        }
    }

    fclose(fp);

    // Calculate used RAM and swap space in gigabytes
    double used_ram_gb = ((double)(total_mem - available_mem) / (1024.0 * 1024.0)); // Convert kB to GB
    double used_swap_gb = ((double)(total_swap - free_swap) / (1024.0 * 1024.0)); // Convert kB to GB

    // Update usage history in gigabytes
    ram_swap_data.ram_usage_history[0] = (float)used_ram_gb;
    ram_swap_data.swap_usage_history[0] = (float)used_swap_gb;
}

void draw_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double radius) {
    double aspect = 1.0;
    double corner_radius = radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - corner_radius, y + corner_radius, corner_radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + width - corner_radius, y + height - corner_radius, corner_radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + corner_radius, y + height - corner_radius, corner_radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + corner_radius, y + corner_radius, corner_radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
}

void draw_ram_swap_graph(cairo_t *cr, int x, int y, int width, int height) {
    // Define colors for RAM and swap bars
    double ram_color[3] = {0.2, 0.4, 0.8}; // Blue color for RAM
    double swap_color[3] = {0.8, 0.2, 0.2}; // Red color for swap

    // Calculate bar heights based on used RAM and swap space in gigabytes
    double used_ram_gb = ram_swap_data.ram_usage_history[0];
    double used_swap_gb = ram_swap_data.swap_usage_history[0];

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) {
        perror("Error opening /proc/meminfo");
        return;
    }

    long total_mem = 0, total_swap = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), meminfo)) {
        long value;
        if (sscanf(buffer, "MemTotal: %ld kB", &value) == 1) {
            total_mem = value;
        } else if (sscanf(buffer, "SwapTotal: %ld kB", &value) == 1) {
            total_swap = value;
        }
    }

    fclose(meminfo);

    double max_ram = (double)total_mem / (1024.0 * 1024.0); // Convert kB to GB
    double max_swap = (double)total_swap / (1024.0 * 1024.0); // Convert kB to GB

    // Calculate bar heights based on total RAM and swap space
    double ram_bar_height = (used_ram_gb / max_ram) * height;
    double swap_bar_height = (used_swap_gb / max_swap) * height;

    // Define bar positions and dimensions at the center of the screen
    double bar_width = 60;
    double ram_bar_x = (width - bar_width * 2 - 20) / 2;
    double swap_bar_x = ram_bar_x + bar_width + 20;

    // Draw RAM bar with rounded corners
    cairo_set_source_rgb(cr, ram_color[0], ram_color[1], ram_color[2]);
    draw_rounded_rectangle(cr, ram_bar_x, y + height - ram_bar_height, bar_width, ram_bar_height, 10);
    cairo_fill(cr);

    // Draw swap bar with rounded corners
    cairo_set_source_rgb(cr, swap_color[0], swap_color[1], swap_color[2]);
    draw_rounded_rectangle(cr, swap_bar_x, y + height - swap_bar_height, bar_width, swap_bar_height, 10);
    cairo_fill(cr);

    // Display text for used RAM and swap space in gigabytes at the top middle
    cairo_set_source_rgb(cr, 0, 0, 0); // Black color for text
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);

    // RAM text
    char ram_text[50];
    snprintf(ram_text, sizeof(ram_text), "RAM: %.2f GB / %.2f GB", used_ram_gb, max_ram);
    cairo_text_extents_t extents_ram;
    cairo_text_extents(cr, ram_text, &extents_ram);
    cairo_move_to(cr, (width - extents_ram.width) / 2, y + extents_ram.height + 10);
    cairo_show_text(cr, ram_text);

    // Swap text
    char swap_text[50];
    snprintf(swap_text, sizeof(swap_text), "Swap: %.2f GB / %.2f GB", used_swap_gb, max_swap);
    cairo_text_extents_t extents_swap;
    cairo_text_extents(cr, swap_text, &extents_swap);
    cairo_move_to(cr, (width - extents_swap.width) / 2, y + extents_ram.height + 10 + extents_ram.height + 10);
    cairo_show_text(cr, swap_text);
}

gboolean update_ram_swap_graph(gpointer data) {
    read_ram_swap_usage();
    gtk_widget_queue_draw((GtkWidget *)data);
    return TRUE;
}