#ifndef RAM_SWAP_H
#define RAM_SWAP_H

#include <gtk/gtk.h>

typedef struct {
    long unsigned int total_ram, free_ram, total_swap, free_swap, last_total_ram, last_free_ram, last_total_swap, last_free_swap;
    int ram_usage_history[60];
    int swap_usage_history[60];
} RAMSwapUsage;

extern RAMSwapUsage ram_swap_data;

void read_ram_swap_usage();
void draw_ram_swap_graph(cairo_t *cr, int x, int y, int width, int height);
gboolean update_ram_swap_graph(gpointer data);

#endif /* RAM_SWAP_H */
