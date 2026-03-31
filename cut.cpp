#include "clipboard.h"

void cut_entry(size_t index) {
    copy_entry_to_clipboard(index);
    g_history.remove_at(index);
    refresh_history_list(nullptr);
}