import { BrowserWindow } from 'electron'
import { join } from 'path'
import { is } from '@electron-toolkit/utils'

let splashWindow: BrowserWindow | null = null
let splashCreatedTime: number = 0

export function createSplashWindow(): BrowserWindow {
  const iconPath = is.dev
    ? join(__dirname, '../../public/icon.ico')
    : join(process.resourcesPath, 'icon.ico')

  splashWindow = new BrowserWindow({
    width: 460,
    height: 220,
    frame: false,
    transparent: false,
    backgroundColor: '#0d0d0d',
    alwaysOnTop: true,
    resizable: false,
    icon: iconPath,
    webPreferences: {
      preload: join(__dirname, '../preload/index.js'),
      sandbox: false
    }
  })

  // Record creation time
  splashCreatedTime = Date.now()

  // Center the splash window
  splashWindow.center()

  // Load splash HTML from the renderer build in production. The old public path is not
  // packaged beside the main bundle, which could leave the splash as a blank black window.
  if (is.dev && process.env['ELECTRON_RENDERER_URL']) {
    splashWindow.loadURL(`${process.env['ELECTRON_RENDERER_URL']}/splash.html`)
  } else {
    splashWindow.loadFile(join(__dirname, '../renderer/splash.html'))
  }

  return splashWindow
}

export function closeSplashWindow(): Promise<void> {
  return new Promise((resolve) => {
    if (splashWindow && !splashWindow.isDestroyed()) {
      const windowToClose = splashWindow
      const elapsedTime = Date.now() - splashCreatedTime
      const remainingTime = Math.max(0, 3000 - elapsedTime)
      
      // Ensure splash shows for at least 3 seconds from creation
      setTimeout(() => {
        if (windowToClose && !windowToClose.isDestroyed()) {
          windowToClose.close()
        }
        splashWindow = null
        resolve()
      }, remainingTime)
    } else {
      // Splash already closed or doesn't exist
      resolve()
    }
  })
}

export function getSplashWindow(): BrowserWindow | null {
  return splashWindow
}
