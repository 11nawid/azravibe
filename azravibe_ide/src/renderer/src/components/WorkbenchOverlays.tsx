import React, { useMemo, useState } from 'react'
import { AlertCircle, Boxes, Command, FileSearch, FolderSearch, Play, Search, Terminal, X } from 'lucide-react'
import { FileNode, useStore } from '../store/useStore'

const collectFiles = (nodes: FileNode[]): FileNode[] => {
  const files: FileNode[] = []
  for (const node of nodes) {
    if (node.isDirectory) {
      files.push(...collectFiles(node.children || []))
    } else {
      files.push(node)
    }
  }
  return files
}

const fileName = (path: string) => path.split('\\').pop() || path

function ModalShell({
  title,
  icon,
  onClose,
  children
}: {
  title: string
  icon: React.ReactNode
  onClose: () => void
  children: React.ReactNode
}) {
  return (
    <div className="fixed inset-0 z-[99998] flex items-start justify-center bg-black/35 pt-20" onMouseDown={onClose}>
      <div
        className="w-[min(720px,calc(100vw-32px))] overflow-hidden border border-editor bg-sidebar"
        onMouseDown={(event) => event.stopPropagation()}
      >
        <div className="flex items-center justify-between border-b border-editor px-4 py-2">
          <div className="flex items-center gap-2 text-xs font-bold text-txt">
            <span className="text-accent">{icon}</span>
            <span>{title}</span>
          </div>
          <button className="p-1 text-neutral-500 hover:bg-editor hover:text-txt" onClick={onClose}>
            <X size={14} />
          </button>
        </div>
        {children}
      </div>
    </div>
  )
}

