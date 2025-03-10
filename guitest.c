#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    GtkProgressBar *battery_bar;
    GtkLabel *speed_label;
    GtkLabel *temp_label;
    GtkLabel *crew_msg_label;
} AppData;

int get_random_speed() {
    return (rand() % 51) + 10; // 10-60 km/h
}

gboolean update_speed(AppData *data) {
    int speed = get_random_speed();
    char speed_text[32];
    snprintf(speed_text, sizeof(speed_text), "%d km/h", speed);
    gtk_label_set_text(data->speed_label, speed_text);
    return TRUE;
}

gboolean update_battery(AppData *data) {
    int battery_percent = rand() % 101; // Random between 0-100
    double fraction = battery_percent / 100.0;
    gtk_progress_bar_set_fraction(data->battery_bar, fraction);
    return TRUE; 
}

void adjust_temp(GtkButton *btn, GtkLabel *label) {
    static int temp = 25;
    const gchar *lbl = gtk_button_get_label(btn);
    
    if(strcmp(lbl, "+") == 0) temp++;
    else if(temp > 0) temp--;
    
    char temp_text[16];
    snprintf(temp_text, sizeof(temp_text), "%d°C", temp);
    gtk_label_set_text(label, temp_text);
}

gboolean update_message(AppData *data) {
    static const char *messages[] = {
        "System Nominal",
        "Check Tire Pressure",
        "Monitor Battery Levels",
        "Efficiency Optimal",
        "All Systems Go"
    };
    static int idx = 0;
    
    gtk_label_set_text(data->crew_msg_label, messages[idx]);
    idx = (idx + 1) % 5;
    return TRUE;
}

void on_ok_clicked(GtkButton *btn, GtkLabel *msg_label) {
    gtk_label_set_text(msg_label, "Acknowledged");
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *box;
    GtkCssProvider *provider;
    AppData *data = g_new(AppData, 1);

    gtk_init(&argc, &argv);
    srand(time(NULL));

    // Main window setup
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Lab Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 240);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    // Battery progress bar
    GtkWidget *battery = gtk_progress_bar_new();
    int initial_batt = rand() % 101;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(battery), initial_batt / 100.0);
    gtk_widget_set_size_request(battery, 30, 200);
    gtk_grid_attach(GTK_GRID(grid), battery, 0, 0, 1, 3);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(battery), GTK_ORIENTATION_VERTICAL);

    // Middle column
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 1);

    GtkWidget *lab_num = gtk_label_new("#2");
    GtkWidget *temp_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *temp_label = gtk_label_new("25°C");
    GtkWidget *eff_label = gtk_label_new("75% Efficiency");

    gtk_box_pack_start(GTK_BOX(box), lab_num, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), temp_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), eff_label, FALSE, FALSE, 0);

    // Temperature controls
    GtkWidget *minus_btn = gtk_button_new_with_label("-");
    GtkWidget *plus_btn = gtk_button_new_with_label("+");

    gtk_box_pack_start(GTK_BOX(temp_box), minus_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp_box), plus_btn, FALSE, FALSE, 0);

    // Speed display
    GtkWidget *speed_label = gtk_label_new("50 km/h");
    gtk_widget_set_name(speed_label, "speed-label");
    gtk_grid_attach(GTK_GRID(grid), speed_label, 2, 0, 1, 1);

    // Bottom section
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_grid_attach(GTK_GRID(grid), bottom_box, 0, 3, 3, 1);

    GtkWidget *msg_label = gtk_label_new("Crew Message");
    GtkWidget *ok_btn = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(bottom_box), msg_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(bottom_box), ok_btn, FALSE, FALSE, 0);

    // CSS Styling
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "progressbar {"
        "   min-width: 30px;"
        "   min-height: 200px;"
        "   -gtk-outline-radius: 0px;"
        "   transform: translateY(100%) scaleY(-1);" 
        "   transform-origin: bottom;"
        "}"
        "progressbar trough {"
        "   background: #ddd;"
        "   border-radius: 3px;"
        "}"
        "progressbar progress {"
        "   background: #4CAF50;"
        "   min-height: 100%;"
        "}"
        "#speed-label {"
        "   font-size: 48px;"
        "   font-weight: bold;"
        "   margin: 20px;"
        "}"
        "label {"
        "   margin: 5px;"
        "}"
        "button {"
        "   padding: 5px 15px;"
        "}", -1, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Initialize app data
    data->battery_bar = GTK_PROGRESS_BAR(battery);
    data->speed_label = GTK_LABEL(speed_label);
    data->temp_label = GTK_LABEL(temp_label);
    data->crew_msg_label = GTK_LABEL(msg_label);

    // Connect signals
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ok_btn, "clicked", G_CALLBACK(on_ok_clicked), msg_label);
    g_signal_connect(minus_btn, "clicked", G_CALLBACK(adjust_temp), temp_label);
    g_signal_connect(plus_btn, "clicked", G_CALLBACK(adjust_temp), temp_label);

    // Set up timers
    g_timeout_add(1000, (GSourceFunc)update_speed, data);
    g_timeout_add(500, (GSourceFunc)update_battery, data); 
    g_timeout_add(3000, (GSourceFunc)update_message, data);

    // Show window
    gtk_widget_show_all(window);
    gtk_main();

    g_free(data);
    return 0;
}
