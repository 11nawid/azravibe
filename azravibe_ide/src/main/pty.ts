import { ipcMain, BrowserWindow } from 'electron'
import * as pty from 'node-pty'
import * as os from 'os'
import { existsSync } from 'fs'

const ptyProcesses = new Map<string, pty.IPty>()

const shellPrompt = [
  '\\[\\e]0;\\w\\a\\]',
  '\\[\\e[32m\\]\\u@\\h\\[\\e[0m\\]',
  ' ',
  '\\[\\e[33m\\]\\w\\[\\e[0m\\]',
  '\\n',
  '\\$ '
].join('')

function getWindowsShell() {
  const gitBashCandidates = [
    'C:\\Program Files\\Git\\bin\\bash.exe',
    'C:\\Program Files\\Git\\usr\\bin\\bash.exe',
    'C:\\Program Files (x86)\\Git\\bin\\bash.exe'
  ]

  for (const candidate of gitBashCandidates) {
    if (existsSync(candidate)) {
      return {
        shell: candidate,
        args: ['--noprofile', '--norc', '-i'],
        env: {
          CHERE_INVOKING: '1',
          MSYSTEM: 'MINGW64',
          TERM: 'xterm-256color',
          LANG: 'en_US.UTF-8',
          TERM_PROGRAM: 'Azravibe',
          PS1: shellPrompt
        }
      }
    }
  }

  return {
    shell: 'powershell.exe',
    args: [],
    env: {
      TERM: 'xterm-256color'
    }
  }
}

export function setupPtyHandlers() {
  ipcMain.handle('pty-spawn', async (event, id: string, cols: number, rows: number, cwd: string) => {
    // Clean up existing if any
    if (ptyProcesses.has(id)) {
      try {
        ptyProcesses.get(id)?.kill()
      } catch (e) {}
      ptyProcesses.delete(id)
    }

    const shellConfig =
      os.platform() === 'win32'
        ? getWindowsShell()
        : {
            shell: 'bash',
            args: ['--login', '-i'],
            env: {
              TERM: 'xterm-256color',
              LANG: 'en_US.UTF-8',
              TERM_PROGRAM: 'Azravibe',
              PS1: shellPrompt
            }
          }
    
    const safeCols = Math.max(1, cols || 80)
    const safeRows = Math.max(1, rows || 24)

    const ptyProcess = pty.spawn(shellConfig.shell, shellConfig.args, {
      name: 'xterm-color',
      cols: safeCols,
      rows: safeRows,
      cwd: cwd || os.homedir(),
      env: {
        ...(process.env as Record<string, string>),
        ...shellConfig.env
      }
    })

    ptyProcesses.set(id, ptyProcess)

    const sender = event.sender
    const win = BrowserWindow.fromWebContents(sender)

    ptyProcess.onData((data) => {
      if (win && !win.isDestroyed()) {
        win.webContents.send(`pty-data-${id}`, data)
      }
    })

    ptyProcess.onExit(() => {
      if (win && !win.isDestroyed()) {
        win.webContents.send(`pty-exit-${id}`)
      }
      ptyProcesses.delete(id)
    })
  })

  ipcMain.on('pty-write', (_event, id: string, data: string) => {
    const proc = ptyProcesses.get(id)
    if (proc) {
      proc.write(data)
    }
  })

  ipcMain.on('pty-resize', (_event, id: string, cols: number, rows: number) => {
    const proc = ptyProcesses.get(id)
    if (proc) {
      try {
        const safeCols = Math.max(1, cols || 80)
        const safeRows = Math.max(1, rows || 24)
        proc.resize(safeCols, safeRows)
      } catch (e) {
        console.error('Error resizing PTY:', e)
      }
    }
  })

  ipcMain.on('pty-kill', (_event, id: string) => {
    const proc = ptyProcesses.get(id)
    if (proc) {
      try {
        proc.kill()
      } catch (e) {}
      ptyProcesses.delete(id)
    }
  })
}

// Clean up all terminals on app close
export function cleanUpTerminals() {
  for (const [id, proc] of ptyProcesses.entries()) {
    try {
      proc.kill()
    } catch (e) {}
  }
  ptyProcesses.clear()
}
