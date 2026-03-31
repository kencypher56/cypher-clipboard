#include "clipboard.h"

void copy_entry_to_clipboard(size_t index) {
    const auto& entries = g_history.get_entries();
    if (index >= entries.size()) return;
    
    const auto& entry = entries[index];
    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    
    switch (entry->type) {
        case EntryType::Text:
            gtk_clipboard_set_text(clipboard, entry->text.c_str(), entry->text.length());
            break;
        case EntryType::Image:
            if (entry->image)
                gtk_clipboard_set_image(clipboard, entry->image);
            break;
        case EntryType::Files: {
            std::string uri_list;
            for (const auto& uri : entry->uris)
                uri_list += uri + "\n";
            gtk_clipboard_set_text(clipboard, uri_list.c_str(), uri_list.length());
            break;
        }
        default:
            // Binary: not supported by GTK directly
            break;
    }
}