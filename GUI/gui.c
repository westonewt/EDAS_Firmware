/*******************************************************
 * ECOCAR DRIVER ASSIST SYSTEM GUI
 * -----------------------------------------------------
 * Displays real-time data for an EcoCar driver:
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

#define MAX_EFFICIENCY 100      // Maximum efficiency value for meter
#define UPDATE_INTERVAL 50      // Efficiency update interval in milliseconds
#define GUI_SCALE_FACTOR 1.7    // Scaling factor for GUI elements

// Structure for efficiency meter data
typedef struct {
    GtkWidget *drawing_area;    // Widget for drawing the efficiency meter
    gdouble current_efficiency; // Current fuel efficiency value
    gdouble average_efficiency; // Average fuel efficiency value
    GtkLabel *current_label;    // Label for current efficiency
    GtkLabel *average_label;    // Label for average efficiency
    gboolean h2_alarm;          // Flag for H2 alarm state
} EfficiencyMeter;

// Structure for application data
typedef struct {
    GtkLabel *speed_label;      // Label for displaying speed
    GtkLabel *temp_label;       // Label for displaying temperature
    GtkLabel *crew_msg_label;   // Label for crew messages
    GtkLabel *lap_label;        // Label for lap number
    GtkLabel *battery_label;    // Label for battery percentage
    EfficiencyMeter efficiency_meter; // Efficiency meter data
    int battery_percent;        // Current battery percentage
    GtkWidget *battery_da;      // Drawing area for battery icon
    int current_temp;           // Current cabin temperature
    guint temp_timeout_id;      // Timeout ID for temperature revert
} AppData;

// Function prototypes
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean update_efficiency(gpointer data);
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data);
static gboolean temp_timeout_callback(gpointer user_data);

// Simulated data retrieval functions
int get_speed() { return 30; }                    // Returns current speed in km/h
int get_battery() { return 50; }                  // Returns battery level (0-100)
int get_lap_number() { return 2; }                // Returns current lap number
int get_temperature() { return 25; }              // Returns cabin temperature in °C
float get_current_fuel_efficiency() { return 50; } // Returns current fuel efficiency
float get_average_fuel_efficiency() { return 60; } // Returns average fuel efficiency
const char* get_crew_message() { return "You are the leader!"; } // Returns crew message
gboolean get_h2_alarm() { return 1; }             // Returns H2 alarm state

// Hides the cursor when the window is realized
void hide_cursor(GtkWidget *widget) {
    GdkWindow *gdk_window = gtk_widget_get_window(widget);
    if (!gdk_window) return;

    GdkDisplay *display = gdk_window_get_display(gdk_window);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    GdkCursor *invisible_cursor = gdk_cursor_new_from_surface(display, surface, 0, 0);
    cairo_surface_destroy(surface);

    gdk_window_set_cursor(gdk_window, invisible_cursor);
    g_object_unref(invisible_cursor);
}

// Updates the speed label with current speed
gboolean update_speed(AppData *data) {
    char speed_text[32];
    snprintf(speed_text, sizeof(speed_text), "%d km/h", get_speed());
    gtk_label_set_text(data->speed_label, speed_text);
    return TRUE;
}

// Updates the battery label and redraws the battery icon
gboolean update_battery(AppData *data) {
    int raw_battery = get_battery();
    data->battery_percent = 100 - raw_battery;

    char batt_text[32];
    snprintf(batt_text, sizeof(batt_text), "%d%%", raw_battery);
    gtk_label_set_text(data->battery_label, batt_text);

    gtk_widget_queue_draw(data->battery_da);
    return TRUE;
}

// Updates the lap number label
gboolean update_lap_number(AppData *data) {
    char lap_text[32];
    snprintf(lap_text, sizeof(lap_text), "#%d", get_lap_number());
    gtk_label_set_text(data->lap_label, lap_text);
    return TRUE;
}

// Adjusts cabin temperature based on button press and sets a revert timeout
void adjust_temp(GtkButton *btn, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    const gchar *lbl = gtk_button_get_label(btn);
    data->current_temp += (strcmp(lbl, "+") == 0) ? 1 : -1;
    data->current_temp = (data->current_temp < 0) ? 0 : data->current_temp;

    char temp_text[16];
    snprintf(temp_text, sizeof(temp_text), "%d°C", data->current_temp);
    gtk_label_set_text(data->temp_label, temp_text);

    if (data->temp_timeout_id != 0) {
        g_source_remove(data->temp_timeout_id);
        data->temp_timeout_id = 0;
    }

    data->temp_timeout_id = g_timeout_add_seconds(5, temp_timeout_callback, data);
}

// Reverts temperature to sensor value after timeout
static gboolean temp_timeout_callback(gpointer user_data) {
    AppData *data = (AppData *)user_data;
    int current_temp = get_temperature();
    data->current_temp = current_temp;

    char temp_text[16];
    snprintf(temp_text, sizeof(temp_text), "%d°C", current_temp);
    gtk_label_set_text(data->temp_label, temp_text);

    data->temp_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

// Updates the crew message label
gboolean update_message(AppData *data) {
    const char *message = get_crew_message();
    gtk_label_set_text(data->crew_msg_label, message);
    return TRUE;
}

// Acknowledges crew message when OK button is clicked
void on_ok_clicked(GtkButton *btn, GtkLabel *msg_label) {
    gtk_label_set_text(msg_label, "Acknowledged");
}

// Draws the battery icon with fill based on percentage
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    int battery_width = 40 * GUI_SCALE_FACTOR;
    int battery_height = 200 * GUI_SCALE_FACTOR;
    int tip_width = 15 * GUI_SCALE_FACTOR;
    int tip_height = 5 * GUI_SCALE_FACTOR;

    int x = (width - battery_width) / 2;
    int y = (height - battery_height - tip_height) / 2;

    // Draw battery tip
    cairo_rectangle(cr, x + (battery_width - tip_width)/2, y, tip_width, tip_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    // Draw battery body outline
    y += tip_height;
    cairo_rectangle(cr, x, y, battery_width, battery_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR);
    cairo_stroke(cr);

    // Fill battery based on percentage
    double fill_height = battery_height * (data->battery_percent / 100.0);
    if (data->battery_percent > 50) {
        cairo_set_source_rgb(cr, 0.2, 0.7, 0.2); // Green for >50%
    } else if (data->battery_percent > 20) {
        cairo_set_source_rgb(cr, 1.0, 0.8, 0.2); // Yellow for 20-50%
    } else {
        cairo_set_source_rgb(cr, 1.0, 0.2, 0.2); // Red for <20%
    }

    cairo_rectangle(cr, x + 2 * GUI_SCALE_FACTOR,
                   y + battery_height - fill_height + 2 * GUI_SCALE_FACTOR,
                   battery_width - 4 * GUI_SCALE_FACTOR,
                   fill_height - 4 * GUI_SCALE_FACTOR);
    cairo_fill(cr);

    return FALSE;
}

// Draws the efficiency meter with needles and optional alarm border
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    gint width = allocation.width;
    gint height = allocation.height;
    gint center_x = width / 2;
    gint center_y = height / 2;
    gint radius = MIN(width, height) * 0.4;

    // Clear background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw meter arc
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR);
    cairo_arc(cr, center_x, center_y, radius, -M_PI, 0);
    cairo_stroke(cr);

    // Draw tick marks and labels
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
            cairo_set_font_size(cr, 12 * GUI_SCALE_FACTOR);
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
    cairo_set_source_rgb(cr, 0, 0.7, 0);
    cairo_set_line_width(cr, 3 * GUI_SCALE_FACTOR);
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
    cairo_set_line_width(cr, 3 * GUI_SCALE_FACTOR);
    cairo_move_to(cr, -radius*0.1, 0);
    cairo_line_to(cr, radius * 0.85, 0);
    cairo_stroke(cr);
    cairo_restore(cr);

    // Draw red border if H2 alarm is active
    if (meter->h2_alarm) {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_set_line_width(cr, 4 * GUI_SCALE_FACTOR);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_stroke(cr);
    }

    return FALSE;
}

// Updates efficiency meter and labels periodically
static gboolean update_efficiency(gpointer data) {
    EfficiencyMeter *meter = (EfficiencyMeter *)data;

    meter->current_efficiency = get_current_fuel_efficiency();
    meter->average_efficiency = get_average_fuel_efficiency();
    meter->h2_alarm = get_h2_alarm();

    char current_text[32], average_text[32];
    snprintf(current_text, sizeof(current_text), "Current: %.1f%%", meter->current_efficiency);
    snprintf(average_text, sizeof(average_text), "Average: %.1f%%", meter->average_efficiency);
    gtk_label_set_text(meter->current_label, current_text);
    gtk_label_set_text(meter->average_label, average_text);

    gtk_widget_queue_draw(meter->drawing_area);
    return G_SOURCE_CONTINUE;
}

// Main function: Initializes GUI and starts event loop
int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *box;
    GtkCssProvider *provider;
    AppData *data = g_new(AppData, 1);

    gtk_init(&argc, &argv);
    srand(time(NULL));

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Lab Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 320 * GUI_SCALE_FACTOR, 240 * GUI_SCALE_FACTOR);
    gtk_window_fullscreen(GTK_WINDOW(window));

    // Set up grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15 * GUI_SCALE_FACTOR);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10 * GUI_SCALE_FACTOR);

    // Battery drawing area
    GtkWidget *battery_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(battery_da, 40 * GUI_SCALE_FACTOR, 100 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), battery_da, 0, 0, 1, 3);

    // Left column box (lap, temp, efficiency)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 1);

    char lap_str[32];
    snprintf(lap_str, sizeof(lap_str), "#%d", get_lap_number());
    GtkWidget *lab_num = gtk_label_new(lap_str);

    GtkWidget *temp_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5 * GUI_SCALE_FACTOR);
    GtkWidget *temp_label = gtk_label_new("25°C");

    GtkWidget *efficiency_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(efficiency_drawing_area, 150 * GUI_SCALE_FACTOR, 150 * GUI_SCALE_FACTOR);

    gtk_box_pack_start(GTK_BOX(box), lab_num, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), temp_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), efficiency_drawing_area, TRUE, TRUE, 0);

    // Temperature control buttons
    GtkWidget *minus_btn = gtk_button_new_with_label("-");
    GtkWidget *plus_btn = gtk_button_new_with_label("+");
    gtk_box_pack_start(GTK_BOX(temp_box), minus_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), plus_btn, FALSE, FALSE, 0);

    // Right column box (battery %, speed, efficiency labels)
    GtkWidget *col2_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), col2_box, 2, 0, 1, 1);

    GtkWidget *battery_percent_label = gtk_label_new("");
    gtk_widget_set_halign(battery_percent_label, GTK_ALIGN_END);
    gtk_widget_set_valign(battery_percent_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(col2_box), battery_percent_label, FALSE, FALSE, 0);

    GtkWidget *speed_label = gtk_label_new("0 km/h");
    gtk_widget_set_name(speed_label, "speed-label");
    gtk_box_pack_start(GTK_BOX(col2_box), speed_label, FALSE, FALSE, 0);

    GtkWidget *current_eff_label = gtk_label_new("");
    gtk_widget_set_name(current_eff_label, "current-eff-label");
    GtkWidget *average_eff_label = gtk_label_new("");
    gtk_widget_set_name(average_eff_label, "average-eff-label");
    gtk_box_pack_start(GTK_BOX(col2_box), current_eff_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(col2_box), average_eff_label, FALSE, FALSE, 0);

    // Bottom message box
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), bottom_box, 0, 3, 3, 1);

    GtkWidget *msg_label = gtk_label_new("Crew Message");
    GtkWidget *ok_btn = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(bottom_box), msg_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(bottom_box), ok_btn, FALSE, FALSE, 0);

    // Apply CSS styling
    provider = gtk_css_provider_new();
    gchar *css = g_strdup_printf(
        "grid, window { background-color: white; }"
        "#speed-label { font-size: %dpx; font-weight: bold; margin: %dpx; }"
        "label { margin: %dpx; }"
        "button { padding: %dpx %dpx; }"
        "#current-eff-label { color: red; font-weight: bold; }"
        "#average-eff-label { color: green; font-weight: bold; }",
        (int)(48 * GUI_SCALE_FACTOR),
        (int)(20 * GUI_SCALE_FACTOR),
        (int)(5 * GUI_SCALE_FACTOR),
        (int)(5 * GUI_SCALE_FACTOR),
        (int)(15 * GUI_SCALE_FACTOR)
    );
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    g_free(css);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Initialize AppData structure
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
    data->current_temp = get_temperature();
    data->temp_timeout_id = 0;

    // Connect signals
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ok_btn, "clicked", G_CALLBACK(on_ok_clicked), msg_label);
    g_signal_connect(minus_btn, "clicked", G_CALLBACK(adjust_temp), data);
    g_signal_connect(plus_btn, "clicked", G_CALLBACK(adjust_temp), data);
    g_signal_connect(efficiency_drawing_area, "draw", G_CALLBACK(on_draw), &data->efficiency_meter);
    g_signal_connect(battery_da, "draw", G_CALLBACK(draw_battery), data);

    // Set up periodic updates
    g_timeout_add(1000, (GSourceFunc)update_speed, data);
    g_timeout_add(500, (GSourceFunc)update_battery Introduced in your code snippet, data);
    g_timeout_add(3000, (GSourceFunc)update_message, data);
    g_timeout_add(1000, (GSourceFunc)update_lap_number, data);
    g_timeout_add(UPDATE_INTERVAL, update_efficiency, &data->efficiency_meter);

    g_signal_connect(window, "realize", G_CALLBACK(hide_cursor), NULL);

    // Show window and start main loop
    gtk_widget_show_all(window);
    gtk_main();

    g_free(data);
    return 0;
}
    g_free(data);
    return 0;
}
