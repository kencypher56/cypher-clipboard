#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <chrono>

enum class EntryType {
    Text,
    Image,
    Files,
    Binary,
    Unknown
};

struct ClipboardEntry {
    EntryType type;
    std::string timestamp;  // human-readable (kept for internal use but not shown in GUI)
    std::string preview;    // short preview text
    std::string text;
    GdkPixbuf* image;
    std::vector<std::string> uris;
    std::vector<uint8_t> binary;
    
    ClipboardEntry() : type(EntryType::Unknown), image(nullptr) {}
    ~ClipboardEntry() { if (image) g_object_unref(image); }
    // Disable copy; enable move
    ClipboardEntry(const ClipboardEntry&) = delete;
    ClipboardEntry& operator=(const ClipboardEntry&) = delete;
    ClipboardEntry(ClipboardEntry&&) = default;
    ClipboardEntry& operator=(ClipboardEntry&&) = default;
};

class History {
public:
    static const size_t MAX_ENTRIES = 100;
    
    void add_entry(std::unique_ptr<ClipboardEntry> entry);
    void clear();
    const std::deque<std::unique_ptr<ClipboardEntry>>& get_entries() const;
    void remove_at(size_t index);
    bool is_duplicate(const ClipboardEntry& new_entry) const;
    
private:
    std::deque<std::unique_ptr<ClipboardEntry>> entries;
};

extern History g_history;

void init_clipboard_monitor();
void on_clipboard_owner_change(GtkClipboard* clipboard, GdkEvent* event, gpointer user_data);
void copy_entry_to_clipboard(size_t index);
void cut_entry(size_t index);
void create_main_window();
void refresh_history_list(GtkListBox* list_box);

#endif