import { ElectronAPI } from '@electron-toolkit/preload'

declare global {
  interface Window {
    electron: ElectronAPI
    api: {
      minimize: () => void
      maximize: () => void
      close: () => void
      openConsoleWindow: () => void
      setWindowTitle: (title: string) => void
      selectDirectory: () => Promise<string | null>
      readDir: (dirPath: string) => Promise<Array<{ name: string; isDirectory: boolean; path: string }>>
      readFile: (filePath: string) => Promise<string>
      writeFile: (filePath: string, content: string) => Promise<void>
      createFile: (filePath: string) => Promise<void>
      createDirectory: (dirPath: string) => Promise<void>
      renamePath: (oldPath: string, newPath: string) => Promise<void>
      deletePath: (filePath: string) => Promise<void>
      trashPath: (filePath: string) => Promise<void>
      copyPath: (srcPath: string, destPath: string) => Promise<void>
      openInExplorer: (filePath: string) => Promise<void>
      
      // Terminal
      spawnTerminal: (id: string, cols: number, rows: number, cwd: string) => Promise<void>
      writeTerminal: (id: string, data: string) => void
      resizeTerminal: (id: string, cols: number, rows: number) => void
      killTerminal: (id: string) => void
      onTerminalData: (id: string, callback: (data: string) => void) => () => void
      onTerminalExit: (id: string, callback: () => void) => () => void
      onOpenFilePath: (callback: (filePath: string) => void) => () => void
      createReplSession: (id: string) => Promise<void>
      writeReplSession: (id: string, data: string) => void
      killReplSession: (id: string) => void
      onReplData: (id: string, callback: (data: string) => void) => () => void
      onReplState: (id: string, callback: (state: 'ready' | 'more' | 'exit') => void) => () => void
    }
  }
}
