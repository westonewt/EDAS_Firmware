#include <gtk/gtk.h>
#include <math.h>

#define MAX_EFFICIENCY 100
#define UPDATE_INTERVAL 50  // Update interval in milliseconds

typedef struct {
    GtkWidget *drawing_area;
    GtkWidget *label;
    gdouble current_efficiency;
    gdouble average_efficiency;
} EfficiencyMeter;

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    gint width = allocation.width;
    gint height = allocation.height;
    gint center_x = width / 2;
    gint center_y = height / 2;
    gint radius = MIN(width, height) * 0.4;

    // White background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw dark gray dial
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 2);
    cairo_arc(cr, center_x, center_y, radius, -M_PI, 0);
    cairo_stroke(cr);

    // Draw markers and labels 
    for (int i = 0; i <= MAX_EFFICIENCY; i += 10) {
        double angle = (i / (double)MAX_EFFICIENCY) * M_PI - M_PI;
        cairo_save(cr);
        cairo_translate(cr, center_x, center_y);
        cairo_rotate(cr, angle);
        
        cairo_move_to(cr, radius * 0.85, 0);
        cairo_line_to(cr, radius * 0.95, 0);
        cairo_stroke(cr);
        
        if (i % 20 == 0) {
            cairo_save(cr);
            cairo_translate(cr, radius * 0.75, 0);
            cairo_rotate(cr, -angle);
            
            cairo_set_font_size(cr, 12);
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_text_extents_t extents;
            char *text = g_strdup_printf("%d", i);
            cairo_text_extents(cr, text, &extents);
            cairo_move_to(cr, -extents.width/2, extents.height/2);
            cairo_show_text(cr, text);
            g_free(text);
            cairo_restore(cr);
        }
        cairo_restore(cr);
    }

    // Draw average efficiency needle (green)
    double avg_needle_angle = (meter->average_efficiency / MAX_EFFICIENCY) * M_PI - M_PI;
    cairo_save(cr);
    cairo_translate(cr, center_x, center_y);
    cairo_rotate(cr, avg_needle_angle);
    
    cairo_set_source_rgb(cr, 0, 0.7, 0);  // Dark green color
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, -radius*0.1, 0);
    cairo_line_to(cr, radius * 0.75, 0);
    cairo_stroke(cr);
    cairo_restore(cr);

    // Draw current efficiency needle (red)
    double needle_angle = (meter->current_efficiency / MAX_EFFICIENCY) * M_PI - M_PI;
    cairo_save(cr);
    cairo_translate(cr, center_x, center_y);
    cairo_rotate(cr, needle_angle);
    
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, -radius*0.1, 0);
    cairo_line_to(cr, radius * 0.85, 0);
    cairo_stroke(cr);
    cairo_restore(cr);

    return FALSE;
}

static gboolean update_efficiency(gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;
    static gdouble t = 0;
    
    // Simulate efficiency using sine wave
    meter->current_efficiency = fabs(sin(t) * MAX_EFFICIENCY);
    // Calculate exponential moving average
    meter->average_efficiency = meter->average_efficiency * 0.9 + meter->current_efficiency * 0.1;
    t += 0.1;
    
    // Update text label
    gtk_label_set_text(GTK_LABEL(meter->label), 
        g_strdup_printf("Current Efficiency: %.1f%% | Average: %.1f%%", 
            meter->current_efficiency, meter->average_efficiency));
    
    // Redraw the meter
    gtk_widget_queue_draw(meter->drawing_area);
    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *box;
    EfficiencyMeter meter = {0};

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Efficiency Meter");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 350);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    meter.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(meter.drawing_area, 400, 300);
    g_signal_connect(meter.drawing_area, "draw", G_CALLBACK(on_draw), &meter);
    gtk_box_pack_start(GTK_BOX(box), meter.drawing_area, TRUE, TRUE, 0);

    meter.label = gtk_label_new("Current Efficiency: 0.0% | Average: 0.0%");
    gtk_box_pack_start(GTK_BOX(box), meter.label, FALSE, FALSE, 0);

    g_timeout_add(UPDATE_INTERVAL, update_efficiency, &meter);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
