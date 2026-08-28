# Azravibe IDE User Guide

The Azravibe IDE is a desktop code editor built specifically for the Azravibe programming language, with full RTL (right-to-left) support and Persian text rendering.

---

## Technology

- **Electron 31** -- Desktop application framework
- **React 18** -- UI framework
- **TypeScript 5** -- Type-safe JavaScript
- **CodeMirror 6** -- Code editor engine
- **xterm.js** -- Terminal emulator
- **node-pty** -- Terminal backend
- **Tailwind CSS** -- Styling

---

## Installation

### Pre-built Release

Download the installer from the [Releases](https://github.com/azravibe/azravibe/releases) page. The IDE installs to your system like any standard application.

### Building from Source

**Prerequisites:** Node.js 18+ and npm

```bash
cd azravibe_ide

# Install dependencies
npm install

# Development mode
npm run dev

# Production build
npm run build

# Create distributable
npm run dist
```

The distributable installer is output to `dist/`.

---

## First Launch

When you first open the IDE, you will see an onboarding wizard:

1. **UI Language:** Choose between Farsi (full RTL) or English (LTR)
2. **Color Theme:** Choose from Azravibe Dark, Classic Dark, or Modern Light

These settings can be changed later in Settings.

---

## Creating a Project

1. Click **Open Workspace** (or press `Ctrl+O`)
2. Select or create a folder for your project
3. The file explorer sidebar appears with the folder contents
4. Create new `.azr` files with `Ctrl+N` or the sidebar button

---

## Editing Code

### Syntax Highlighting

Azravibe files (`.azr`) get full syntax highlighting:

| Token | Color |
|---|---|
| Keywords (`اگه`، `کار`، etc.) | Hot pink |
| Built-in functions (`بنویس`، `طول`، etc.) | Cyan |
| Strings | Green |
| Numbers | Yellow |
| Comments | Gray |
| Operators | Orange |
| Functions | Blue |
| Variables | Red |
| Types | Purple |
| Properties | Yellow-gold |

### Autocomplete

Type in an Azravibe file and the editor suggests keywords and built-in functions. The completions include all 46 keywords and 52 built-in functions.

### RTL Code Editing

When the language is set to Farsi, the editor renders code RTL. Per-line text direction is supported, so mixed LTR/RTL content displays correctly.

### Indentation

The editor uses 4-space indentation by default (configurable to 2 or 8 spaces). Indent guides are displayed as visual dotted lines.

### Minimap

A canvas-based minimap is displayed on the right side (configurable). It tokenizes code and renders colored blocks matching syntax highlighting.

---

## Running Code

### From the Editor

Press `F5` to save and run the active file. The command `{compilerPath} "{file}"` is sent to the integrated terminal.

### From the Terminal

The IDE includes an integrated terminal (toggle with `Ctrl+``).

```bash
azravibe myfile.azr
```

### Terminal Themes

The terminal supports three themes matching the IDE: Azravibe Dark, VS Code Dark, and Modern Light.

### Multiple Terminals

Click the `+` button in the terminal panel to create additional terminal instances.

---

## File Management

### File Explorer

The sidebar shows a recursive file tree with:

- **File icons:** `.azr` files get a cyan badge with Persian letter "آ"
- **Right-click context menu:** New File, New Folder, Rename, Delete, Copy, Cut, Paste, Duplicate, Copy Path, Open in Explorer
- **Drag and drop:** Move or copy files between directories
- **Inline creation:** Create files and folders directly in the tree

### Tab Management

- Open multiple files in tabs
- Drag tabs to reorder
- `Ctrl+Tab` / `Ctrl+Shift+Tab` to switch tabs
- `Ctrl+W` to close a tab
- `Ctrl+Shift+T` to reopen a closed tab
- Unsaved files show a dot indicator

### Saving

- `Ctrl+S` to save the active file
- Auto-save is available in Settings
- `Ctrl+N` to create a new file in the current workspace

---

## Panel System

### Bottom Panel (`Ctrl+J`)

Tabbed panel at the bottom with:
- **Terminal** -- Integrated terminal instances
- **Problems** -- Linting diagnostics
- **Output** -- Program output

### Editor Panels

Opened as editor tabs:
- **Settings** -- IDE configuration
- **Problems** -- Diagnostics list
- **Search** -- Workspace-wide search and replace
- **Outline** -- Code symbols from the active file
- **About** -- IDE information

---

## Search

### In-File Search

`Ctrl+F` opens the find panel in the editor. `Ctrl+H` opens find and replace.

### Workspace Search

`Ctrl+Shift+F` opens workspace-wide search:
- Search across all files in the workspace
- Case-sensitive toggle
- Results show filename, line, and preview
- Click a result to open the file at that location
- Replace across workspace with confirmation

---

## Diagnostics

The IDE performs basic static analysis on `.azr` files:

- **Bracket matching:** Tracks `(`, `[`, `{` and reports unmatched brackets
- **String literal validation:** Detects unclosed string literals
- Results appear in the Problems panel and the lint gutter

---

## Keyboard Shortcuts

### File Operations

| Shortcut | Action |
|---|---|
| `Ctrl+O` | Open workspace folder |
| `Ctrl+N` | Create new file |
| `Ctrl+S` | Save file |
| `Ctrl+W` | Close active tab |
| `Ctrl+Shift+T` | Reopen closed tab |

### Navigation

| Shortcut | Action |
|---|---|
| `Ctrl+Tab` | Next tab |
| `Ctrl+Shift+Tab` | Previous tab |
| `Ctrl+P` | Quick file open |
| `Ctrl+Shift+E` | Focus explorer |

### Editing

| Shortcut | Action |
|---|---|
| `Ctrl+Z` | Undo |
| `Ctrl+Shift+Z` | Redo |
| `Ctrl+/` | Toggle line comment |
| `Alt+Up/Down` | Move line up/down |
| `Shift+Alt+Up/Down` | Copy line up/down |
| `Ctrl+L` | Select current line |
| `Ctrl+Shift+K` | Delete line |
| `Ctrl+Enter` | Insert line below |
| `Ctrl+Shift+Enter` | Insert line above |

### View

| Shortcut | Action |
|---|---|
| `Ctrl+`` ` | Toggle terminal |
| `Ctrl+B` | Toggle sidebar |
| `Ctrl+J` | Toggle bottom panel |
| `Ctrl+Shift+P` | Command palette |
| `Ctrl+Shift+F` | Workspace search |
| `Ctrl+Shift+M` | Problems panel |
| `Ctrl+Shift+O` | Outline panel |
| `Ctrl+,` | Settings |
| `Ctrl+Scroll` | Zoom font size |

### Run

| Shortcut | Action |
|---|---|
| `F5` | Save and run active file |

### Find

| Shortcut | Action |
|---|---|
| `Ctrl+F` | Find in file |
| `Ctrl+H` | Replace in file |
| `Ctrl+D` | Add selection to next match |

---

## Settings

Access via `Ctrl+,` or the titlebar menu.

### Interface

| Setting | Options | Default |
|---|---|---|
| Language | Farsi / English | Farsi |
| Editor Direction | RTL / LTR | RTL |
| Color Theme | Azravibe / VS Code / Light | Azravibe |

### Editor

| Setting | Options | Default |
|---|---|---|
| Font Size | 12-24px | 14 |
| Font Family | Any monospace font | Vazirmatn, Consolas, monospace |
| Word Wrap | On / Off | On |
| Minimap | On / Off | On |
| Auto Save | On / Off | Off |
| Tab Size | 2 / 4 / 8 | 4 |
| Insert Spaces | On / Off | Off |
| Compiler Path | String | azravibe |

---

## Console Mode

The IDE can launch in a standalone Console mode focused on the REPL experience.

Launch with `?mode=console` URL parameter or from the titlebar menu.

Features:
- Chat-bubble style input/output
- Persian RTL rendering with Vazir Code font
- Status indicator showing REPL state
- New window, copy transcript, clear history
- Keyboard shortcuts: Enter (execute), Shift+Enter (new line)

---

## Splash Screen

On launch, a splash screen shows for at least 3 seconds while the workspace initializes.

---

## Planned IDE Features

The following are planned but not yet implemented:

- Language server protocol (LSP) support
- Advanced refactoring tools
- Git integration
- Extension/plugin system
- Debugger integration
- More diagnostics and code analysis
