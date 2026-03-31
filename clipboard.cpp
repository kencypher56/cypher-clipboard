#include "clipboard.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstring>

History g_history;

void History::add_entry(std::unique_ptr<ClipboardEntry> entry) {
    // Avoid duplicate of the most recent entry
    if (!entries.empty() && is_duplicate(*entry)) {
        return;
    }
    entries.push_front(std::move(entry));
    while (entries.size() > MAX_ENTRIES) {
        entries.pop_back();
    }
}

void History::clear() {
    entries.clear();
}

const std::deque<std::unique_ptr<ClipboardEntry>>& History::get_entries() const {
    return entries;
}

void History::remove_at(size_t index) {
    if (index < entries.size()) {
        auto it = entries.begin();
        std::advance(it, index);
        entries.erase(it);
    }
}

bool History::is_duplicate(const ClipboardEntry& new_entry) const {
    const auto& latest = entries.front();
    if (latest->type != new_entry.type) return false;
    
    switch (new_entry.type) {
        case EntryType::Text:
            return latest->text == new_entry.text;
        case EntryType::Files:
            return latest->uris == new_entry.uris;
        case EntryType::Image:
            // Not implemented; could compare pixel data or skip
            return false;
        case EntryType::Binary:
            return latest->binary == new_entry.binary;
        default:
            return false;
    }
}

static std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static std::string make_preview(const ClipboardEntry& entry) {
    switch (entry.type) {
        case EntryType::Text:
            if (entry.text.length() > 80)
                return entry.text.substr(0, 77) + "...";
            return entry.text;
        case EntryType::Image:
            return "[Image]";
        case EntryType::Files: {
            std::string preview = "[Files: ";
            for (size_t i = 0; i < entry.uris.size() && i < 3; ++i) {
                if (i) preview += ", ";
                preview += entry.uris[i];
            }
            if (entry.uris.size() > 3) preview += ", ...";
            preview += "]";
            return preview;
        }
        case EntryType::Binary:
            return "[Binary data]";
        default:
            return "[Unknown]";
    }
}

static void on_clipboard_received(GtkClipboard* clipboard, GtkSelectionData* data, gpointer user_data) {
    if (!data) return;
    
    auto entry = std::make_unique<ClipboardEntry>();
    entry->timestamp = get_timestamp();
    
    GdkAtom target = gtk_selection_data_get_target(data);
    const guchar* raw = gtk_selection_data_get_data(data);
    gint length = gtk_selection_data_get_length(data);
    
    if (target == gdk_atom_intern_static_string("UTF8_STRING") ||
        target == gdk_atom_intern_static_string("TEXT")) {
        if (raw && length > 0) {
            entry->type = EntryType::Text;
            entry->text = std::string(reinterpret_cast<const char*>(raw), length);
            entry->preview = make_preview(*entry);
            g_history.add_entry(std::move(entry));
            refresh_history_list(nullptr);
        }
    }
    else if (target == gdk_atom_intern_static_string("image/png") ||
             target == gdk_atom_intern_static_string("image/bmp") ||
             target == gdk_atom_intern_static_string("image/jpeg")) {
        if (raw && length > 0) {
            GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
            if (gdk_pixbuf_loader_write(loader, raw, length, nullptr)) {
                gdk_pixbuf_loader_close(loader, nullptr);
                GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
                if (pixbuf) {
                    entry->type = EntryType::Image;
                    entry->image = g_object_ref(pixbuf);
                    entry->preview = make_preview(*entry);
                    g_history.add_entry(std::move(entry));
                    refresh_history_list(nullptr);
                }
            }
            g_object_unref(loader);
        }
    }
    else if (target == gdk_atom_intern_static_string("text/uri-list")) {
        if (raw && length > 0) {
            entry->type = EntryType::Files;
            std::string uris_str(reinterpret_cast<const char*>(raw), length);
            std::istringstream iss(uris_str);
            std::string line;
            while (std::getline(iss, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (!line.empty())
                    entry->uris.push_back(line);
            }
            entry->preview = make_preview(*entry);
            g_history.add_entry(std::move(entry));
            refresh_history_list(nullptr);
        }
    }
    else if (raw && length > 0) {
        entry->type = EntryType::Binary;
        entry->binary.assign(raw, raw + length);
        entry->preview = make_preview(*entry);
        g_history.add_entry(std::move(entry));
        refresh_history_list(nullptr);
    }
}

void on_clipboard_owner_change(GtkClipboard* clipboard, GdkEvent* event, gpointer user_data) {
    gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("UTF8_STRING"),
                                   on_clipboard_received, nullptr);
    // Also request images and files asynchronously; we can add more if needed
}

void init_clipboard_monitor() {
    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    g_signal_connect(clipboard, "owner-change", G_CALLBACK(on_clipboard_owner_change), nullptr);
}