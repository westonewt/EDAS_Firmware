/*******************************************************
 * ECOCAR DRIVER ASSIST SYSTEM GUI
 * -----------------------------------------------------
 * Receives and displays real-time data:
 *   - Speed
 *   - Battery Level
 *   - Lap Number
 *   - Cabin Temperature
 *   - Current Fuel Efficiency
 *   - Average Fuel Efficiency
 *   - Crew Message to Driver
 *******************************************************/

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_EFFICIENCY 100
#define UPDATE_INTERVAL 50
#define GUI_SCALE_FACTOR 1.2 // Added scaling factor

typedef struct {
    GtkWidget *drawing_area;
    gdouble current_efficiency;
    gdouble average_efficiency;
    GtkLabel *current_label;
    GtkLabel *average_label;
} EfficiencyMeter;

typedef struct {
    GtkLabel *speed_label;
    GtkLabel *temp_label;
    GtkLabel *crew_msg_label;
    GtkLabel *lap_label;
    GtkLabel *battery_label;
    EfficiencyMeter efficiency_meter;
    int battery_percent;
    GtkWidget *battery_da;
} AppData;

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean update_efficiency(gpointer data);
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data);

int get_speed() { return 75; }
int get_battery() { return 50; }
int get_lap_number() { return 2; }
int get_temperature() { return 25; }
float get_current_fuel_efficiency() { return 50; }
float get_average_fuel_efficiency() { return 60; }
const char* get_crew_message() { return "You are the leader!"; }

gboolean update_speed(AppData *data) {
    char speed_text[32];
    snprintf(speed_text, sizeof(speed_text), "%d km/h", get_speed());
    gtk_label_set_text(data->speed_label, speed_text);
    return TRUE;
}

gboolean update_battery(AppData *data) {
    int raw_battery = get_battery();
    data->battery_percent = 100 - raw_battery;

    char batt_text[32];
    snprintf(batt_text, sizeof(batt_text), "%d%%", raw_battery);
    gtk_label_set_text(data->battery_label, batt_text);

    gtk_widget_queue_draw(data->battery_da);
    return TRUE;
}

gboolean update_lap_number(AppData *data) {
    char lap_text[32];
    snprintf(lap_text, sizeof(lap_text), "#%d", get_lap_number());
    gtk_label_set_text(data->lap_label, lap_text);
    return TRUE;
}

void adjust_temp(GtkButton *btn, GtkLabel *label) {
    int temp = get_temperature();
    const gchar *lbl = gtk_button_get_label(btn);
    temp += (strcmp(lbl, "+") == 0) ? 1 : -1;
    temp = (temp < 0) ? 0 : temp;

    char temp_text[16];
    snprintf(temp_text, sizeof(temp_text), "%d°C", temp);
    gtk_label_set_text(label, temp_text);
}

// Updates the message box
gboolean update_message(AppData *data) {
    const char *message = get_crew_message();
    gtk_label_set_text(data->crew_msg_label, message);
    return TRUE;
}

void on_ok_clicked(GtkButton *btn, GtkLabel *msg_label) {
    gtk_label_set_text(msg_label, "Acknowledged");
}

// The battery icon
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    int battery_width = 40 * GUI_SCALE_FACTOR; // Scaled
    int battery_height = 200 * GUI_SCALE_FACTOR; // Scaled
    int tip_width = 15 * GUI_SCALE_FACTOR; // Scaled
    int tip_height = 5 * GUI_SCALE_FACTOR; // Scaled

    int x = (width - battery_width) / 2;
    int y = (height - battery_height - tip_height) / 2;

    // Draw battery tip
    cairo_rectangle(cr, x + (battery_width - tip_width)/2, y, tip_width, tip_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    // Main battery body
    y += tip_height;
    cairo_rectangle(cr, x, y, battery_width, battery_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR); // Scaled
    cairo_stroke(cr);

    // Battery fill
    double fill_height = battery_height * (data->battery_percent / 100.0);

    if(data->battery_percent > 50) {
        cairo_set_source_rgb(cr, 0.2, 0.7, 0.2);
    } else if(data->battery_percent > 20) {
        cairo_set_source_rgb(cr, 1.0, 0.8, 0.2);
    } else {
        cairo_set_source_rgb(cr, 1.0, 0.2, 0.2);
    }

    cairo_rectangle(cr, x + 2 * GUI_SCALE_FACTOR, // Scaled
                   y + battery_height - fill_height + 2 * GUI_SCALE_FACTOR, // Scaled
                   battery_width - 4 * GUI_SCALE_FACTOR, // Scaled
                   fill_height - 4 * GUI_SCALE_FACTOR); // Scaled
    cairo_fill(cr);

    return FALSE;
}

