import { contextBridge, ipcRenderer } from 'electron'
import { electronAPI } from '@electron-toolkit/preload'

// Custom APIs for renderer
const api = {
  minimize: () => ipcRenderer.send('window-minimize'),
  maximize: () => ipcRenderer.send('window-maximize'),
  close: () => ipcRenderer.send('window-close'),
  openConsoleWindow: () => ipcRenderer.send('window-new-console'),
  setWindowTitle: (title: string) => ipcRenderer.send('window-set-title', title),
  
  selectDirectory: () => ipcRenderer.invoke('fs-select-dir'),
  readDir: (dirPath: string) => ipcRenderer.invoke('fs-read-dir', dirPath),
  readFile: (filePath: string) => ipcRenderer.invoke('fs-read-file', filePath),
  writeFile: (filePath: string, content: string) => ipcRenderer.invoke('fs-write-file', filePath, content),
  createFile: (filePath: string) => ipcRenderer.invoke('fs-create-file', filePath),
  createDirectory: (dirPath: string) => ipcRenderer.invoke('fs-create-dir', dirPath),
  renamePath: (oldPath: string, newPath: string) => ipcRenderer.invoke('fs-rename', oldPath, newPath),
  deletePath: (filePath: string) => ipcRenderer.invoke('fs-delete', filePath),
  trashPath: (filePath: string) => ipcRenderer.invoke('fs-trash', filePath),
  copyPath: (srcPath: string, destPath: string) => ipcRenderer.invoke('fs-copy', srcPath, destPath),
  openInExplorer: (filePath: string) => ipcRenderer.invoke('fs-open-in-explorer', filePath),

  // Terminal PTY
  spawnTerminal: (id: string, cols: number, rows: number, cwd: string) => 
    ipcRenderer.invoke('pty-spawn', id, cols, rows, cwd),
  writeTerminal: (id: string, data: string) => 
    ipcRenderer.send('pty-write', id, data),
  resizeTerminal: (id: string, cols: number, rows: number) => 
    ipcRenderer.send('pty-resize', id, cols, rows),
  killTerminal: (id: string) => 
    ipcRenderer.send('pty-kill', id),
  onTerminalData: (id: string, callback: (data: string) => void) => {
    const channel = `pty-data-${id}`;
    const listener = (_event: Electron.IpcRendererEvent, data: string) => callback(data)
    ipcRenderer.on(channel, listener);
    return () => ipcRenderer.removeListener(channel, listener)
  },
  onTerminalExit: (id: string, callback: () => void) => {
    const channel = `pty-exit-${id}`;
    const listener = () => callback()
    ipcRenderer.on(channel, listener);
    return () => ipcRenderer.removeListener(channel, listener)
  },
  onOpenFilePath: (callback: (filePath: string) => void) => {
    const listener = (_event: Electron.IpcRendererEvent, filePath: string) => callback(filePath)
    ipcRenderer.on('app-open-file', listener)
    return () => ipcRenderer.removeListener('app-open-file', listener)
  },

  createReplSession: (id: string) =>
    ipcRenderer.invoke('repl-create', id),
  writeReplSession: (id: string, data: string) =>
    ipcRenderer.send('repl-write', id, data),
  killReplSession: (id: string) =>
    ipcRenderer.send('repl-kill', id),
  onReplData: (id: string, callback: (data: string) => void) => {
    const channel = `repl-data-${id}`
    const listener = (_event: Electron.IpcRendererEvent, data: string) => callback(data)
    ipcRenderer.on(channel, listener)
    return () => ipcRenderer.removeListener(channel, listener)
  },
  onReplState: (id: string, callback: (state: 'ready' | 'more' | 'exit') => void) => {
    const channel = `repl-state-${id}`
    const listener = (_event: Electron.IpcRendererEvent, state: 'ready' | 'more' | 'exit') => callback(state)
    ipcRenderer.on(channel, listener)
    return () => ipcRenderer.removeListener(channel, listener)
  }
}

// Expose Electron APIs and Custom APIs
if (process.contextIsolated) {
  try {
    contextBridge.exposeInMainWorld('electron', electronAPI)
    contextBridge.exposeInMainWorld('api', api)
  } catch (error) {
    console.error(error)
  }
} else {
  // @ts-ignore (define in window)
  window.electron = electronAPI
  // @ts-ignore (define in window)
  window.api = api
}
