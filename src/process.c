#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/sysinfo.h>

typedef struct {
    char name[256];          // Process name
    pid_t pid;               // Process ID
    uid_t uid;               // User ID
    unsigned long mem;       // Memory usage in KB
    char state;              // Process state
    long unsigned int utime; // User mode jiffies
    long unsigned int stime; // Kernel mode jiffies
    // Add other fields as needed
} ProcessInfo;

void update_process_list(GtkListStore *store) {
    gtk_list_store_clear(store);

    DIR *dir = opendir("/proc");
    if (dir != NULL) {
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {
            if (isdigit(entry->d_name[0])) {
                ProcessInfo process;
                memset(&process, 0, sizeof(ProcessInfo));

                char status_path[PATH_MAX];
                char stat_path[PATH_MAX];
                sprintf(status_path, "/proc/%s/status", entry->d_name);
                sprintf(stat_path, "/proc/%s/stat", entry->d_name);

                FILE *status_file = fopen(status_path, "r");
                FILE *stat_file = fopen(stat_path, "r");
                if (status_file && stat_file) {
                    process.pid = atoi(entry->d_name);

                    char line[256];
                    while (fgets(line, sizeof(line), status_file)) {
                        if (strncmp(line, "Name:", 5) == 0) {
                            sscanf(line, "Name:\t%255s", process.name);
                        } else if (strncmp(line, "Uid:", 4) == 0) {
                            sscanf(line, "Uid:\t%u", &process.uid);
                        } else if (strncmp(line, "VmSize:", 7) == 0) {
                            sscanf(line, "VmSize:\t%lu", &process.mem);
                        }
                        // Further parsing for other details
                    }

                    if (fscanf(stat_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu",
                               &process.state, &process.utime, &process.stime) != 3) {
                        // Handle error
                        // Consider what to do in case of fscanf failure
                    }

                    fclose(status_file);
                    fclose(stat_file);

                    // Getting user name from UID
                    struct passwd *pwd = getpwuid(process.uid);
                    const char *username = (pwd != NULL) ? pwd->pw_name : "Unknown";

                    GtkTreeIter iter;
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                        0, process.name,
                        1, process.pid,
                        2, username,
                        3, process.mem,
                        4, process.state,
                        5, process.utime,
                        6, process.stime,
                        -1);
                }
            }
        }
        closedir(dir);
    }
}

GtkWidget* create_process_view(void) {
    GtkListStore *store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_CHAR, G_TYPE_ULONG, G_TYPE_ULONG);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("Process Name", renderer, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("User", renderer, "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("Memory (KB)", renderer, "text", 3, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("State", renderer, "text", 4, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("User Time", renderer, "text", 5, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), 
        gtk_tree_view_column_new_with_attributes("System Time", renderer, "text", 6, NULL));

    // Apply CSS styling to set background to white and text color to black
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider, "treeview { background-color: white; color: black; }", -1, NULL);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);
    gtk_style_context_add_provider(gtk_widget_get_style_context(tree_view), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_timeout_add_seconds(1, (GSourceFunc)update_process_list, store);

    return scrolled_window;
}