// Efficiency Meter
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    gint width = allocation.width;
    gint height = allocation.height;
    gint center_x = width / 2;
    gint center_y = height / 2;
    gint radius = MIN(width, height) * 0.4;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw efficiency meter
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR); // Scaled
    cairo_arc(cr, center_x, center_y, radius, -M_PI, 0);
    cairo_stroke(cr);

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
            cairo_set_font_size(cr, 12 * GUI_SCALE_FACTOR); // Scaled
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

    double avg_needle_angle = (meter->average_efficiency / MAX_EFFICIENCY) * M_PI - M_PI;
    cairo_save(cr);
    cairo_translate(cr, center_x, center_y);
    cairo_rotate(cr, avg_needle_angle);
    cairo_set_source_rgb(cr, 0, 0.7, 0);
    cairo_set_line_width(cr, 3 * GUI_SCALE_FACTOR); // Scaled
    cairo_move_to(cr, -radius*0.1, 0);
    cairo_line_to(cr, radius * 0.75, 0);
    cairo_stroke(cr);
    cairo_restore(cr);

    double needle_angle = (meter->current_efficiency / MAX_EFFICIENCY) * M_PI - M_PI;
    cairo_save(cr);
    cairo_translate(cr, center_x, center_y);
    cairo_rotate(cr, needle_angle);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_set_line_width(cr, 3 * GUI_SCALE_FACTOR); // Scaled
    cairo_move_to(cr, -radius*0.1, 0);
    cairo_line_to(cr, radius * 0.85, 0);
    cairo_stroke(cr);
    cairo_restore(cr);

    return FALSE;
}

