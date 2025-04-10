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
#include <glib.h>
#include <gpiod.h>
#include <errno.h>

// Constants for efficiency meter and GUI settings
#define MAX_EFFICIENCY 100      // Maximum efficiency value (100%)
#define UPDATE_INTERVAL 50      // Update interval in milliseconds
#define GUI_SCALE_FACTOR 1.45   // Scaling factor for GUI elements

// GPIO pin definitions
#define GPIO_CHIP "gpiochip0"   // GPIO chip identifier
#define GPIO_TEMP_UP 12         // GPIO pin for temperature increase
#define GPIO_TEMP_DOWN 5        // GPIO pin for temperature decrease
#define GPIO_ACK 6             // GPIO pin for message acknowledgment

// Structure to hold efficiency meter data
typedef struct {
    GtkWidget *drawing_area;    // Widget for drawing the efficiency gauge
    gdouble current_efficiency; // Current efficiency value
    gdouble average_efficiency; // Average efficiency value
    GtkLabel *current_label;    // Label for current efficiency
    GtkLabel *average_label;    // Label for average efficiency
    gboolean h2_alarm;          // Hydrogen alarm status
} EfficiencyMeter;

// Main application data structure
typedef struct {
    GtkLabel *speed_label;      // Label for displaying speed
    GtkLabel *temp_label;       // Label for displaying temperature
    GtkLabel *crew_msg_label;   // Label for crew messages
    GtkLabel *lap_label;        // Label for lap number
    GtkLabel *battery_label;    // Label for battery percentage
    EfficiencyMeter efficiency_meter; // Embedded efficiency meter structure
    int battery_percent;        // Current battery percentage
    GtkWidget *battery_da;      // Drawing area for battery visualization
    int current_temp;           // Current temperature value
    guint temp_timeout_id;      // ID for temperature timeout
    struct gpiod_chip *chip;    // GPIO chip handle
    struct gpiod_line *line_temp_up;    // GPIO line for temp up
    struct gpiod_line *line_temp_down;  // GPIO line for temp down
    struct gpiod_line *line_ack;        // GPIO line for ack
} AppData;

// Function prototypes
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean update_efficiency(gpointer data);
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data);
static gboolean temp_timeout_callback(gpointer user_data);
static gboolean update_message(gpointer data);
static gboolean gpio_event_handler(GIOChannel *source, GIOCondition condition, gpointer user_data);

// Generates a random speed value between 28-33 km/h
int get_speed() {
    int base_speed = rand() % 6 + 28;
    if (base_speed == 29) {
        return (rand() % 2 == 0) ? 28 : 30;
    }
    return base_speed;
}

// Simulates battery level decreasing every 5 seconds
int get_battery() {
    static int battery = 100;
    static GTimer *timer = NULL;
    if (timer == NULL) timer = g_timer_new();
    if (g_timer_elapsed(timer, NULL) >= 5.0) {
        battery--;
        if (battery < 0) battery = 0;
        g_timer_reset(timer);
    }
    return battery;
}

// Tracks lap number, incrementing every 10 seconds
int get_lap_number() {
    static int lap = 0;
    static GTimer *timer = NULL;
    if (timer == NULL) timer = g_timer_new();
    if (g_timer_elapsed(timer, NULL) >= 10.0) {
        lap++;
        g_timer_reset(timer);
    }
    return lap;
}

// Returns a constant temperature value (placeholder)
int get_temperature() { return 25; }

// Simulates current fuel efficiency with random fluctuations
float get_current_fuel_efficiency() {
    static float last_efficiency = 50.0;
    static GTimer *timer = NULL;
    if (timer == NULL) timer = g_timer_new();
    if (g_timer_elapsed(timer, NULL) >= 2.0) {
        float range = last_efficiency * 0.1;
        float min_val = last_efficiency - range;
        float max_val = last_efficiency + range;
        if (min_val < 30.0) min_val = 30.0;
        if (max_val > 80.0) max_val = 80.0;
        last_efficiency = min_val + ((float)rand() / RAND_MAX) * (max_val - min_val);
        g_timer_reset(timer);
    }
    return last_efficiency;
}

