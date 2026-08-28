import { create } from 'zustand'
import {
  collectVisibleFiles,
  getFileName,
  ignoredTreeDirs,
  pathContains,
  persistTabsState,
  savedActivePath,
  savedAutoSave,
  savedCompilerPath,
  savedEditorRtl,
  savedFolder,
  savedFontFamily,
  savedFontSize,
  savedInsertSpaces,
  savedLang,
  savedMinimap,
  savedOnboarded,
  savedTabSize,
  savedTabs,
  savedTheme,
  savedWordWrap
} from './persistence'
import { createBottomPanelVisibilityToggle, createPanelTabToggle } from './panel-helpers'
import { FileNode, IdeState, TabFile, TerminalSession } from './types'

export type { FileNode, IdeState, TabFile, TerminalSession } from './types'

export const useStore = create<IdeState>((set, get) => ({
  currentFolder: savedFolder || null,
  fileTree: [],
  tabs: savedTabs,
  closedTabs: [],
  activePath: savedActivePath || null,
  selectedPath: savedActivePath || null,
  terminals: [],
  activeTerminalId: null,
  cursorLine: 1,
  cursorCol: 1,
  errorCount: 0,
  diagnostics: [],
  runOutput: [],
  onboarded: savedOnboarded,
  language: savedLang,
  theme: savedTheme,
  terminalOpen: false,
  sidebarOpen: true,
  showAboutModal: false,
  showSettingsModal: false,
  showCommandPalette: false,
  showQuickOpen: false,
  showSearchPanel: false,
  showProblemsPanel: false,
  showOutlinePanel: false,
  problemsPanelOpen: false,
  problemsPanelPosition: 'bottom',
  problemsPanelSize: 30,
  bottomPanelLayout: 'vertical',
  bottomPanelTabs: ['terminal'],
  activeBottomPanel: 'terminal',
  terminalPanelVisible: false,
  problemsPanelVisible: false,
  outputPanelVisible: false,
  wordWrap: savedWordWrap,
  fontSize: savedFontSize,
  minimap: savedMinimap,
  editorRtl: savedEditorRtl,
  autoSave: savedAutoSave,
  fontFamily: savedFontFamily,
  compilerPath: savedCompilerPath,
  tabSize: savedTabSize,
  insertSpaces: savedInsertSpaces,
  toastMessage: null,
  showToast: false,

  openFolder: async (dirPath) => {
    localStorage.setItem('azravibe_currentfolder', dirPath)
    localStorage.removeItem('azravibe_activepath')
    localStorage.setItem('azravibe_tabs', '[]')
    set({ currentFolder: dirPath, tabs: [], activePath: null, selectedPath: null })
    await get().refreshFileTree()
  },

  closeFolder: () => {
    localStorage.removeItem('azravibe_currentfolder')
    localStorage.removeItem('azravibe_activepath')
    localStorage.setItem('azravibe_tabs', '[]')
    set({ currentFolder: null, fileTree: [], tabs: [], activePath: null, selectedPath: null })
  },

  refreshFileTree: async () => {
    const { currentFolder } = get()
    if (!currentFolder) return

    const loadNode = async (dirPath: string): Promise<FileNode[]> => {
      const items = await window.api.readDir(dirPath)
      return items
        .filter((item) => !(item.isDirectory && ignoredTreeDirs.has(item.name)))
        .map((item) => ({
          name: item.name,
          path: item.path,
          isDirectory: item.isDirectory
        }))
    }

    const readRecursive = async (dirPath: string, depth = 0): Promise<FileNode[]> => {
      if (depth > 20) return []
      const list = await loadNode(dirPath)
      for (const node of list) {
        if (node.isDirectory) {
          node.children = await readRecursive(node.path, depth + 1)
        }
      }
      return list
    }

    try {
      const tree = await readRecursive(currentFolder)
      const visibleFiles = collectVisibleFiles(tree)
      const { tabs, activePath, selectedPath } = get()
      const validTabs = tabs.filter((tab) => tab.isPanel || visibleFiles.has(tab.path))
      const validActivePath = activePath && validTabs.some((tab) => tab.path === activePath)
        ? activePath
        : validTabs[0]?.path || null
      const validSelectedPath = selectedPath && (visibleFiles.has(selectedPath) || selectedPath === currentFolder)
        ? selectedPath
        : validActivePath

      localStorage.setItem('azravibe_tabs', JSON.stringify(validTabs))
      if (validActivePath) {
        localStorage.setItem('azravibe_activepath', validActivePath)
      } else {
        localStorage.removeItem('azravibe_activepath')
      }

      set({
        fileTree: tree,
        tabs: validTabs,
        activePath: validActivePath,
        selectedPath: validSelectedPath
      })
    } catch (error) {
      console.error(error)
    }
  },

  openFile: async (filePath, name) => {
    const { tabs } = get()
    const existing = tabs.find((tab) => tab.path === filePath)
    if (existing) {
      localStorage.setItem('azravibe_activepath', filePath)
      set({ activePath: filePath, selectedPath: filePath })
      return
    }

    try {
      const content = await window.api.readFile(filePath)
      const newTab: TabFile = {
        name,
        path: filePath,
        content,
        originalContent: content,
        isUnsaved: false
      }
      const nextTabs = [...tabs, newTab]
      localStorage.setItem('azravibe_activepath', filePath)
      localStorage.setItem('azravibe_tabs', JSON.stringify(nextTabs))
      set({
        tabs: nextTabs,
        activePath: filePath,
        selectedPath: filePath
      })
    } catch (error) {
      console.error('Error opening file:', error)
    }
  },

  closeFile: (filePath) => {
    const { tabs, activePath } = get()
    const closingTab = tabs.find((tab) => tab.path === filePath)
    const remaining = tabs.filter((tab) => tab.path !== filePath)
    const nextActive = activePath === filePath
      ? remaining[remaining.length - 1]?.path || null
      : activePath

    if (nextActive) {
      localStorage.setItem('azravibe_activepath', nextActive)
    } else {
      localStorage.removeItem('azravibe_activepath')
    }
    localStorage.setItem('azravibe_tabs', JSON.stringify(remaining))

    set({
      tabs: remaining,
      activePath: nextActive,
      selectedPath: nextActive,
      closedTabs: closingTab ? [closingTab, ...get().closedTabs].slice(0, 20) : get().closedTabs
    })
  },

  reopenClosedTab: () => {
    const { closedTabs, tabs } = get()
    const [tab, ...rest] = closedTabs
    if (!tab) return
    const nextTabs = tabs.some((item) => item.path === tab.path) ? tabs : [...tabs, tab]
    localStorage.setItem('azravibe_tabs', JSON.stringify(nextTabs))
    localStorage.setItem('azravibe_activepath', tab.path)
    set({
      tabs: nextTabs,
      activePath: tab.path,
      selectedPath: tab.path,
      closedTabs: rest
    })
  },

  saveFile: async (filePath) => {
    const { tabs } = get()
    const tabIndex = tabs.findIndex((tab) => tab.path === filePath)
    if (tabIndex === -1) return

    const tab = tabs[tabIndex]
    try {
      await window.api.writeFile(filePath, tab.content)
      const updated = [...tabs]
      updated[tabIndex] = {
        ...tab,
        originalContent: tab.content,
        isUnsaved: false
      }
      localStorage.setItem('azravibe_tabs', JSON.stringify(updated))
      set({ tabs: updated })
      await get().refreshFileTree()
    } catch (error) {
      console.error('Error saving file:', error)
    }
  },

  updateFileContent: (filePath, content) => {
    const { tabs, autoSave } = get()
    const nextTabs = tabs.map((tab) => {
      if (tab.path !== filePath) return tab
      return {
        ...tab,
        content,
        isUnsaved: autoSave ? false : content !== tab.originalContent,
        originalContent: autoSave ? content : tab.originalContent
      }
    })

    localStorage.setItem('azravibe_tabs', JSON.stringify(nextTabs))
    set({ tabs: nextTabs })
    if (autoSave) {
      window.api.writeFile(filePath, content).catch((error) => console.error('Auto-save failed:', error))
    }
  },

  syncPathChange: (oldPath, newPath) => {
    const { tabs, activePath, selectedPath } = get()
    const nextTabs = tabs.map((tab) => {
      if (!pathContains(oldPath, tab.path)) return tab
      const nextPath = tab.path === oldPath ? newPath : `${newPath}${tab.path.slice(oldPath.length)}`
      return {
        ...tab,
        path: nextPath,
        name: getFileName(nextPath)
      }
    })

    const nextActivePath = activePath && pathContains(oldPath, activePath)
      ? `${newPath}${activePath.slice(oldPath.length)}`
      : activePath
    const nextSelectedPath = selectedPath && pathContains(oldPath, selectedPath)
      ? `${newPath}${selectedPath.slice(oldPath.length)}`
      : selectedPath

    persistTabsState(nextTabs, nextActivePath, nextSelectedPath, set)
  },

  removeTabsUnderPath: (targetPath) => {
    const { tabs, activePath, selectedPath } = get()
    const nextTabs = tabs.filter((tab) => !pathContains(targetPath, tab.path))
    const nextActivePath = activePath && pathContains(targetPath, activePath)
      ? nextTabs[nextTabs.length - 1]?.path || null
      : activePath
    const nextSelectedPath = selectedPath && pathContains(targetPath, selectedPath)
      ? nextActivePath
      : selectedPath

    persistTabsState(nextTabs, nextActivePath, nextSelectedPath, set)
  },

  setActivePath: (path) => {
    if (path) {
      localStorage.setItem('azravibe_activepath', path)
    } else {
      localStorage.removeItem('azravibe_activepath')
    }
    set({ activePath: path, selectedPath: path })
  },

  setSelectedPath: (path) => set({ selectedPath: path }),

  addTerminal: (cwd) => {
    const { terminals, currentFolder } = get()
    const id = `term-${Date.now()}`
    const terminalCwd = cwd || currentFolder || 'C:\\'
    const newTerm: TerminalSession = {
      id,
      name: get().language === 'fa' ? `ترمینال ${terminals.length + 1}` : `Terminal ${terminals.length + 1}`,
      cwd: terminalCwd,
      isLoading: true
    }
    set({
      terminals: [...terminals, newTerm],
      activeTerminalId: id,
      terminalOpen: true
    })
    return id
  },

  removeTerminal: (id) => {
    const { terminals, activeTerminalId } = get()
    const termToRemove = terminals.find((terminal) => terminal.id === id)
    const remaining = terminals.filter((terminal) => terminal.id !== id)
    const nextActive = activeTerminalId === id
      ? remaining[remaining.length - 1]?.id || null
      : activeTerminalId

    if (termToRemove?.xtermInstance) {
      try {
        termToRemove.xtermInstance.dispose()
      } catch {}
    }

    set({ terminals: remaining, activeTerminalId: nextActive })
    window.api.killTerminal(id)
  },

  setActiveTerminalId: (id) => set({ activeTerminalId: id }),
  setCursorPos: (line, col) => set({ cursorLine: line, cursorCol: col }),
  setErrorCount: (count) => set({ errorCount: count }),
  setDiagnostics: (diagnostics) => set({
    diagnostics,
    errorCount: diagnostics.filter((item) => item.severity === 'error').length
  }),
  appendRunOutput: (line) => set({ runOutput: [...get().runOutput.slice(-400), line] }),
  clearRunOutput: () => set({ runOutput: [] }),

  setTabs: (tabs) => {
    localStorage.setItem('azravibe_tabs', JSON.stringify(tabs))
    set({ tabs })
  },

  setOnboarded: (onboarded) => {
    localStorage.setItem('azravibe_onboarded', String(onboarded))
    set({ onboarded })
  },

  setLanguage: (lang) => {
    localStorage.setItem('azravibe_lang', lang)
    set({ language: lang })
  },

  setTheme: (theme) => {
    localStorage.setItem('azravibe_theme', theme)
    set({ theme })
  },

  setTerminalOpen: (open) => set({ terminalOpen: open }),
  setSidebarOpen: (open) => set({ sidebarOpen: open }),
  setShowAboutModal: (show) => set({ showAboutModal: show }),
  setShowSettingsModal: createPanelTabToggle(set, get, 'settings', 'Settings'),
  setShowCommandPalette: (show) => set({ showCommandPalette: show }),
  setShowQuickOpen: (show) => set({ showQuickOpen: show }),
  setShowSearchPanel: createPanelTabToggle(set, get, 'search', 'Search'),
  setShowProblemsPanel: createPanelTabToggle(set, get, 'problems', 'Problems'),
  setShowOutlinePanel: createPanelTabToggle(set, get, 'outline', 'Outline'),
  setProblemsPanelOpen: (open) => set({ problemsPanelOpen: open }),
  setProblemsPanelPosition: (position) => set({ problemsPanelPosition: position }),
  setProblemsPanelSize: (size) => set({ problemsPanelSize: size }),
  setBottomPanelLayout: (layout) => set({ bottomPanelLayout: layout }),
  setBottomPanelTabs: (tabs) => set({ bottomPanelTabs: tabs }),
  setActiveBottomPanel: (panel) => set({ activeBottomPanel: panel }),

  moveBottomPanelTab: (fromIndex, toIndex) => {
    const { bottomPanelTabs } = get()
    const newTabs = [...bottomPanelTabs]
    const [removed] = newTabs.splice(fromIndex, 1)
    newTabs.splice(toIndex, 0, removed)
    set({ bottomPanelTabs: newTabs })
  },

  setTerminalPanelVisible: createBottomPanelVisibilityToggle(set, get, 'terminal'),
  setProblemsPanelVisible: createBottomPanelVisibilityToggle(set, get, 'problems'),
  setOutputPanelVisible: createBottomPanelVisibilityToggle(set, get, 'output'),
  setAboutPanelOpen: createPanelTabToggle(set, get, 'about', 'About'),

  setWordWrap: (wrap) => {
    localStorage.setItem('azravibe_wordwrap', String(wrap))
    set({ wordWrap: wrap })
  },

  setFontSize: (size) => {
    localStorage.setItem('azravibe_fontsize', String(size))
    set({ fontSize: size })
  },

  setMinimap: (show) => {
    localStorage.setItem('azravibe_minimap', String(show))
    set({ minimap: show })
  },

  setEditorRtl: (rtl) => {
    localStorage.setItem('azravibe_editorrtl', String(rtl))
    set({ editorRtl: rtl })
  },

  setAutoSave: (enabled) => {
    localStorage.setItem('azravibe_autosave', String(enabled))
    set({ autoSave: enabled })
  },

  setFontFamily: (family) => {
    localStorage.setItem('azravibe_fontfamily', family)
    set({ fontFamily: family })
  },

  setCompilerPath: (path) => {
    localStorage.setItem('azravibe_compilerpath', path)
    set({ compilerPath: path })
  },

  setTabSize: (size) => {
    localStorage.setItem('azravibe_tabsize', String(size))
    set({ tabSize: size })
  },

  setInsertSpaces: (enabled) => {
    localStorage.setItem('azravibe_insertspaces', String(enabled))
    set({ insertSpaces: enabled })
  },

  showToastMessage: (message) => {
    set({ toastMessage: message, showToast: true })
    setTimeout(() => {
      set({ showToast: false })
    }, 2000)
  },

  hideToast: () => {
    set({ showToast: false })
  }
}))