export default function WorkbenchOverlays() {
  const {
    activePath,
    tabs,
    fileTree,
    diagnostics,
    showCommandPalette,
    showQuickOpen,
    showSearchPanel,
    showProblemsPanel,
    showOutlinePanel,
    autoSave,
    compilerPath,
    terminalOpen,
    sidebarOpen,
    setShowCommandPalette,
    setShowQuickOpen,
    setShowSearchPanel,
    setShowProblemsPanel,
    setShowOutlinePanel,
    setAutoSave,
    setTerminalOpen,
    setSidebarOpen,
    addTerminal,
    saveFile,
    openFile
  } = useStore()

  const [query, setQuery] = useState('')
  const [searchQuery, setSearchQuery] = useState('')
  const [replaceText, setReplaceText] = useState('')
  const [matches, setMatches] = useState<Array<{ path: string; line: number; preview: string }>>([])
  const [busy, setBusy] = useState(false)
  const files = useMemo(() => collectFiles(fileTree), [fileTree])
  const activeTab = tabs.find((tab) => tab.path === activePath)

  const closeAll = () => {
    setShowCommandPalette(false)
    setShowQuickOpen(false)
    setShowSearchPanel(false)
    setShowProblemsPanel(false)
    setShowOutlinePanel(false)
  }

  const commands = [
    {
      label: 'Save Active File',
      shortcut: 'Ctrl+S',
      icon: <FileSearch size={14} />,
      run: () => activePath && saveFile(activePath)
    },
    {
      label: 'Run Active File',
      shortcut: 'F5',
      icon: <Play size={14} />,
      run: async () => {
        if (!activePath) return
        const termId = useStore.getState().activeTerminalId || addTerminal()
        await saveFile(activePath)
        window.api.writeTerminal(termId, `${compilerPath} "${activePath}"\r`)
      }
    },
    { label: 'Quick Open File', shortcut: 'Ctrl+P', icon: <FileSearch size={14} />, run: () => setShowQuickOpen(true) },
    { label: 'Search Workspace', shortcut: 'Ctrl+Shift+F', icon: <Search size={14} />, run: () => setShowSearchPanel(true) },
    { label: 'Problems', shortcut: 'Ctrl+Shift+M', icon: <AlertCircle size={14} />, run: () => setShowProblemsPanel(true) },
    { label: 'Outline', shortcut: 'Ctrl+Shift+O', icon: <Boxes size={14} />, run: () => setShowOutlinePanel(true) },
    { label: terminalOpen ? 'Hide Terminal' : 'Show Terminal', shortcut: 'Ctrl+`', icon: <Terminal size={14} />, run: () => setTerminalOpen(!terminalOpen) },
    { label: sidebarOpen ? 'Hide Sidebar' : 'Show Sidebar', shortcut: 'Ctrl+B', icon: <FolderSearch size={14} />, run: () => setSidebarOpen(!sidebarOpen) },
    { label: autoSave ? 'Disable Auto Save' : 'Enable Auto Save', shortcut: '', icon: <Command size={14} />, run: () => setAutoSave(!autoSave) }
  ]

  const filteredCommands = commands.filter((item) => item.label.toLowerCase().includes(query.toLowerCase()))
  const filteredFiles = files.filter((item) => item.path.toLowerCase().includes(query.toLowerCase())).slice(0, 60)

  const runWorkspaceSearch = async () => {
    if (!searchQuery.trim()) {
      setMatches([])
      return
    }
    setBusy(true)
    const nextMatches: Array<{ path: string; line: number; preview: string }> = []
    for (const item of files) {
      try {
        const content = await window.api.readFile(item.path)
        content.split(/\r?\n/).forEach((line, index) => {
          if (line.toLowerCase().includes(searchQuery.toLowerCase())) {
            nextMatches.push({ path: item.path, line: index + 1, preview: line.trim() })
          }
        })
      } catch {}
    }
    setMatches(nextMatches.slice(0, 300))
    setBusy(false)
  }

  const replaceWorkspaceMatches = async () => {
    if (!searchQuery.trim() || matches.length === 0) return
    setBusy(true)
    const touched = new Set(matches.map((match) => match.path))
    const confirmed = window.confirm(`Replace ${matches.length} matches in ${touched.size} files?`)
    if (!confirmed) {
      setBusy(false)
      return
    }
    const escaped = searchQuery.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
    const matcher = new RegExp(escaped, 'gi')
    for (const path of touched) {
      try {
        const content = await window.api.readFile(path)
        await window.api.writeFile(path, content.replace(matcher, replaceText))
      } catch {}
    }
    await useStore.getState().refreshFileTree()
    await runWorkspaceSearch()
    setBusy(false)
  }

  const outlineItems = useMemo(() => {
    if (!activeTab) return []
    return activeTab.content
      .split(/\r?\n/)
      .map((line, index) => ({ line: index + 1, text: line.trim() }))
      .filter((item) => /^(تابع|function|class|اگر|for|while)\b/.test(item.text))
      .slice(0, 80)
  }, [activeTab])

  return (
    <>
      {showCommandPalette && (
        <ModalShell title="Command Palette" icon={<Command size={15} />} onClose={closeAll}>
          <input
            autoFocus
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Type a command..."
            className="w-full border-b border-editor bg-bg px-4 py-3 text-sm text-txt outline-none placeholder:text-neutral-500"
          />
          <div className="max-h-[420px] overflow-y-auto">
            {filteredCommands.map((item) => (
              <button
                key={item.label}
                className="flex w-full items-center justify-between border-b border-editor px-3 py-2 text-left text-xs text-txt hover:bg-editor"
                onClick={() => {
                  void item.run()
                  closeAll()
                }}
              >
                <span className="flex items-center gap-2">{item.icon}{item.label}</span>
                <span className="text-[10px] text-neutral-500">{item.shortcut}</span>
              </button>
            ))}
          </div>
        </ModalShell>
      )}

      {showQuickOpen && (
        <ModalShell title="Quick Open" icon={<FileSearch size={15} />} onClose={closeAll}>
          <input
            autoFocus
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Search files by name or path..."
            className="w-full border-b border-editor bg-bg px-4 py-3 text-sm text-txt outline-none placeholder:text-neutral-500"
          />
          <div className="max-h-[420px] overflow-y-auto">
            {filteredFiles.map((item) => (
              <button
                key={item.path}
                className="block w-full border-b border-editor px-3 py-2 text-left text-xs hover:bg-editor"
                onClick={() => {
                  void openFile(item.path, item.name)
                  closeAll()
                }}
              >
                <div className="font-semibold text-txt">{item.name}</div>
                <div className="truncate text-[10px] text-neutral-500">{item.path}</div>
              </button>
            ))}
          </div>
        </ModalShell>
      )}

      {showSearchPanel && (
        <ModalShell title="Workspace Search" icon={<Search size={15} />} onClose={closeAll}>
          <div className="grid gap-3 border-b border-editor bg-bg p-3 sm:grid-cols-[1fr_1fr_auto_auto]">
            <input value={searchQuery} onChange={(event) => setSearchQuery(event.target.value)} placeholder="Find text" className="border-b border-editor bg-transparent px-1 py-2 text-xs text-txt outline-none focus:border-accent" />
            <input value={replaceText} onChange={(event) => setReplaceText(event.target.value)} placeholder="Replace with" className="border-b border-editor bg-transparent px-1 py-2 text-xs text-txt outline-none focus:border-accent" />
            <button onClick={runWorkspaceSearch} className="border border-accent px-3 py-2 text-xs font-semibold text-accent hover:bg-accent/10">{busy ? 'Searching...' : 'Find'}</button>
            <button onClick={replaceWorkspaceMatches} className="border border-editor px-3 py-2 text-xs font-semibold text-neutral-300 hover:bg-editor hover:text-txt">Replace</button>
          </div>
          <div className="max-h-[420px] overflow-y-auto">
            {matches.map((match) => (
              <button key={`${match.path}:${match.line}:${match.preview}`} className="block w-full border-b border-editor px-3 py-2 text-left text-xs hover:bg-editor" onClick={() => void openFile(match.path, fileName(match.path))}>
                <div className="text-txt">{fileName(match.path)}:{match.line}</div>
                <div className="truncate text-[10px] text-neutral-500">{match.preview}</div>
              </button>
            ))}
          </div>
        </ModalShell>
      )}

      {showProblemsPanel && (
        <ModalShell title="Problems" icon={<AlertCircle size={15} />} onClose={closeAll}>
          <div className="max-h-[420px] overflow-y-auto">
            {diagnostics.length === 0 && <div className="px-3 py-8 text-center text-xs text-neutral-500">No problems detected.</div>}
            {diagnostics.map((item, index) => (
              <button key={`${item.path}:${item.line}:${item.col}:${index}`} className="block w-full border-b border-editor px-3 py-2 text-left text-xs hover:bg-editor" onClick={() => void openFile(item.path, fileName(item.path))}>
                <div className="font-semibold text-red-400">{item.message}</div>
                <div className="truncate text-[10px] text-neutral-500">{fileName(item.path)}:{item.line}:{item.col}</div>
              </button>
            ))}
          </div>
        </ModalShell>
      )}

      {showOutlinePanel && (
        <ModalShell title="Outline" icon={<Boxes size={15} />} onClose={closeAll}>
          <div className="max-h-[420px] overflow-y-auto">
            {outlineItems.length === 0 && <div className="px-3 py-8 text-center text-xs text-neutral-500">No symbols in active file.</div>}
            {outlineItems.map((item) => (
              <div key={`${item.line}:${item.text}`} className="border-b border-editor px-3 py-2 text-xs">
                <div className="text-txt">{item.text}</div>
                <div className="text-[10px] text-neutral-500">Line {item.line}</div>
              </div>
            ))}
          </div>
        </ModalShell>
      )}
    </>
  )
}
