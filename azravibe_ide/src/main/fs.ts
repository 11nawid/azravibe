import { ipcMain, dialog, shell } from 'electron'
import * as fs from 'fs/promises'
import * as fsSync from 'fs'
import * as path from 'path'

export function setupFsHandlers() {
  ipcMain.handle('fs-select-dir', async () => {
    const result = await dialog.showOpenDialog({
      properties: ['openDirectory']
    })
    if (result.canceled || result.filePaths.length === 0) {
      return null
    }
    return result.filePaths[0]
  })

  ipcMain.handle('fs-read-dir', async (_event, dirPath: string) => {
    try {
      const files = await fs.readdir(dirPath)
      const list = await Promise.all(
        files.map(async (name) => {
          const fullPath = path.join(dirPath, name)
          let isDirectory = false
          try {
            const stat = await fs.stat(fullPath)
            isDirectory = stat.isDirectory()
          } catch (e) {}
          return { name, isDirectory, path: fullPath }
        })
      )
      // Sort: Directories first, then alphabetically
      return list.sort((a, b) => {
        if (a.isDirectory && !b.isDirectory) return -1
        if (!a.isDirectory && b.isDirectory) return 1
        return a.name.localeCompare(b.name)
      })
    } catch (error) {
      console.error('Error reading directory:', error)
      return []
    }
  })

  ipcMain.handle('fs-read-file', async (_event, filePath: string) => {
    return await fs.readFile(filePath, 'utf-8')
  })

  ipcMain.handle('fs-write-file', async (_event, filePath: string, content: string) => {
    await fs.mkdir(path.dirname(filePath), { recursive: true })
    await fs.writeFile(filePath, content, 'utf-8')
  })

  ipcMain.handle('fs-create-file', async (_event, filePath: string) => {
    await fs.mkdir(path.dirname(filePath), { recursive: true })
    // If it doesn't exist, create empty file
    if (!fsSync.existsSync(filePath)) {
      await fs.writeFile(filePath, '', 'utf-8')
    }
  })

  ipcMain.handle('fs-create-dir', async (_event, dirPath: string) => {
    await fs.mkdir(dirPath, { recursive: true })
  })

  ipcMain.handle('fs-rename', async (_event, oldPath: string, newPath: string) => {
    await fs.rename(oldPath, newPath)
  })

  ipcMain.handle('fs-delete', async (_event, filePath: string) => {
    await fs.rm(filePath, { recursive: true, force: true })
  })

  ipcMain.handle('fs-trash', async (_event, filePath: string) => {
    await shell.trashItem(filePath)
  })

  ipcMain.handle('fs-copy', async (_event, srcPath: string, destPath: string) => {
    await fs.cp(srcPath, destPath, { recursive: true })
  })

  ipcMain.handle('fs-open-in-explorer', async (_event, filePath: string) => {
    await shell.showItemInFolder(filePath)
  })
}