// Calculates running average of fuel efficiency
float get_average_fuel_efficiency() {
    static float total = 0.0;
    static int count = 0;
    static float last_avg = 60.0;
    float current = get_current_fuel_efficiency();
    total += current;
    count++;
    last_avg = total / count;
    if (last_avg < 40) last_avg = 40;
    if (last_avg > 70) last_avg = 70;
    return last_avg;
}

// Returns a static crew message (placeholder)
const char* get_crew_message() { return "You are the leader!"; }

// Simulates H2 alarm toggling every 5 seconds
gboolean get_h2_alarm() {
    static gboolean alarm_state = FALSE;
    static GTimer *timer = NULL;
    if (timer == NULL) timer = g_timer_new();
    if (g_timer_elapsed(timer, NULL) >= 5.0) {
        alarm_state = !alarm_state;
        g_timer_reset(timer);
    }
    return alarm_state;
}

// Hides the mouse cursor in the window
void hide_cursor(GtkWidget *widget) {
    GdkWindow *gdk_window = gtk_widget_get_window(widget);
    GdkDisplay *display = gdk_window_get_display(gdk_window);
    GdkCursor *invisible_cursor;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    invisible_cursor = gdk_cursor_new_from_surface(display, surface, 0, 0);
    cairo_surface_destroy(surface);
    gdk_window_set_cursor(gdk_window, invisible_cursor);
    g_object_unref(invisible_cursor);
}

// Updates the speed label with current value
gboolean update_speed(AppData *data) {
    char speed_text[32];
    snprintf(speed_text, sizeof(speed_text), "%d km/h", get_speed());
    gtk_label_set_text(data->speed_label, speed_text);
    return TRUE;
}

// Updates battery level and triggers redraw
gboolean update_battery(AppData *data) {
    int raw_battery = get_battery();
    data->battery_percent = raw_battery;
    char batt_text[32];
    snprintf(batt_text, sizeof(batt_text), "%d%%", raw_battery);
    gtk_label_set_text(data->battery_label, batt_text);
    gtk_widget_queue_draw(data->battery_da);
    return TRUE;
}

// Updates lap number display
gboolean update_lap_number(AppData *data) {
    char lap_text[32];
    snprintf(lap_text, sizeof(lap_text), "#%d", get_lap_number());
    gtk_label_set_text(data->lap_label, lap_text);
    return TRUE;
}

// Adjusts temperature based on GPIO input and sets timeout
void adjust_temp(AppData *data, int delta) {
    data->current_temp += delta;
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

// Resets temperature to default after timeout
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

// Updates crew message display
static gboolean update_message(gpointer data) {
    AppData *app_data = (AppData *)data;
    const char *message = get_crew_message();
    gtk_label_set_text(app_data->crew_msg_label, message);
    return TRUE;
}

// Sets acknowledgment message when GPIO triggered
void acknowledge_message(AppData *data) {
    gtk_label_set_text(data->crew_msg_label, "Acknowledged");
}

// Handles GPIO events for temperature and acknowledgment
static gboolean gpio_event_handler(GIOChannel *source, GIOCondition condition, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    struct gpiod_line_event event;

    if (condition & G_IO_IN) {
        int fd = g_io_channel_unix_get_fd(source);
        if (gpiod_line_event_read_fd(fd, &event) == 0) {
            if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                if (fd == gpiod_line_event_get_fd(data->line_temp_up)) {
                    adjust_temp(data, 1);
                } else if (fd == gpiod_line_event_get_fd(data->line_temp_down)) {
                    adjust_temp(data, -1);
                } else if (fd == gpiod_line_event_get_fd(data->line_ack)) {
                    acknowledge_message(data);
                }
            }
        }
    }
    return G_SOURCE_CONTINUE;
}

