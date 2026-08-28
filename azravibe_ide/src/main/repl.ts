import { app, BrowserWindow, ipcMain } from 'electron'
import { spawn, ChildProcessWithoutNullStreams } from 'child_process'
import { existsSync } from 'fs'
import { join, dirname } from 'path'

type ReplState = 'ready' | 'more' | 'exit'

interface ReplSession {
  id: string
  proc: ChildProcessWithoutNullStreams
  win: BrowserWindow
  pending: string
}

const sessions = new Map<string, ReplSession>()
const READY_MARK = '[[AZR_READY]]'
const MORE_MARK = '[[AZR_MORE]]'

function findEnginePath(): string {
  const devCandidates = [
    join(app.getAppPath(), '..', '..', '..', 'azravibe_lang_v1', 'azravibe.exe'),
    join(process.cwd(), '..', 'azravibe_lang_v1', 'azravibe.exe'),
    join(process.cwd(), 'azravibe.exe')
  ]

  const packagedCandidates = [
    join(process.resourcesPath, 'azravibe-runtime', 'azravibe.exe'),
    join(dirname(process.execPath), 'azravibe.exe'),
    join(dirname(process.execPath), '..', 'azravibe.exe')
  ]

  const candidates = app.isPackaged ? packagedCandidates : devCandidates
  for (const candidate of candidates) {
    if (existsSync(candidate)) return candidate
  }
  return 'azravibe.exe'
}

function emitData(session: ReplSession, chunk: string): void {
  if (!chunk) return
  if (!session.win.isDestroyed()) {
    session.win.webContents.send(`repl-data-${session.id}`, chunk)
  }
}

function emitState(session: ReplSession, state: ReplState): void {
  if (!session.win.isDestroyed()) {
    session.win.webContents.send(`repl-state-${session.id}`, state)
  }
}

function processChunk(session: ReplSession, data: Buffer): void {
  session.pending += data.toString('utf8')

  while (true) {
    const readyIndex = session.pending.indexOf(READY_MARK)
    const moreIndex = session.pending.indexOf(MORE_MARK)

    let markerIndex = -1
    let markerText = ''
    let state: ReplState | null = null

    if (readyIndex >= 0 && (moreIndex === -1 || readyIndex < moreIndex)) {
      markerIndex = readyIndex
      markerText = READY_MARK
      state = 'ready'
    } else if (moreIndex >= 0) {
      markerIndex = moreIndex
      markerText = MORE_MARK
      state = 'more'
    }

    if (markerIndex === -1) break

    emitData(session, session.pending.slice(0, markerIndex))
    session.pending = session.pending.slice(markerIndex + markerText.length)
    if (session.pending.startsWith('\r\n')) session.pending = session.pending.slice(2)
    else if (session.pending.startsWith('\n')) session.pending = session.pending.slice(1)
    if (state) emitState(session, state)
  }

  if (session.pending.length > 0 && !session.pending.includes('[[AZR_')) {
    emitData(session, session.pending)
    session.pending = ''
  }
}

function disposeSession(id: string): void {
  const session = sessions.get(id)
  if (!session) return
  sessions.delete(id)
  session.proc.removeAllListeners()
  if (!session.proc.killed) {
    session.proc.kill()
  }
}

export function setupReplHandlers(): void {
  ipcMain.handle('repl-create', async (event, id: string) => {
    disposeSession(id)

    const enginePath = findEnginePath()
    const proc = spawn(enginePath, ['--gui-repl'], {
      cwd: dirname(enginePath),
      windowsHide: true
    })

    const win = BrowserWindow.fromWebContents(event.sender)
    if (!win) throw new Error('Unable to attach REPL session to window')

    const session: ReplSession = {
      id,
      proc,
      win,
      pending: ''
    }

    sessions.set(id, session)

    proc.stdout.on('data', (data: Buffer) => processChunk(session, data))
    proc.stderr.on('data', (data: Buffer) => processChunk(session, data))

    proc.on('close', () => {
      emitState(session, 'exit')
      sessions.delete(id)
    })
  })

  ipcMain.on('repl-write', (_event, id: string, data: string) => {
    const session = sessions.get(id)
    if (!session || !session.proc.stdin.writable) return
    session.proc.stdin.write(data)
  })

  ipcMain.on('repl-kill', (_event, id: string) => {
    disposeSession(id)
  })
}

export function cleanUpReplSessions(): void {
  for (const id of sessions.keys()) disposeSession(id)
}
