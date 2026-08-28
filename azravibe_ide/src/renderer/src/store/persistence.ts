import { FileNode, IdeSet, IdeState, TabFile } from './types'

export const savedFolder = localStorage.getItem('azravibe_currentfolder')
export const savedActivePath = localStorage.getItem('azravibe_activepath')

export let savedTabs: TabFile[] = []
try {
  savedTabs = JSON.parse(localStorage.getItem('azravibe_tabs') || '[]')
} catch {}

export const savedOnboarded = localStorage.getItem('azravibe_onboarded') === 'true'
export const savedLang = (localStorage.getItem('azravibe_lang') as 'fa' | 'en') || 'fa'
export const savedTheme = (localStorage.getItem('azravibe_theme') as 'azravibe' | 'vscode' | 'light') || 'azravibe'
export const savedWordWrap = localStorage.getItem('azravibe_wordwrap') !== 'false'
export const savedFontSize = parseInt(localStorage.getItem('azravibe_fontsize') || '14', 10)
export const savedMinimap = localStorage.getItem('azravibe_minimap') !== 'false'

if (localStorage.getItem('azravibe_editorrtl_defaulted_v2') !== 'true') {
  localStorage.setItem('azravibe_editorrtl', 'true')
  localStorage.setItem('azravibe_editorrtl_defaulted_v2', 'true')
}

export const savedEditorRtl = localStorage.getItem('azravibe_editorrtl') !== 'false'
export const savedAutoSave = localStorage.getItem('azravibe_autosave') === 'true'
export const savedFontFamily = localStorage.getItem('azravibe_fontfamily') || 'Vazirmatn, Consolas, monospace'
export const savedCompilerPath = localStorage.getItem('azravibe_compilerpath') || 'azravibe'
export const savedTabSize = parseInt(localStorage.getItem('azravibe_tabsize') || '4', 10)
export const savedInsertSpaces = localStorage.getItem('azravibe_insertspaces') === 'true'
export const ignoredTreeDirs = new Set(['node_modules', '.git', 'dist', 'out', 'build', '.vite', '.cache'])

export const pathContains = (parent: string, child: string) => (
  child === parent || child.startsWith(`${parent}\\`)
)

export const getFileName = (filePath: string) => filePath.split('\\').pop() || filePath

export const persistTabsState = (
  tabs: TabFile[],
  activePath: string | null,
  selectedPath: string | null,
  set: IdeSet
) => {
  if (activePath) {
    localStorage.setItem('azravibe_activepath', activePath)
  } else {
    localStorage.removeItem('azravibe_activepath')
  }

  localStorage.setItem('azravibe_tabs', JSON.stringify(tabs))
  set({ tabs, activePath, selectedPath })
}

export const collectVisibleFiles = (nodes: FileNode[]) => {
  const visibleFiles = new Set<string>()

  const walk = (entries: FileNode[]) => {
    for (const node of entries) {
      if (node.isDirectory) {
        walk(node.children || [])
      } else {
        visibleFiles.add(node.path)
      }
    }
  }

  walk(nodes)
  return visibleFiles
}