// Initializes GPIO pins with pull-ups using pinctrl and libgpiod v1.6.3
static void setup_gpio(AppData *data) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "pinctrl set %d pu", GPIO_TEMP_UP);
    if (system(cmd) != 0) g_printerr("Failed to set pull-up on GPIO %d\n", GPIO_TEMP_UP);
    snprintf(cmd, sizeof(cmd), "pinctrl set %d pu", GPIO_TEMP_DOWN);
    if (system(cmd) != 0) g_printerr("Failed to set pull-up on GPIO %d\n", GPIO_TEMP_DOWN);
    snprintf(cmd, sizeof(cmd), "pinctrl set %d pu", GPIO_ACK);
    if (system(cmd) != 0) g_printerr("Failed to set pull-up on GPIO %d\n", GPIO_ACK);

    data->chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!data->chip) {
        g_printerr("Failed to open GPIO chip: %s\n", strerror(errno));
        return;
    }

    data->line_temp_up = gpiod_chip_get_line(data->chip, GPIO_TEMP_UP);
    data->line_temp_down = gpiod_chip_get_line(data->chip, GPIO_TEMP_DOWN);
    data->line_ack = gpiod_chip_get_line(data->chip, GPIO_ACK);

    if (!data->line_temp_up || !data->line_temp_down || !data->line_ack) {
        g_printerr("Failed to get GPIO lines\n");
        gpiod_chip_close(data->chip);
        return;
    }

    if (gpiod_line_request_falling_edge_events(data->line_temp_up, "temp_up") < 0 ||
        gpiod_line_request_falling_edge_events(data->line_temp_down, "temp_down") < 0 ||
        gpiod_line_request_falling_edge_events(data->line_ack, "ack") < 0) {
        g_printerr("Failed to request GPIO events: %s\n", strerror(errno));
        gpiod_chip_close(data->chip);
        return;
    }

    GIOChannel *channel_temp_up = g_io_channel_unix_new(gpiod_line_event_get_fd(data->line_temp_up));
    GIOChannel *channel_temp_down = g_io_channel_unix_new(gpiod_line_event_get_fd(data->line_temp_down));
    GIOChannel *channel_ack = g_io_channel_unix_new(gpiod_line_event_get_fd(data->line_ack));

    g_io_add_watch(channel_temp_up, G_IO_IN, gpio_event_handler, data);
    g_io_add_watch(channel_temp_down, G_IO_IN, gpio_event_handler, data);
    g_io_add_watch(channel_ack, G_IO_IN, gpio_event_handler, data);

    g_io_channel_unref(channel_temp_up);
    g_io_channel_unref(channel_temp_down);
    g_io_channel_unref(channel_ack);
}

// Draws battery level visualization
static gboolean draw_battery(GtkWidget *widget, cairo_t *cr, AppData *data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    int battery_width = 40 * GUI_SCALE_FACTOR;
    int battery_height = 200 * GUI_SCALE_FACTOR;
    int tip_width = 15 * GUI_SCALE_FACTOR;
    int tip_height = 5 * GUI_SCALE_FACTOR;
    int x = (width - battery_width) / 2;
    int y = (height - battery_height - tip_height) / 2;

    cairo_rectangle(cr, x + (battery_width - tip_width)/2, y, tip_width, tip_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    y += tip_height;
    cairo_rectangle(cr, x, y, battery_width, battery_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR);
    cairo_stroke(cr);

    double fill_height = battery_height * (data->battery_percent / 100.0);
    if (data->battery_percent > 50) {
        cairo_set_source_rgb(cr, 0.2, 0.7, 0.2);
    } else if (data->battery_percent > 20) {
        cairo_set_source_rgb(cr, 1.0, 0.8, 0.2);
    } else {
        cairo_set_source_rgb(cr, 1.0, 0.2, 0.2);
    }
    cairo_rectangle(cr, x + 2 * GUI_SCALE_FACTOR,
                   y + battery_height - fill_height + 2 * GUI_SCALE_FACTOR,
                   battery_width - 4 * GUI_SCALE_FACTOR,
                   fill_height - 4 * GUI_SCALE_FACTOR);
    cairo_fill(cr);
    return FALSE;
}

// Draws the efficiency gauge
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

    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 2 * GUI_SCALE_FACTOR);
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

    if (meter->h2_alarm) {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_set_line_width(cr, 4 * GUI_SCALE_FACTOR);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_stroke(cr);
    }
    return FALSE;
}

// Updates efficiency meter values and redraws
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