static gboolean update_efficiency(gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;

    meter->current_efficiency = get_current_fuel_efficiency();
    meter->average_efficiency = get_average_fuel_efficiency();

    // Update efficiency labels
    char current_text[32], average_text[32];
    snprintf(current_text, sizeof(current_text), "Current: %.1f%%", meter->current_efficiency);
    snprintf(average_text, sizeof(average_text), "Average: %.1f%%", meter->average_efficiency);
    gtk_label_set_text(meter->current_label, current_text);
    gtk_label_set_text(meter->average_label, average_text);

    gtk_widget_queue_draw(meter->drawing_area);
    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *box;
    GtkCssProvider *provider;
    AppData *data = g_new(AppData, 1);

    gtk_init(&argc, &argv);
    srand(time(NULL));

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Lab Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 320 * GUI_SCALE_FACTOR, 240 * GUI_SCALE_FACTOR); // Scaled
    gtk_window_fullscreen(GTK_WINDOW(window));

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15 * GUI_SCALE_FACTOR); // Scaled
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10 * GUI_SCALE_FACTOR); // Scaled

    // Battery drawing area
    GtkWidget *battery_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(battery_da, 40 * GUI_SCALE_FACTOR, 100 * GUI_SCALE_FACTOR); // Scaled
    gtk_grid_attach(GTK_GRID(grid), battery_da, 0, 0, 1, 3);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR); // Scaled
    gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 1);

    char lap_str[32];
    snprintf(lap_str, sizeof(lap_str), "#%d", get_lap_number());
    GtkWidget *lab_num = gtk_label_new(lap_str);

    GtkWidget *temp_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5 * GUI_SCALE_FACTOR); // Scaled
    GtkWidget *temp_label = gtk_label_new("25°C");

    GtkWidget *efficiency_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(efficiency_drawing_area, 150 * GUI_SCALE_FACTOR, 150 * GUI_SCALE_FACTOR); // Scaled

    gtk_box_pack_start(GTK_BOX(box), lab_num, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), temp_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), efficiency_drawing_area, TRUE, TRUE, 0);

    GtkWidget *minus_btn = gtk_button_new_with_label("-");
    GtkWidget *plus_btn = gtk_button_new_with_label("+");

    gtk_box_pack_start(GTK_BOX(temp_box), minus_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), plus_btn, FALSE, FALSE, 0);

    GtkWidget *col2_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR); // Scaled
    gtk_grid_attach(GTK_GRID(grid), col2_box, 2, 0, 1, 1);

    // Battery percentage label
    GtkWidget *battery_percent_label = gtk_label_new("");
    gtk_widget_set_halign(battery_percent_label, GTK_ALIGN_END);
    gtk_widget_set_valign(battery_percent_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(col2_box), battery_percent_label, FALSE, FALSE, 0);

    // Speed label
    GtkWidget *speed_label = gtk_label_new("0 km/h");
    gtk_widget_set_name(speed_label, "speed-label");
    gtk_box_pack_start(GTK_BOX(col2_box), speed_label, FALSE, FALSE, 0);

    // Efficiency labels
    GtkWidget *current_eff_label = gtk_label_new("");
    gtk_widget_set_name(current_eff_label, "current-eff-label");
    GtkWidget *average_eff_label = gtk_label_new("");
    gtk_widget_set_name(average_eff_label, "average-eff-label");
    gtk_box_pack_start(GTK_BOX(col2_box), current_eff_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(col2_box), average_eff_label, FALSE, FALSE, 0);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10 * GUI_SCALE_FACTOR); // Scaled
    gtk_grid_attach(GTK_GRID(grid), bottom_box, 0, 3, 3, 1);

    GtkWidget *msg_label = gtk_label_new("Crew Message");
    GtkWidget *ok_btn = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(bottom_box), msg_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(bottom_box), ok_btn, FALSE, FALSE, 0);

    provider = gtk_css_provider_new();
    gchar *css = g_strdup_printf(
        "grid, window { background-color: white; }"
        "#speed-label { font-size: %dpx; font-weight: bold; margin: %dpx; }" // Scaled
        "label { margin: %dpx; }" // Scaled
        "button { padding: %dpx %dpx; }" // Scaled
        "#current-eff-label { color: red; font-weight: bold; }"
        "#average-eff-label { color: green; font-weight: bold; }",
        (int)(48 * GUI_SCALE_FACTOR), // Scaled
        (int)(20 * GUI_SCALE_FACTOR), // Scaled
        (int)(5 * GUI_SCALE_FACTOR),  // Scaled
        (int)(5 * GUI_SCALE_FACTOR),  // Scaled
        (int)(15 * GUI_SCALE_FACTOR)  // Scaled
    );
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    g_free(css);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    data->speed_label = GTK_LABEL(speed_label);
    data->temp_label = GTK_LABEL(temp_label);
    data->crew_msg_label = GTK_LABEL(msg_label);
    data->lap_label = GTK_LABEL(lab_num);
    data->battery_label = GTK_LABEL(battery_percent_label);
    data->efficiency_meter.drawing_area = efficiency_drawing_area;
    data->efficiency_meter.current_efficiency = 0.0;
    data->efficiency_meter.average_efficiency = 0.0;
    data->efficiency_meter.current_label = GTK_LABEL(current_eff_label);
    data->efficiency_meter.average_label = GTK_LABEL(average_eff_label);
    data->battery_da = battery_da;
    data->battery_percent = 100 - get_battery();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ok_btn, "clicked", G_CALLBACK(on_ok_clicked), msg_label);
    g_signal_connect(minus_btn, "clicked", G_CALLBACK(adjust_temp), temp_label);
    g_signal_connect(plus_btn, "clicked", G_CALLBACK(adjust_temp), temp_label);
    g_signal_connect(efficiency_drawing_area, "draw", G_CALLBACK(on_draw), &data->efficiency_meter);
    g_signal_connect(battery_da, "draw", G_CALLBACK(draw_battery), data);

    g_timeout_add(1000, (GSourceFunc)update_speed, data);
    g_timeout_add(500, (GSourceFunc)update_battery, data);
    g_timeout_add(3000, (GSourceFunc)update_message, data);
    g_timeout_add(1000, (GSourceFunc)update_lap_number, data);
    g_timeout_add(UPDATE_INTERVAL, update_efficiency, &data->efficiency_meter);

    gtk_widget_show_all(window);
    gtk_main();

    g_free(data);
    return 0;
}
