#ifndef CPU_H
#define CPU_H

#include <gtk/gtk.h>

#define MAX_CPU_CORES 9
#define HISTORY_SIZE 60
#define HORIZONTAL_LINES 5

// Structure to hold CPU usage information
typedef struct {
    long unsigned int idle, total, last_idle, last_total;
    int usage_history[HISTORY_SIZE];
} CPUUsage;

// Prototypes
void read_cpu_usage();
void draw_cpu_graph(cairo_t *cr, int x, int y, int width, int height);
gboolean update_cpu_graph(gpointer data);

#endif // CPU_H
