#include "clipboard.h"
#include <gtk/gtk.h>
#include <iostream>

static GtkWidget* history_list = nullptr;
static GtkWidget* preview_text_view = nullptr;
static GtkWidget* preview_image = nullptr;

static void on_row_activated(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    guint index = gtk_list_box_row_get_index(row);
    copy_entry_to_clipboard(index);
}

static void on_cut_button_clicked(GtkButton* button, gpointer user_data) {
    GtkListBoxRow* selected = gtk_list_box_get_selected_row(GTK_LIST_BOX(history_list));
    if (selected) {
        guint index = gtk_list_box_row_get_index(selected);
        cut_entry(index);
        refresh_history_list(GTK_LIST_BOX(history_list));
    }
}

static void on_clear_button_clicked(GtkButton* button, gpointer user_data) {
    g_history.clear();
    refresh_history_list(GTK_LIST_BOX(history_list));
}

static void on_row_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    if (!row) {
        gtk_widget_hide(preview_image);
        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
        gtk_text_buffer_set_text(buffer, "No selection", -1);
        return;
    }
    
    guint index = gtk_list_box_row_get_index(row);
    const auto& entries = g_history.get_entries();
    if (index >= entries.size()) return;
    
    const auto& entry = entries[index];
    
    // Update preview based on type
    switch (entry->type) {
        case EntryType::Text:
            gtk_widget_hide(preview_image);
            {
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, entry->text.c_str(), -1);
            }
            break;
        case EntryType::Image:
            if (entry->image) {
                gtk_widget_show(preview_image);
                // Scale image to fit in preview area while maintaining aspect ratio
                GtkAllocation alloc;
                gtk_widget_get_allocation(preview_image, &alloc);
                int width = alloc.width, height = alloc.height;
                if (width > 0 && height > 0) {
                    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(entry->image, width, height, GDK_INTERP_BILINEAR);
                    gtk_image_set_from_pixbuf(GTK_IMAGE(preview_image), scaled);
                    g_object_unref(scaled);
                } else {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(preview_image), entry->image);
                }
                // Clear text view
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, "", -1);
            } else {
                gtk_widget_hide(preview_image);
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, "Image data unavailable", -1);
            }
            break;
        case EntryType::Files:
            gtk_widget_hide(preview_image);
            {
                std::string text;
                for (const auto& uri : entry->uris) {
                    text += uri + "\n";
                }
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, text.c_str(), -1);
            }
            break;
        case EntryType::Binary:
            gtk_widget_hide(preview_image);
            {
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, "Binary data (not shown)", -1);
            }
            break;
        default:
            gtk_widget_hide(preview_image);
            {
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview_text_view));
                gtk_text_buffer_set_text(buffer, "Unknown type", -1);
            }
            break;
    }
}

static GtkWidget* create_row_widget(const ClipboardEntry& entry) {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_start(hbox, 5);
    gtk_widget_set_margin_end(hbox, 5);
    gtk_widget_set_margin_top(hbox, 2);
    gtk_widget_set_margin_bottom(hbox, 2);
    
    const char* icon_name = "text-x-generic";
    switch (entry.type) {
        case EntryType::Text: icon_name = "text-x-generic"; break;
        case EntryType::Image: icon_name = "image-x-generic"; break;
        case EntryType::Files: icon_name = "folder"; break;
        case EntryType::Binary: icon_name = "application-x-executable"; break;
        default: break;
    }
    GtkWidget* icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
    
    // Only show preview text (no timestamp)
    GtkWidget* preview_label = gtk_label_new(entry.preview.c_str());
    gtk_widget_set_halign(preview_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(preview_label), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start(GTK_BOX(hbox), preview_label, TRUE, TRUE, 0);
    
    return hbox;
}

void refresh_history_list(GtkListBox* list_box) {
    if (!history_list) return;
    
    // Clear existing rows
    gtk_container_foreach(GTK_CONTAINER(history_list), (GtkCallback)gtk_widget_destroy, nullptr);
    
    // Add new rows
    const auto& entries = g_history.get_entries();
    for (const auto& entry : entries) {
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* widget = create_row_widget(*entry);
        gtk_container_add(GTK_CONTAINER(row), widget);
        gtk_list_box_insert(GTK_LIST_BOX(history_list), row, -1);
        gtk_widget_show_all(row);
    }
}

void create_main_window() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Clipboard Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Toolbar
    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget* clear_btn = gtk_button_new_with_label("Clear");
    GtkWidget* cut_btn = gtk_button_new_with_label("Cut Selected");
    gtk_box_pack_start(GTK_BOX(toolbar), clear_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), cut_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    // Paned for history and preview
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    
    // History list (scrollable)
    history_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(history_list), GTK_SELECTION_BROWSE);
    g_signal_connect(history_list, "row-activated", G_CALLBACK(on_row_activated), nullptr);
    g_signal_connect(history_list, "row-selected", G_CALLBACK(on_row_selected), nullptr);
    GtkWidget* scroll_list = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_list), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll_list), history_list);
    gtk_paned_pack1(GTK_PANED(paned), scroll_list, TRUE, FALSE);
    
    // Preview area: text view + image container
    GtkWidget* preview_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    preview_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(preview_text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(preview_text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(preview_text_view), FALSE);
    GtkWidget* scroll_text = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_text), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll_text), preview_text_view);
    gtk_box_pack_start(GTK_BOX(preview_vbox), scroll_text, TRUE, TRUE, 0);
    
    preview_image = gtk_image_new();
    gtk_widget_set_size_request(preview_image, 200, 200);
    gtk_box_pack_start(GTK_BOX(preview_vbox), preview_image, FALSE, FALSE, 0);
    gtk_widget_hide(preview_image);
    
    GtkWidget* scroll_preview = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_preview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll_preview), preview_vbox);
    gtk_paned_pack2(GTK_PANED(paned), scroll_preview, TRUE, FALSE);
    
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_button_clicked), nullptr);
    g_signal_connect(cut_btn, "clicked", G_CALLBACK(on_cut_button_clicked), nullptr);
    
    gtk_widget_show_all(window);
    
    // Initial fill
    refresh_history_list(GTK_LIST_BOX(history_list));
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    init_clipboard_monitor();
    create_main_window();
    gtk_main();
    return 0;
}