import { app, shell, BrowserWindow, ipcMain } from 'electron'
import { join } from 'path'
import { electronApp, optimizer, is } from '@electron-toolkit/utils'
import { setupFsHandlers } from './fs'
import { setupPtyHandlers, cleanUpTerminals } from './pty'
import { setupReplHandlers, cleanUpReplSessions } from './repl'
import { createSplashWindow, closeSplashWindow } from './splash'

let mainWindow: BrowserWindow | null = null
let pendingOpenFile: string | null = null

function getAzravibeFileArg(argv: string[]): string | null {
  return argv.find((arg) => arg.toLowerCase().endsWith('.azr')) || null
}

function sendOpenFile(filePath: string): void {
  if (!mainWindow || mainWindow.isDestroyed()) {
    pendingOpenFile = filePath
    return
  }

  if (mainWindow.webContents.isLoading()) {
    pendingOpenFile = filePath
    return
  }

  mainWindow.webContents.send('app-open-file', filePath)
  pendingOpenFile = null
}

function loadWindowContents(window: BrowserWindow, consoleMode: boolean): void {
  if (is.dev && process.env['ELECTRON_RENDERER_URL']) {
    window.loadURL(`${process.env['ELECTRON_RENDERER_URL']}?mode=${consoleMode ? 'console' : 'ide'}`)
  } else {
    window.loadFile(join(__dirname, '../renderer/index.html'), {
      query: { mode: consoleMode ? 'console' : 'ide' }
    })
  }
}

function createWindow(consoleMode = false, options?: { waitForSplash?: boolean }): BrowserWindow {
  const iconPath = is.dev
    ? join(__dirname, '../../public/icon.ico')
    : join(process.resourcesPath, 'icon.ico')
  const waitForSplash = options?.waitForSplash ?? false

  const window = new BrowserWindow({
    width: consoleMode ? 1180 : 1200,
    height: consoleMode ? 760 : 800,
    minWidth: consoleMode ? 760 : 900,
    minHeight: consoleMode ? 480 : 600,
    show: false,
    frame: consoleMode,
    autoHideMenuBar: true,
    title: consoleMode ? 'Azravibe Console' : 'Azravibe IDE',
    icon: iconPath,
    transparent: false,
    backgroundColor: consoleMode ? '#f6f1e8' : '#0d0d0d',
    webPreferences: {
      preload: join(__dirname, '../preload/index.js'),
      sandbox: false,
      contextIsolation: true,
      nodeIntegration: false
    }
  })

  if (!mainWindow || mainWindow.isDestroyed()) {
    mainWindow = window
  }

  window.on('focus', () => {
    if (!window.isDestroyed()) {
      mainWindow = window
    }
  })

  window.on('closed', () => {
    if (mainWindow === window) {
      const [fallback] = BrowserWindow.getAllWindows()
      mainWindow = fallback ?? null
    }
  })

  window.on('ready-to-show', async () => {
    if (waitForSplash) {
      await closeSplashWindow()
    }
    window.show()
    if (pendingOpenFile) {
      sendOpenFile(pendingOpenFile)
    }
  })

  window.webContents.setWindowOpenHandler((details) => {
    shell.openExternal(details.url)
    return { action: 'deny' }
  })

  loadWindowContents(window, consoleMode)

  return window
}

function createConsoleWindow(): BrowserWindow {
  return createWindow(true, { waitForSplash: false })
}

const gotSingleInstanceLock = app.requestSingleInstanceLock()

if (!gotSingleInstanceLock) {
  app.quit()
} else {
  app.on('second-instance', (_event, argv) => {
    const wantsConsole = argv.includes('--console')
    const filePath = getAzravibeFileArg(argv)
    if (wantsConsole) {
      const window = createConsoleWindow()
      window.focus()
      return
    }
    if (mainWindow) {
      if (mainWindow.isMinimized()) mainWindow.restore()
      mainWindow.focus()
    }
    if (filePath) sendOpenFile(filePath)
  })
}

// This method will be called when Electron has finished initialization
app.whenReady().then(() => {
  // Set app user model id for windows notifications
  electronApp.setAppUserModelId('com.azravibe.ide')

  // Default open or close DevTools by F12 in development
  // and ignore CommandOrControl + R in production.
  app.on('browser-window-created', (_, window) => {
    optimizer.watchWindowShortcuts(window)
  })

  // Setup FS and PTY handlers
  setupFsHandlers()
  setupPtyHandlers()
  setupReplHandlers()

  ipcMain.on('window-minimize', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.minimize()
  })

  ipcMain.on('window-maximize', (event) => {
    const window = BrowserWindow.fromWebContents(event.sender)
    if (!window) return
    if (window.isMaximized()) {
      window.unmaximize()
    } else {
      window.maximize()
    }
  })

  ipcMain.on('window-close', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.close()
  })

  ipcMain.on('window-new-console', () => {
    createConsoleWindow()
  })

  ipcMain.on('window-set-title', (event, title: string) => {
    const window = BrowserWindow.fromWebContents(event.sender)
    if (!window || window.isDestroyed()) return
    window.setTitle(title || 'Azravibe')
  })

  // Show splash screen before creating main window
  createSplashWindow()

  pendingOpenFile = getAzravibeFileArg(process.argv)
  createWindow(process.argv.includes('--console'), { waitForSplash: true })

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) createWindow(false, { waitForSplash: false })
  })
})

// Quit when all windows are closed, except on macOS.
app.on('window-all-closed', () => {
  cleanUpTerminals()
  cleanUpReplSessions()
  if (process.platform !== 'darwin') {
    app.quit()
  }
})
