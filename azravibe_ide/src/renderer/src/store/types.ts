import type { StateCreator } from 'zustand'

export interface TabFile {
  name: string
  path: string
  content: string
  originalContent: string
  isUnsaved: boolean
  isPanel?: boolean
  panelType?: 'problems' | 'settings' | 'search' | 'outline' | 'about'
}

export interface FileNode {
  name: string
  path: string
  isDirectory: boolean
  children?: FileNode[]
}

export interface TerminalSession {
  id: string
  name: string
  cwd: string
  isLoading: boolean
  xtermInstance?: any
  isInitialized?: boolean
}

export interface IdeState {
  currentFolder: string | null
  fileTree: FileNode[]
  tabs: TabFile[]
  closedTabs: TabFile[]
  activePath: string | null
  selectedPath: string | null
  terminals: TerminalSession[]
  activeTerminalId: string | null
  cursorLine: number
  cursorCol: number
  errorCount: number
  diagnostics: Array<{ path: string; line: number; col: number; message: string; severity: 'error' | 'warning' }>
  runOutput: string[]
  onboarded: boolean
  language: 'fa' | 'en'
  theme: 'azravibe' | 'vscode' | 'light'
  terminalOpen: boolean
  sidebarOpen: boolean
  showAboutModal: boolean
  showSettingsModal: boolean
  showCommandPalette: boolean
  showQuickOpen: boolean
  showSearchPanel: boolean
  showProblemsPanel: boolean
  showOutlinePanel: boolean
  problemsPanelOpen: boolean
  problemsPanelPosition: 'bottom' | 'right' | 'left'
  problemsPanelSize: number
  bottomPanelLayout: 'vertical' | 'horizontal'
  bottomPanelTabs: Array<'terminal' | 'problems' | 'output'>
  activeBottomPanel: 'terminal' | 'problems' | 'output' | null
  terminalPanelVisible: boolean
  problemsPanelVisible: boolean
  outputPanelVisible: boolean
  wordWrap: boolean
  fontSize: number
  minimap: boolean
  editorRtl: boolean
  autoSave: boolean
  fontFamily: string
  compilerPath: string
  tabSize: number
  insertSpaces: boolean
  toastMessage: string | null
  showToast: boolean
  openFolder: (dirPath: string) => Promise<void>
  closeFolder: () => void
  refreshFileTree: () => Promise<void>
  openFile: (filePath: string, name: string) => Promise<void>
  closeFile: (filePath: string) => void
  reopenClosedTab: () => void
  saveFile: (filePath: string) => Promise<void>
  updateFileContent: (filePath: string, content: string) => void
  syncPathChange: (oldPath: string, newPath: string) => void
  removeTabsUnderPath: (targetPath: string) => void
  setActivePath: (path: string | null) => void
  setSelectedPath: (path: string | null) => void
  addTerminal: (cwd?: string) => string
  removeTerminal: (id: string) => void
  setActiveTerminalId: (id: string | null) => void
  setOutputPanelVisible: (visible: boolean) => void
  setAboutPanelOpen: (open: boolean) => void
  setCursorPos: (line: number, col: number) => void
  setErrorCount: (count: number) => void
  setDiagnostics: (diagnostics: IdeState['diagnostics']) => void
  appendRunOutput: (line: string) => void
  clearRunOutput: () => void
  setTabs: (tabs: TabFile[]) => void
  setOnboarded: (onboarded: boolean) => void
  setLanguage: (lang: 'fa' | 'en') => void
  setTheme: (theme: 'azravibe' | 'vscode' | 'light') => void
  setTerminalOpen: (open: boolean) => void
  setSidebarOpen: (open: boolean) => void
  setShowAboutModal: (show: boolean) => void
  setShowSettingsModal: (show: boolean) => void
  setShowCommandPalette: (show: boolean) => void
  setShowQuickOpen: (show: boolean) => void
  setShowSearchPanel: (show: boolean) => void
  setShowProblemsPanel: (show: boolean) => void
  setShowOutlinePanel: (show: boolean) => void
  setProblemsPanelOpen: (open: boolean) => void
  setProblemsPanelPosition: (position: 'bottom' | 'right' | 'left') => void
  setProblemsPanelSize: (size: number) => void
  setBottomPanelLayout: (layout: 'vertical' | 'horizontal') => void
  setBottomPanelTabs: (tabs: Array<'terminal' | 'problems' | 'output'>) => void
  setActiveBottomPanel: (panel: 'terminal' | 'problems' | 'output' | null) => void
  moveBottomPanelTab: (fromIndex: number, toIndex: number) => void
  setTerminalPanelVisible: (visible: boolean) => void
  setProblemsPanelVisible: (visible: boolean) => void
  setWordWrap: (wrap: boolean) => void
  setFontSize: (size: number) => void
  setMinimap: (show: boolean) => void
  setEditorRtl: (rtl: boolean) => void
  setAutoSave: (enabled: boolean) => void
  setFontFamily: (family: string) => void
  setCompilerPath: (path: string) => void
  setTabSize: (size: number) => void
  setInsertSpaces: (enabled: boolean) => void
  showToastMessage: (message: string) => void
  hideToast: () => void
}

export type PanelType = NonNullable<TabFile['panelType']>
export type BottomPanelKey = NonNullable<IdeState['activeBottomPanel']>
export type IdeSet = Parameters<StateCreator<IdeState>>[0]
export type IdeGet = Parameters<StateCreator<IdeState>>[1]