// Main function - initializes and runs the GUI
int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *box;
    GtkCssProvider *provider;
    AppData *data = g_new0(AppData, 1);

    gtk_init(&argc, &argv);
    srand(time(NULL));

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Lab Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 320 * GUI_SCALE_FACTOR, 240 * GUI_SCALE_FACTOR);
    gtk_window_fullscreen(GTK_WINDOW(window));

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15 * GUI_SCALE_FACTOR);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10 * GUI_SCALE_FACTOR);

    GtkWidget *battery_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(battery_da, 40 * GUI_SCALE_FACTOR, 100 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), battery_da, 0, 0, 1, 3);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 1);

    char lap_str[32];
    snprintf(lap_str, sizeof(lap_str), "#%d", get_lap_number());
    GtkWidget *lab_num = gtk_label_new(lap_str);
    gtk_widget_set_name(lab_num, "lap-label"); 

    GtkWidget *temp_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5 * GUI_SCALE_FACTOR);
    GtkWidget *temp_label = gtk_label_new("25°C");
    gtk_widget_set_name(temp_label, "temp-label"); 

    GtkWidget *efficiency_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(efficiency_drawing_area, 150 * GUI_SCALE_FACTOR, 150 * GUI_SCALE_FACTOR);

    gtk_box_pack_start(GTK_BOX(box), lab_num, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), temp_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), efficiency_drawing_area, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);

    GtkWidget *col2_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), col2_box, 2, 0, 1, 1);

    GtkWidget *battery_percent_label = gtk_label_new("");
    gtk_widget_set_name(battery_percent_label, "battery-label"); 
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

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10 * GUI_SCALE_FACTOR);
    gtk_grid_attach(GTK_GRID(grid), bottom_box, 0, 3, 3, 1);

    GtkWidget *msg_label = gtk_label_new("Crew Message");
    gtk_widget_set_name(msg_label, "crew-msg-label");
    gtk_box_pack_start(GTK_BOX(bottom_box), msg_label, TRUE, TRUE, 0);

    provider = gtk_css_provider_new();
    gchar *css = g_strdup_printf(
        "grid, window { background-color: white; }"
        "#speed-label { font-size: %dpx; font-weight: bold; margin: %dpx; }" 
        "label { font-size: %dpx; margin: %dpx; }" 
        "#current-eff-label { font-size: %dpx; font-weight: bold; color: red; margin: %dpx; }"
        "#average-eff-label { font-size: %dpx; font-weight: bold; color: green; margin: %dpx; }"
        "#crew-msg-label { font-size: %dpx; margin: %dpx; }"
        "#battery-label { font-size: %dpx; margin: %dpx; }"
        "#lap-label { font-size: %dpx; margin: %dpx; }"
        "#temp-label { font-size: %dpx; margin: %dpx; }",
        (int)(48 * GUI_SCALE_FACTOR), (int)(20 * GUI_SCALE_FACTOR),  
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR),   
        (int)(16 * GUI_SCALE_FACTOR), (int)(5 * GUI_SCALE_FACTOR)   
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
    data->battery_percent = get_battery();
    data->current_temp = get_temperature();
    data->temp_timeout_id = 0;

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(efficiency_drawing_area, "draw", G_CALLBACK(on_draw), &data->efficiency_meter);
    g_signal_connect(battery_da, "draw", G_CALLBACK(draw_battery), data);

    g_timeout_add(1000, (GSourceFunc)update_speed, data);
    g_timeout_add(500, (GSourceFunc)update_battery, data);
    g_timeout_add(3000, (GSourceFunc)update_message, data);
    g_timeout_add(1000, (GSourceFunc)update_lap_number, data);
    g_timeout_add(UPDATE_INTERVAL, update_efficiency, &data->efficiency_meter);

    g_signal_connect(window, "realize", G_CALLBACK(hide_cursor), NULL);

    setup_gpio(data);

    gtk_widget_show_all(window);
    gtk_main();

    if (data->line_temp_up) gpiod_line_release(data->line_temp_up);
    if (data->line_temp_down) gpiod_line_release(data->line_temp_down);
    if (data->line_ack) gpiod_line_release(data->line_ack);
    if (data->chip) gpiod_chip_close(data->chip);

    g_free(data);
    return 0;
}
