#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
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
static pthread_t thread_recording;
static pthread_t thread_ffmpeg_processing;
ScreenRect record_rect;
int fps = 50;


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
GtkWidget* timer_label;
GtkWidget* fps_entry;
GtkWidget* drop_down;
GtkWidget* record_button;

time_t start_record_time;
struct tm* time_info;


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

bool modify_time() {
    time_t cur_time;
    time(&cur_time);
    cur_time -= start_record_time;
    time_info = localtime(&cur_time);

    char time_str[5];
    sprintf(time_str,"%02d:%02d",time_info->tm_min,time_info->tm_sec);    

    gtk_label_set_text( timer_label,time_str);


    // its not intended to last more then a hour
    if(time_info->tm_hour >= 1) { 
        is_recording = false;
    }

    if(is_recording) {return true;}
    return false;
}
void* record() {
    char file_name[256];

    
    while (is_recording){
        sprintf(file_name,"output/%d.png",cur_img_idx);
        take_screenshot(file_name,record_rect);
        cur_img_idx += 1;

        g_usleep((int)((1.f / fps) * 1000) * 1000);
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
void make_gif(char* fp) {
    // just run the ./make_gif.sh with the fps argument
    char cmd_str[64];
    sprintf(cmd_str,"./make_gif.sh %s %d",fp,fps);   
    system(cmd_str); 
}

char* get_save_folder_path() {
    GtkWidget* file_broswer;
    file_broswer = gtk_file_chooser_dialog_new("Select Save Folder",window,GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,"Select",GTK_RESPONSE_APPLY,NULL);
    gtk_dialog_run(file_broswer);
    char* fp;
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(file_broswer);
    fp = gtk_file_chooser_get_current_folder(file_broswer);
    gtk_widget_destroy(file_broswer);
    return fp;
}

void on_record(GtkButton* widget ,GtkEntry* entry) {
    update_record_rect(-1,-1);


    if(!is_recording) {
        is_recording = true;

        // changing styles
        gtk_button_set_label(widget,"Recording...");
        gtk_widget_set_name(widget, "record_btn_recording");
        gtk_widget_set_name(timer_label,"");

        // record timer
        time(&start_record_time);
        g_timeout_add_seconds(1,modify_time,NULL);

        // disable those widgets
        gtk_widget_set_sensitive(fps_entry,false);
        gtk_widget_set_sensitive(drop_down,false);

        char* fps_text =  gtk_entry_get_text(fps_entry);
        fps = atoi(fps_text);

        // disable window resizing 
        gtk_window_set_resizable(window,false);
        pthread_create(&thread_recording,NULL,record,NULL);
    } else {
        is_recording = false;

        // changing styles
        gtk_button_set_label(widget,"Record!");
        gtk_widget_set_name(widget, "");
        gtk_widget_set_name(timer_label,"timer_label_not_recording");

        // enable those widgets
        gtk_widget_set_sensitive(fps_entry,true);
        gtk_widget_set_sensitive(drop_down,true);
        
        // enable window resizing
        gtk_window_set_resizable(window,true);



        char* fp = get_save_folder_path();
        pthread_create(&thread_ffmpeg_processing,NULL,make_gif,fp);
    }

}



gboolean on_key_press(GtkWidget* widget,GdkEventKey* event) {
    if(!is_recording && event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
    }
    if(event->keyval == GDK_KEY_space && event->type == GDK_KEY_RELEASE) {
        gtk_button_clicked(record_button);
    }

    return FALSE;
}
gboolean on_window_move(GtkWindow* window, GdkEvent* event) {
    update_record_rect(event->configure.x,event->configure.y);
    return False;
}

int main(int argc, char** argv) {
    XInitThreads();
    gtk_init(&argc,&argv);
    load_css();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size((GtkWindow*)window,800,600);
    gtk_widget_set_name(window,"window");
    gtk_window_set_title(window,"peak");

    g_signal_connect(GTK_WIDGET(window),"destory",G_CALLBACK(gtk_main_quit),NULL);
    g_signal_connect(GTK_WIDGET(window),"configure-event",G_CALLBACK(on_window_move),NULL);
    g_signal_connect(GTK_WIDGET(window),"key_press_event",G_CALLBACK(on_key_press),NULL);


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
        printf("Failed to init, no composition\n");
        return 0;
    }



    // global container for layout stuff
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(window),box);

    top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_widget_set_name(top_box, "top_bar");
    gtk_box_pack_start(box,top_box,false,false,0);



    // record btn
    record_button = gtk_button_new_with_label("Record!");
    g_signal_connect(GTK_WIDGET(record_button),"clicked",G_CALLBACK(on_record),NULL);
    GtkStyleContext* btn_style_ctx = gtk_widget_get_style_context(record_button);
    gtk_style_context_add_class(btn_style_ctx,"record_btn");
    gtk_box_pack_start(top_box,record_button,false,false,0);



    // drop down menu to select output file type
    GtkTreeModel* drop_down_model = gtk_list_store_new(1,G_TYPE_STRING);
    GtkTreeIter drop_down_model_iter;
    gtk_list_store_append(drop_down_model,&drop_down_model_iter);
    gtk_list_store_set(drop_down_model,&drop_down_model_iter,0,"GIF",-1);
    gtk_list_store_append(drop_down_model,&drop_down_model_iter);
    gtk_list_store_set(drop_down_model,&drop_down_model_iter,0,"APNG",-1);
    drop_down = gtk_combo_box_new_with_model(drop_down_model);
    g_object_unref(drop_down_model);
    gtk_combo_box_set_active(GTK_COMBO_BOX(drop_down),0);
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (drop_down), renderer, false);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (drop_down), renderer, "text", 0, NULL);    
    gtk_box_pack_end(top_box,drop_down,false,false,0);



    // fps entry
    fps_entry = gtk_entry_new();
    gtk_entry_set_width_chars(fps_entry,2);
    gtk_entry_set_max_width_chars(fps_entry,3);
    GtkWidget* fps_label = gtk_label_new("FPS:");
    gtk_entry_set_text(fps_entry,"50");
    GtkWidget* fps_container = gtk_grid_new();
    gtk_grid_attach(fps_container,fps_entry,1,0,1,1);
    gtk_grid_attach(fps_container,fps_label,0,0,1,1);
    gtk_grid_set_column_spacing(fps_container,10);
    gtk_box_pack_end(top_box,fps_container,false,false,0);

    

    //timer label
    timer_label = gtk_label_new("00:00");
    gtk_widget_set_name(timer_label,"timer_label_not_recording");
    gtk_box_pack_start(top_box,timer_label,false,false,20);



    
    gtk_widget_show_all(window);
    gdk_window_set_events(gtk_widget_get_window(GTK_WIDGET(window)),GDK_ALL_EVENTS_MASK);


    gtk_window_set_resizable((GtkWindow*)window,true);
    gtk_main();

    return 0;
}