#ifndef PROCESS_H
#define PROCESS_H

#include <gtk/gtk.h>

// ProcessInfo structure definition
typedef struct {
    char name[256]; // Process name
    pid_t pid;      // Process ID
    // Add other fields as needed
} ProcessInfo;

// Function declarations
GtkWidget* create_process_view(void);

#endif // PROCESS_H
