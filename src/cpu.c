// cpu.c

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h> 

#define MAX_CPU_CORES 8
#define HISTORY_SIZE 60
#define HORIZONTAL_LINES 5

typedef struct {
    long unsigned int idle, total, last_idle, last_total;
    int usage_history[HISTORY_SIZE];
} CPUUsage;

CPUUsage cpu_cores[MAX_CPU_CORES];
int num_cores = 0;

void read_cpu_usage() {

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Error opening /proc/stat");
        return;
    }

    char buffer[1024];
    for (int i = 0; i < MAX_CPU_CORES + 1; ++i) {
        if (!fgets(buffer, sizeof(buffer), fp)) {
            break;
        }

        if (strncmp(buffer, "cpu", 3) == 0 && isdigit(buffer[3])) {
            long user, nice, system, idle, iowait, irq, softirq, steal;
            sscanf(buffer, "cpu%*d %ld %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
            long total = user + nice + system + idle + iowait + irq + softirq + steal;
            long idle_time = idle + iowait;

            CPUUsage *cpu = &cpu_cores[i];
            long total_diff = total - cpu->last_total;
            long idle_diff = idle_time - cpu->last_idle;
            
            cpu->last_total = total;
            cpu->last_idle = idle_time;

            if (total_diff == 0) total_diff = 1;
            int usage = (int)(100 * (1 - (idle_diff / (double)total_diff)));
            memmove(&cpu->usage_history[1], &cpu->usage_history[0], (HISTORY_SIZE - 1) * sizeof(int));
            cpu->usage_history[0] = usage;


            num_cores = i + 1;
        }
    }

    fclose(fp);
}


void draw_cpu_graph(cairo_t *cr, int x, int y, int width, int height) {
    float colors[MAX_CPU_CORES][3] = {
        {0.7, 0.2, 0.2}, // Dark pastel red
        {0.2, 0.7, 0.2}, // Dark pastel green
        {0.2, 0.2, 0.7}, // Dark pastel blue
        {0.7, 0.7, 0.2}, // Dark pastel yellow
        {0.7, 0.2, 0.7}, // Dark pastel purple
        {0.2, 0.7, 0.7}, // Dark pastel cyan
        {0.5, 0.2, 0.2}, // Medium pastel red
        {0.5, 0.5, 0.5}  // Medium pastel grey
    };

    // Calculate the y positions for horizontal lines
    double horizontal_lines[HORIZONTAL_LINES];
    for (int i = 0; i < HORIZONTAL_LINES; ++i) {
        horizontal_lines[i] = height - (i * height / (HORIZONTAL_LINES - 1));
    }

    // Draw light grey horizontal lines
    for (int i = 0; i < HORIZONTAL_LINES; ++i) {
        cairo_set_source_rgb(cr, 0.8, 0.8, 0.8); // Light grey color
        double y_line = horizontal_lines[i];
        cairo_move_to(cr, x, y_line);
        cairo_line_to(cr, x + width, y_line);
        cairo_stroke(cr);

        // Add descriptions for 25%, 50%, and 75% horizontal lines
        if (i == 1 || i == 2 || i == 3) {
            int percentage = 25 * (i);
            char description[64];
            snprintf(description, sizeof(description), "%d%%", percentage);
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2); // Dark grey color for text
            cairo_move_to(cr, x + 10, y_line - 5); // Adjust position as needed
            cairo_show_text(cr, description);
        }
    }

    // Draw light grey vertical lines
    int num_vertical_lines = ceil(HISTORY_SIZE / 10.0);
    for (int i = 1; i < num_vertical_lines; ++i) {
        cairo_set_source_rgb(cr, 0.8, 0.8, 0.8); // Light grey color
        double vx = x + width - i * (width / num_vertical_lines);
        cairo_move_to(cr, vx, y);
        cairo_line_to(cr, vx, y + height);
        cairo_stroke(cr);

        // Add descriptions for 2, 4, 6, and 8 seconds vertical lines
        if (i == 5 || i == 3 || i == 1) {
            int seconds = i * 2;
            char description[64];
            snprintf(description, sizeof(description), "%d sec", seconds);
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2); // Dark grey color for text
            cairo_move_to(cr, vx - 25, y + height + 15); // Adjust position as needed
            cairo_show_text(cr, description);
        }
    }

    for (int i = 0; i < num_cores; ++i) {
        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_set_line_width(cr, 2);

        for (int j = 0; j < HISTORY_SIZE - 1; ++j) {
            double x1 = x + width - j * (width / HISTORY_SIZE);
            double y1 = y + height - (cpu_cores[i].usage_history[j] * height / 100);

            double x2 = x + width - (j + 1) * (width / HISTORY_SIZE);
            double y2 = y + height - (cpu_cores[i].usage_history[j + 1] * height / 100);

            if (j == 0) {
                cairo_move_to(cr, x1, y1);
            }

            double midx = (x1 + x2) / 2;
            double cp1x = (x1 + midx) / 2;
            double cp1y = y1;
            double cp2x = (x2 + midx) / 2;
            double cp2y = y2;

            cairo_curve_to(cr, cp1x, cp1y, cp2x, cp2y, x2, y2);
        }

        cairo_stroke(cr);
    }

    // Define starting positions for the legend
    int x_start = x + 50; // Horizontal starting position for the legend
    int legend_y_start = 30; // Vertical starting position for the legend
    int legend_y_offset = 20; // Vertical space between each legend item

    for (int i = 1; i < num_cores; ++i) {
        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_rectangle(cr,  x_start , legend_y_start + i * legend_y_offset - 20, 10, 10); // Draw a small square for the color
        cairo_fill(cr);

        // Prepare the text for the legend
        char legend_text[64];
        snprintf(legend_text, sizeof(legend_text), "Core %d: %d%%", i, cpu_cores[i].usage_history[0]);

        // Set text color and display it next to the color square
        cairo_set_source_rgb(cr, 0, 0, 0); // Black color for text
        cairo_move_to(cr, x_start + 20, legend_y_start + i * legend_y_offset - 10); // Adjust text position for each core
        cairo_show_text(cr, legend_text);
    }

}


gboolean update_cpu_graph(gpointer data) {

    read_cpu_usage();

    for (int i = 0; i < num_cores; ++i) {
        gtk_widget_queue_draw((GtkWidget *)data);
    }

    return TRUE;
}