#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <gtk-3.0/gtk/gtk.h>
#include <X11/Xlib.h>


// types
typedef struct ScreenRect {
    int x ,y , w , h;
} ScreenRect;





static const char* css_fp = "style.css";
static bool is_recording = false;
static int cur_img_idx = 0;
static pthread_t recording_thread;
ScreenRect record_rect;


// if the border of your window manager is getting into the img capture just
// offset it here
// this is for my i3 wm
static const int x_offset_left = 2;
static const int x_offset_right = 2;
static const int y_offset_top = 0;
static const int y_offset_bottom = 0;



GtkWidget* window;
GtkWidget* box;
GtkWidget* top_box;

void load_css() {
    // load css to use it for styling
    // gtk uses css for some reason
    
    GtkCssProvider* css_provider;
    css_provider = gtk_css_provider_new();

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),css_provider,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    //TODO: i should take the error and handle it, instead of NULL there
    gtk_css_provider_load_from_file(css_provider,g_file_new_for_path(css_fp),NULL);
    g_object_unref(css_provider);
}


void take_screenshot(char* fp,ScreenRect rect) {
    GdkPixbuf* buf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,rect.w,rect.h);
    buf = gdk_pixbuf_get_from_window(gdk_get_default_root_window(),rect.x,rect.y,rect.w,rect.h);
    gdk_pixbuf_save(buf,fp,"png",NULL,NULL);
}
void* record() {
    char file_name[256];
    while (is_recording){
        sprintf(file_name,"output/%d.png",cur_img_idx);
        take_screenshot(file_name,record_rect);
        cur_img_idx += 1;
    }

    pthread_exit(EXIT_SUCCESS);
}

void update_record_rect(int x , int y) {
    int top_box_height = gtk_widget_get_allocated_height(top_box);

    record_rect.w = gtk_widget_get_allocated_width(window) - x_offset_right;
    record_rect.h = gtk_widget_get_allocated_height(window) - y_offset_bottom - top_box_height;


    if(x == -1 && y == -1) {
        gtk_window_get_position((GtkWindow*)window,&record_rect.x,&record_rect.y);
    } else {
        record_rect.x = x;
        record_rect.y = y;
    }

    record_rect.x += x_offset_left;
    record_rect.y += top_box_height + y_offset_top;
}

void on_record(GtkButton* widget ,GtkEntry* entry) {
    update_record_rect(-1,-1);



    if(!is_recording) {
        is_recording = true;

        gtk_button_set_label(widget,"Recording...");
        gtk_widget_set_name(widget, "btn_recording");



        // disable window resizing 
        gtk_window_set_resizable(window,false);

        pthread_create(&recording_thread,NULL,record,NULL);
    } else {
        is_recording = false;

        gtk_button_set_label(widget,"Record!");
        gtk_widget_set_name(widget, "btn_not_recording");
    }

}
void on_draw(GtkWidget* widget,cairo_t* cr,gpointer* data) {
}



void on_window_resize(GtkWindow* window, GdkEvent* event) {
    update_record_rect(event->configure.x,event->configure.y);
}

int main(int argc, char** argv) {
    XInitThreads();
    gtk_init(&argc,&argv);
    load_css();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size((GtkWindow*)window,800,600);
    g_signal_connect(GTK_WIDGET(window),"destory",G_CALLBACK(gtk_main_quit),NULL);
    g_signal_connect(GTK_WIDGET(window),"configure-event",G_CALLBACK(on_window_resize),NULL);

    // disable resizing window to stop window managers from making it fullscreen i3 does that 
    // dont worry i set later to true after showing the window
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





    // global container for layout stuff
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);    
    gtk_container_add(GTK_CONTAINER(window),box);

    // top container
    top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_widget_set_name(top_box, "top_bar");



    GtkWidget* button = gtk_button_new_with_label("Record!");


    g_signal_connect(GTK_WIDGET(button),"clicked",G_CALLBACK(on_record),NULL);
    gtk_box_pack_start((GtkBox*)top_box,button,false,true,0);

    gtk_box_pack_start((GtkBox*)box,top_box,false,true,0);



    // maybe use canvas to draw stuff ??
    GtkWidget* canvas = gtk_drawing_area_new();
    gtk_widget_set_app_paintable(canvas, true);
    g_signal_connect(GTK_WIDGET(canvas),"draw",G_CALLBACK(on_draw),NULL);
    gtk_box_pack_start((GtkBox*)box,canvas,true,true,0);




    
    gtk_widget_show_all(window);
    gtk_window_set_resizable((GtkWindow*)window,true);
    gtk_main();

    return 0;
}