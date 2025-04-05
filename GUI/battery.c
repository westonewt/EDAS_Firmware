#include <gtk/gtk.h>
#include <cairo.h>

typedef struct {
    GtkWidget *da;
} AppWidgets;

int battery_percentage = 94;  // Simulated battery level

static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppWidgets *widgets) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    
    // Battery dimensions
    int battery_width = 40;
    int battery_height = 70;
    int tip_width = 15;
    int tip_height = 5;
    
    // Calculate centered position
    int x = (width - battery_width) / 2;
    int y = (height - battery_height - tip_height) / 2;

    // Draw battery tip (top)
    cairo_rectangle(cr, 
                   x + (battery_width - tip_width)/2, 
                   y, 
                   tip_width, 
                   tip_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    // Draw main battery body
    y += tip_height;
    cairo_rectangle(cr, x, y, battery_width, battery_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);

    // Calculate fill height
    double fill_height = battery_height * (battery_percentage / 100.0);
    
    // Set fill color
    if(battery_percentage > 50) {
        cairo_set_source_rgb(cr, 0.2, 0.7, 0.2); // Green
    } else if(battery_percentage > 20) {
        cairo_set_source_rgb(cr, 1.0, 0.8, 0.2); // Yellow
    } else {
        cairo_set_source_rgb(cr, 1.0, 0.2, 0.2); // Red
    }

    // Draw fill (from bottom up)
    cairo_rectangle(cr, 
                   x + 2, 
                   y + battery_height - fill_height + 2, 
                   battery_width - 4, 
                   fill_height - 4);
    cairo_fill(cr);

    return FALSE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    AppWidgets widgets;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Vertical Battery");
    gtk_window_set_default_size(GTK_WINDOW(window), 80, 120);

    widgets.da = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), widgets.da);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(widgets.da, "draw", G_CALLBACK(draw_battery), &widgets);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
