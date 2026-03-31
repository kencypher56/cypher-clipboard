<!--
  cypher-clipboard – Linux clipboard manager with history
  (c) 2026 kencypher
-->

<h1 align="center">
  📋 <strong>cypher‑clipboard</strong> 🐧
</h1>

<p align="center">
  <em>Never lose what you copied again – the clipboard history you've been missing on Linux.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++" />
  <img src="https://img.shields.io/badge/GTK-7C4DFF?style=for-the-badge&logo=gtk&logoColor=white" alt="GTK" />
  <img src="https://img.shields.io/badge/Wayland-FF9E0F?style=for-the-badge&logo=wayland&logoColor=white" alt="Wayland" />
  <img src="https://img.shields.io/badge/X11-0078D7?style=for-the-badge&logo=x.org&logoColor=white" alt="X11" />
</p>

---

## 🚀 What is this?

**cypher‑clipboard** is a lightweight, cross‑distribution clipboard manager for Linux.  
It automatically captures everything you copy or cut – **text, images, files, and even binary data** – and stores it in a scrollable history.  

Click any past item to **re‑copy** it, or use the **cut** option to move it out of history.  
If you ever miss the `Win + V` clipboard history from Windows, this is your new best friend.

---

## ✨ Features

| Icon | Feature |
|------|---------|
| 📝 | **Text** – plain and formatted |
| 🖼️ | **Images** – preview & restore |
| 📁 | **Files** – full file paths (URI‑list) |
| 🔢 | **Binary** – any raw clipboard content |
| ⏱️ | **History** – up to 100 entries (configurable) |
| 🧹 | **Duplicate avoidance** – repeated copies are ignored |
| 🔄 | **X11 + Wayland** – works out of the box on both |
| 🎨 | **Native GUI** – clean, resizable, uses system theme |
| ⚡ | **Lightweight** – minimal CPU/RAM footprint |

---

## 📦 Installation

### Prerequisites

You need **GTK+ 3** development libraries:

```bash
# Debian / Ubuntu
sudo apt update && sudo apt install libgtk-3-dev

# Fedora / RHEL
sudo dnf install gtk3-devel

# Arch Linux
sudo pacman -S gtk3
```

### Build from source

Clone the repository and compile:

```bash
git clone https://github.com/kencypher/cypher-clipboard.git
cd cypher-clipboard
g++ -std=c++17 *.cpp -o clipboard `pkg-config --cflags --libs gtk+-3.0`
```

That’s it – the binary `clipboard` is ready.

### Install system‑wide

```bash
# User install (recommended)
mkdir -p ~/.local/bin
cp clipboard ~/.local/bin/

# System install (requires sudo)
sudo cp clipboard /usr/local/bin/
```

Make sure `~/.local/bin` is in your `PATH` (add it to `~/.bashrc` if needed).

---

## 🖥️ Usage

Run the application from a terminal:

```bash
clipboard
```

A window will appear showing your clipboard history.  
From now on, every time you copy (`Ctrl+C`) or cut (`Ctrl+X`), the new content is added to the top of the list.

- **Click** any entry → copies it back to the clipboard.
- **Right‑click** (or select and press **Cut Selected**) → copies and removes the entry (simulates a cut).
- **Clear** → wipes the entire history.

The application stays in the foreground – close it when you don’t need it.

### 🧠 Keyboard shortcut (coming soon)

A future version will allow opening the GUI with a global shortcut (e.g., `Ctrl+Shift+V`).  
For now, you can bind the `clipboard` command to a custom shortcut using your desktop environment’s settings.

---

## ⚙️ Configuration

Currently, configuration is minimal:

- **History size** – defined as `MAX_ENTRIES` in `clipboard.h` (default 100). Change and recompile to adjust.
- **Persistent history** – not yet implemented (planned).

---

## 🛠️ Development

### Project structure

```
cypher-clipboard/
├── main.cpp         # GUI & application entry
├── clipboard.cpp    # Monitoring & history management
├── copy.cpp         # Copy‑back logic
├── cut.cpp          # Cut logic
├── clipboard.h      # Shared definitions
└── README.md
```

### Contributing

Contributions are welcome!  
- Fork the repo, create a feature branch, and open a pull request.
- For major changes, please open an issue first to discuss.

---

## 📄 License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**kencypher**  
- GitHub: [@kencypher](https://github.com/kencypher56)

---

<p align="center">
  <strong>Enjoy your clipboard history – the Linux way!</strong> 🐧📋✨
</p>
