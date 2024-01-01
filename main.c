#include <stdio.h>
#include <stdbool.h>
#include <gtk-3.0/gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_write.h"

// if the border of your window manager is getting into the img capture just
// offset it here
// this is for my i3 wm
static const int x_offset_left = 2;
static const int x_offset_right = 0;
static const int y_offset_top = 0;
static const int y_offset_bottom = 0;



GtkWidget* window;
GtkWidget* box;
GtkWidget* top_box;



void on_record() {
    int top_box_height = gtk_widget_get_allocated_height(top_box);


    int w , h;
    w = gtk_widget_get_allocated_width(window) - x_offset_right;
    h = gtk_widget_get_allocated_height(window) - y_offset_bottom - top_box_height;

    int x , y;
    gtk_window_get_position((GtkWindow*)window,&x,&y);
    y += top_box_height + y_offset_top;
    x += x_offset_left;

}
void on_draw(GtkWidget* widget,cairo_t* cr,gpointer* data) {
}

int main(int argc, char** argv) {
    gtk_init(&argc,&argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size((GtkWindow*)window,800,600);
    gtk_window_set_resizable((GtkWindow*)window,false);

    GdkScreen *screen;
    GdkVisual *visual;
    gtk_widget_set_app_paintable(window, true);
    screen = gdk_screen_get_default();
    visual = gdk_screen_get_rgba_visual(screen);

    if(visual != NULL && gdk_screen_is_composited(screen)) {
        gtk_widget_set_visual(window,visual);
    } else {
        printf("Failed to init, no composition");
        return 0;
    }


    g_signal_connect(GTK_WIDGET(window),"destory",G_CALLBACK(gtk_main_quit),NULL);



    // global container for layout stuff
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);    
    gtk_container_add(GTK_CONTAINER(window),box);

    // top container
    top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    GdkRGBA color = {1,1,1,1};
    gtk_widget_override_background_color(top_box,GTK_STATE_FLAG_NORMAL,&color);



    GtkWidget* button = gtk_button_new_with_label("Record!");
    g_signal_connect(GTK_WIDGET(button),"clicked",G_CALLBACK(on_record),NULL);
    gtk_box_pack_start((GtkBox*)top_box,button,false,true,0);



    gtk_box_pack_start((GtkBox*)box,top_box,false,true,0);

    GtkWidget* canvas = gtk_drawing_area_new();
    gtk_widget_set_app_paintable(canvas, true);

    g_signal_connect(GTK_WIDGET(canvas),"draw",G_CALLBACK(on_draw),NULL);
    gtk_box_pack_start((GtkBox*)box,canvas,true,true,0);




    
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}