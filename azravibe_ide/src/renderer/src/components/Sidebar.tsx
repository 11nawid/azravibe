import * as React from 'react'
import { useState, useEffect, useRef } from 'react'
import { useStore, FileNode } from '../store/useStore'
import { 
  FilePlus, 
  FolderPlus, 
  FolderOpen, 
  RotateCw,
  FolderMinus
} from 'lucide-react'
import SidebarContextMenu from './sidebar/ContextMenu'
import SidebarDropPrompt from './sidebar/DropPrompt'
import TreeNode from './sidebar/TreeNode'
import { pathContains, pathParent, validatePathName } from './sidebar/shared'

export default function Sidebar() {
  const { 
    currentFolder, 
    fileTree, 
    refreshFileTree, 
    openFolder, 
    language,
    selectedPath,
    setSelectedPath,
    syncPathChange,
    removeTabsUnderPath
  } = useStore()

  const [contextMenu, setContextMenu] = useState<{ x: number; y: number; node: FileNode } | null>(null)
  
  // Inline File/Folder creation state
  const [inlineInput, setInlineInput] = useState<{ parentPath: string; isDirectory: boolean } | null>(null)
  const [expandedPaths, setExpandedPaths] = useState<Set<string>>(new Set())
  const [selectedPaths, setSelectedPaths] = useState<Set<string>>(new Set())
  const [draggingPaths, setDraggingPaths] = useState<string[]>([])
  const [dropTargetPath, setDropTargetPath] = useState<string | null>(null)
  const [dropPrompt, setDropPrompt] = useState<{ sources: string[]; targetDir: string } | null>(null)

  // Renaming state
  const [renamingPath, setRenamingPath] = useState<string | null>(null)
  const [renameValue, setRenameValue] = useState('')
  const [renameError, setRenameError] = useState<string | null>(null)

  // Clipboard support state
  const [clipboard, setClipboard] = useState<{ 
    path: string 
    action: 'copy' | 'cut' 
    name: string 
    isDirectory: boolean 
  } | null>(null)

  const menuRef = useRef<HTMLDivElement>(null)
  const isFa = language === 'fa'

  useEffect(() => {
    function handleClickOutside() {
      setContextMenu(null)
    }
    document.addEventListener('click', handleClickOutside)
    return () => document.removeEventListener('click', handleClickOutside)
  }, [])

  useEffect(() => {
    if (!currentFolder) return
    setExpandedPaths((prev) => {
      if (prev.has(currentFolder)) return prev
      return new Set([...prev, currentFolder])
    })
  }, [currentFolder])

  useEffect(() => {
    if (selectedPath && !selectedPaths.has(selectedPath)) {
      setSelectedPaths(new Set([selectedPath]))
    }
  }, [selectedPath])

  // Helper to find a node by path recursively
  const findNodeByPath = (nodes: FileNode[], pathStr: string): FileNode | null => {
    if (currentFolder && pathStr === currentFolder) {
      return {
        name: currentFolder.split('\\').pop() || currentFolder,
        path: currentFolder,
        isDirectory: true,
        children: fileTree
      }
    }
    for (const node of nodes) {
      if (node.path === pathStr) return node
      if (node.children) {
        const found = findNodeByPath(node.children, pathStr)
        if (found) return found
      }
    }
    return null
  }

  // Get active directory to create inside based on user clicks
  const getActiveDirectory = (): string => {
    if (!currentFolder) return ''
    if (!selectedPath) return currentFolder

    const activeNode = findNodeByPath(fileTree, selectedPath)
    if (activeNode) {
      if (activeNode.isDirectory) {
        return activeNode.path
      } else {
        // Active node is file, return parent folder
        return activeNode.path.substring(0, activeNode.path.lastIndexOf('\\'))
      }
    }
    return currentFolder
  }

  const handleContextMenu = (e: React.MouseEvent, node: FileNode) => {
    e.preventDefault()
    e.stopPropagation()
    setSelectedPath(node.path)
    if (!selectedPaths.has(node.path)) {
      setSelectedPaths(new Set([node.path]))
    }
    setContextMenu({
      x: e.clientX,
      y: e.clientY,
      node
    })
  }

  // Handle right-clicking empty space of explorer to launch root menu
  const handleEmptyContextMenu = (e: React.MouseEvent) => {
    e.preventDefault()
    if (!currentFolder) return
    const rootNode: FileNode = {
      name: currentFolder.split('\\').pop() || 'Root',
      path: currentFolder,
      isDirectory: true
    }
    setContextMenu({
      x: e.clientX,
      y: e.clientY,
      node: rootNode
    })
  }

  const toggleExpand = (path: string) => {
    const next = new Set(expandedPaths)
    if (next.has(path)) {
      next.delete(path)
    } else {
      next.add(path)
    }
    setExpandedPaths(next)
  }

  const getUniqueDestinationPath = (dirPath: string, name: string) => {
    const dotIndex = name.lastIndexOf('.')
    const isFileName = dotIndex > 0
    const base = isFileName ? name.slice(0, dotIndex) : name
    const ext = isFileName ? name.slice(dotIndex) : ''
    let candidate = `${dirPath}\\${name}`
    let index = 1
    while (findNodeByPath(fileTree, candidate)) {
      candidate = `${dirPath}\\${base} copy${index === 1 ? '' : ` ${index}`}${ext}`
      index += 1
    }
    return candidate
  }

  const handleNodeClick = (e: React.MouseEvent, node: FileNode) => {
    e.stopPropagation()

    if (e.ctrlKey || e.metaKey) {
      const next = new Set(selectedPaths)
      if (next.has(node.path)) {
        next.delete(node.path)
      } else {
        next.add(node.path)
      }
      setSelectedPaths(next.size ? next : new Set([node.path]))
      setSelectedPath(node.path)
      return
    }

    setSelectedPaths(new Set([node.path]))
    setSelectedPath(node.path)

    if (node.isDirectory) {
      toggleExpand(node.path)
    } else {
      useStore.getState().openFile(node.path, node.name)
    }
  }

  const getDragSources = (node: FileNode) => {
    const active = selectedPaths.has(node.path) ? Array.from(selectedPaths) : [node.path]
    const withoutRoot = active.filter((sourcePath) => sourcePath !== currentFolder)
    return withoutRoot.filter((sourcePath) => !withoutRoot.some((candidate) => candidate !== sourcePath && pathContains(candidate, sourcePath)))
  }

  const handleDragStart = (e: React.DragEvent, node: FileNode) => {
    const sources = getDragSources(node)
    if (sources.length === 0) {
      e.preventDefault()
      return
    }
    setDraggingPaths(sources)
    e.dataTransfer.effectAllowed = 'copyMove'
    e.dataTransfer.setData('application/x-azravibe-paths', JSON.stringify(sources))
    e.dataTransfer.setData('text/plain', sources.join('\n'))
  }

  const handleDragEnd = () => {
    setDraggingPaths([])
    setDropTargetPath(null)
  }

  const resolveDropTargetDir = (node: FileNode) => node.isDirectory ? node.path : pathParent(node.path)

  const handleDropOnNode = (e: React.DragEvent, node: FileNode) => {
    e.preventDefault()
    e.stopPropagation()
    const targetDir = resolveDropTargetDir(node)
    const raw = e.dataTransfer.getData('application/x-azravibe-paths')
    let sources = draggingPaths
    try {
      if (raw) sources = JSON.parse(raw)
    } catch {}

    sources = sources
      .filter((sourcePath) => !sources.some((candidate) => candidate !== sourcePath && pathContains(candidate, sourcePath)))
      .filter((sourcePath) => {
      if (!sourcePath || sourcePath === targetDir) return false
      if (pathParent(sourcePath) === targetDir) return false
      if (pathContains(sourcePath, targetDir)) return false
      return true
    })

    setDropTargetPath(null)
    if (sources.length > 0) {
      setDropPrompt({ sources, targetDir })
    }
  }

  const runDropOperation = async (operation: 'copy' | 'move') => {
    if (!dropPrompt) return
    const { sources, targetDir } = dropPrompt
    const completedDestinations: string[] = []

    try {
      for (const sourcePath of sources) {
        const sourceNode = findNodeByPath(fileTree, sourcePath)
        const sourceName = sourceNode?.name || sourcePath.split('\\').pop() || 'item'
        const destination = getUniqueDestinationPath(targetDir, sourceName)

        if (operation === 'copy') {
          await window.api.copyPath(sourcePath, destination)
        } else {
          await window.api.renamePath(sourcePath, destination)
          syncPathChange(sourcePath, destination)
        }
        completedDestinations.push(destination)
      }

      const nextExpanded = new Set(expandedPaths)
      nextExpanded.add(targetDir)
      setExpandedPaths(nextExpanded)
      await refreshFileTree()
      if (completedDestinations.length > 0) {
        setSelectedPath(completedDestinations[completedDestinations.length - 1])
        setSelectedPaths(new Set(completedDestinations))
      }
    } catch (error) {
      console.error(`Failed to ${operation} dragged items:`, error)
    } finally {
      setDropPrompt(null)
      setDraggingPaths([])
    }
  }

  const handleCollapseAll = () => {
    setExpandedPaths(new Set())
  }

  // Trigger inline creation for file inside active selected path
  const triggerInlineFile = () => {
    if (!currentFolder) return
    const targetDir = getActiveDirectory()
    
    // Automatically expand the active folder
    const next = new Set(expandedPaths)
    next.add(targetDir)
    setExpandedPaths(next)

    setInlineInput({
      parentPath: targetDir,
      isDirectory: false
    })
    setContextMenu(null)
  }

  // Trigger inline creation for folder inside active selected path
  const triggerInlineFolder = () => {
    if (!currentFolder) return
    const targetDir = getActiveDirectory()
    
    // Automatically expand the active folder
    const next = new Set(expandedPaths)
    next.add(targetDir)
    setExpandedPaths(next)

    setInlineInput({
      parentPath: targetDir,
      isDirectory: true
    })
    setContextMenu(null)
  }

  const handleInlineSubmit = async (parent: string, name: string, isDir: boolean) => {
    const fullPath = `${parent}\\${name}`
    try {
      if (isDir) {
        await window.api.createDirectory(fullPath)
      } else {
        await window.api.createFile(fullPath)
      }
      await refreshFileTree()
      setSelectedPath(fullPath)
    } catch (e) {
      console.error(e)
    }
    setInlineInput(null)
  }

  const handleInlineCancel = () => {
    setInlineInput(null)
  }

  const handleRenameInit = () => {
    if (!contextMenu) return
    setRenamingPath(contextMenu.node.path)
    setRenameValue(contextMenu.node.name)
    setRenameError(null)
    setContextMenu(null)
  }

  const handleRenameSubmit = async (node: FileNode) => {
    const nextError = validatePathName(renameValue, isFa)
    if (nextError) {
      setRenameError(nextError)
      return
    }

    if (!renameValue || renameValue === node.name) {
      setRenamingPath(null)
      setRenameError(null)
      return
    }

    const parentDir = node.path.substring(0, node.path.lastIndexOf('\\'))
    const newPath = `${parentDir}\\${renameValue}`
    try {
      await window.api.renamePath(node.path, newPath)
      syncPathChange(node.path, newPath)
      await refreshFileTree()
      setSelectedPath(newPath)
    } catch (e) {
      console.error(e)
    }
    setRenamingPath(null)
    setRenameError(null)
  }

  // Clipboard actions
  const handleCopy = () => {
    if (!contextMenu) return
    setClipboard({
      path: contextMenu.node.path,
      name: contextMenu.node.name,
      isDirectory: contextMenu.node.isDirectory,
      action: 'copy'
    })
    setContextMenu(null)
  }

  const handleCut = () => {
    if (!contextMenu) return
    setClipboard({
      path: contextMenu.node.path,
      name: contextMenu.node.name,
      isDirectory: contextMenu.node.isDirectory,
      action: 'cut'
    })
    setContextMenu(null)
  }

  const handlePaste = async () => {
    if (!clipboard || !currentFolder) return
    const targetDir = getActiveDirectory()
    const destPath = `${targetDir}\\${clipboard.name}`

    try {
      await window.api.copyPath(clipboard.path, destPath)
      if (clipboard.action === 'cut') {
        await window.api.deletePath(clipboard.path)
        syncPathChange(clipboard.path, destPath)
        setClipboard(null) // clear clipboard after cut
      }
      await refreshFileTree()
      setSelectedPath(destPath)
    } catch (e) {
      console.error(e)
    }
    setContextMenu(null)
  }

  const handleDuplicate = async () => {
    if (!contextMenu) return
    const node = contextMenu.node
    const parentDir = node.path.substring(0, node.path.lastIndexOf('\\'))
    const dotIndex = node.name.lastIndexOf('.')
    const base = !node.isDirectory && dotIndex > 0 ? node.name.slice(0, dotIndex) : node.name
    const ext = !node.isDirectory && dotIndex > 0 ? node.name.slice(dotIndex) : ''
    let index = 1
    let destPath = `${parentDir}\\${base}_copy${ext}`

    while (findNodeByPath(fileTree, destPath)) {
      index += 1
      destPath = `${parentDir}\\${base}_copy_${index}${ext}`
    }

    try {
      await window.api.copyPath(node.path, destPath)
      await refreshFileTree()
      setSelectedPath(destPath)
    } catch (e) {
      console.error(e)
    }
    setContextMenu(null)
  }

  const handleDelete = async () => {
    if (!contextMenu) return
    const confirm = window.confirm(
      isFa 
        ? `آیا "${contextMenu.node.name}" به سطل بازیافت منتقل شود؟` 
        : `Move "${contextMenu.node.name}" to the Recycle Bin?`
    )
    if (confirm) {
      try {
        await window.api.trashPath(contextMenu.node.path)
      } catch {
        await window.api.deletePath(contextMenu.node.path)
      }
      removeTabsUnderPath(contextMenu.node.path)
      await refreshFileTree()
      setSelectedPath(null)
    }
    setContextMenu(null)
  }

  const handleCopyPath = async () => {
    if (!contextMenu) return
    try {
      await navigator.clipboard.writeText(contextMenu.node.path)
    } catch (e) {
      console.error('Failed to copy path:', e)
    }
    setContextMenu(null)
  }

  const handleOpenInExplorer = async () => {
    if (!contextMenu) return
    try {
      await window.api.openInExplorer(contextMenu.node.path)
    } catch (e) {
      console.error('Failed to open in explorer:', e)
    }
    setContextMenu(null)
  }

  return (
    <div 
      onContextMenu={handleEmptyContextMenu}
      onDragOver={(event) => {
        if (!currentFolder || draggingPaths.length === 0) return
        if ((event.target as HTMLElement).closest('.explorer-tree-row')) return
        event.preventDefault()
        setDropTargetPath(currentFolder)
      }}
      onDrop={(event) => {
        if (!currentFolder) return
        if ((event.target as HTMLElement).closest('.explorer-tree-row')) return
        event.preventDefault()
        const raw = event.dataTransfer.getData('application/x-azravibe-paths')
        let sources = draggingPaths
        try {
          if (raw) sources = JSON.parse(raw)
        } catch {}
        const filtered = sources.filter((sourcePath) => pathParent(sourcePath) !== currentFolder && !pathContains(sourcePath, currentFolder))
        setDropTargetPath(null)
        if (filtered.length > 0) setDropPrompt({ sources: filtered, targetDir: currentFolder })
      }}
      onClick={() => {
        setSelectedPath(currentFolder)
        setSelectedPaths(currentFolder ? new Set([currentFolder]) : new Set())
      }}
      className={`h-full bg-sidebar flex flex-col relative select-none border-neutral-900 ${isFa ? 'border-l' : 'border-r'}`}
    >
      {/* Header Panel */}
      <div 
        className={`h-9 border-b border-editor flex items-center justify-between px-3 ${isFa ? 'flex-row' : 'flex-row-reverse'}`}
        onClick={(e) => e.stopPropagation()}
      >
        <span className="text-[10px] font-extrabold uppercase tracking-wider text-neutral-400">
          {isFa ? 'جستجوگر پرونده' : 'Explorer'}
        </span>
        
        {currentFolder && (
          <div className="flex items-center space-x-1 no-drag">
            <button
              onClick={triggerInlineFile}
              title={isFa ? 'پرونده جدید' : 'New File'}
              className="p-1 hover:bg-neutral-800 rounded text-neutral-400 hover:text-accent transition-colors"
            >
              <FilePlus size={13.5} />
            </button>
            <button
              onClick={triggerInlineFolder}
              title={isFa ? 'پوشه جدید' : 'New Folder'}
              className="p-1 hover:bg-neutral-800 rounded text-neutral-400 hover:text-accent transition-colors"
            >
              <FolderPlus size={13.5} />
            </button>
            <button
              onClick={handleCollapseAll}
              title={isFa ? 'بستن همه پوشه‌ها' : 'Collapse Folders'}
              className="p-1 hover:bg-neutral-800 rounded text-neutral-400 hover:text-accent transition-colors"
            >
              <FolderMinus size={13.5} />
            </button>
            <button
              onClick={refreshFileTree}
              title={isFa ? 'به‌روزرسانی' : 'Refresh'}
              className="p-1 hover:bg-neutral-800 rounded text-neutral-400 hover:text-accent transition-colors"
            >
              <RotateCw size={12} />
            </button>
          </div>
        )}
      </div>

      {/* Tree Content: Flow directions dynamically mapped to isFa */}
      <div className="flex-1 overflow-y-auto py-1" dir={isFa ? 'rtl' : 'ltr'}>
        {currentFolder ? (
          <div className="relative w-full">
            <TreeNode
              key={currentFolder}
              node={{
                name: currentFolder.split('\\').pop() || currentFolder,
                path: currentFolder,
                isDirectory: true,
                children: fileTree
              }}
              depth={0}
              isRoot
              onContextMenu={handleContextMenu}
              renamingPath={renamingPath}
              setRenamingPath={setRenamingPath}
              renameValue={renameValue}
              setRenameValue={setRenameValue}
              renameError={renameError}
              setRenameError={setRenameError}
              onRenameSubmit={handleRenameSubmit}
              expandedPaths={expandedPaths}
              toggleExpand={toggleExpand}
              inlineInput={inlineInput}
              onInlineSubmit={handleInlineSubmit}
              onInlineCancel={handleInlineCancel}
              selectedPath={selectedPath}
              selectedPaths={selectedPaths}
              onNodeClick={handleNodeClick}
              onDragStart={handleDragStart}
              onDragEnd={handleDragEnd}
              onDropOnNode={handleDropOnNode}
              dropTargetPath={dropTargetPath}
              setDropTargetPath={setDropTargetPath}
              isFa={isFa}
            />
            {fileTree.length === 0 && !inlineInput && (
              <div className="px-8 py-2 text-[11px] text-neutral-500">
                {isFa ? 'پوشه خالی است' : 'Folder is empty'}
              </div>
            )}
          </div>
        ) : (
          /* flat borderless placeholder */
          <div className="flex flex-col items-center justify-center h-full px-4 text-center mt-10" dir={isFa ? 'rtl' : 'ltr'}>
            <div className="text-neutral-500 mb-3">
              <FolderOpen size={24} />
            </div>
            <h3 className="text-xs font-semibold text-neutral-400 mb-1">
              {isFa ? 'پروژه‌ای باز نشده است' : 'No Active Workspace'}
            </h3>
            <p className="text-[10px] text-neutral-500 max-w-[160px] leading-relaxed mb-4">
              {isFa 
                ? 'برای مشاهده ساختار درختی فایل‌ها، پوشه پروژه خود را باز کنید.' 
                : 'Open your project folder to view the hierarchical workspace tree.'
              }
            </p>
            <button
              onClick={async () => {
                const path = await window.api.selectDirectory()
                if (path) openFolder(path)
              }}
              className="px-4 py-1.5 bg-neutral-800 hover:bg-neutral-700 text-white rounded text-[11px] font-semibold transition-all"
            >
              {isFa ? 'باز کردن پوشه پروژه' : 'Open Workspace'}
            </button>
          </div>
        )}
      </div>

      <SidebarContextMenu
        currentFolder={currentFolder}
        contextMenu={contextMenu}
        clipboard={clipboard}
        isFa={isFa}
        menuRef={menuRef}
        onCreateFile={triggerInlineFile}
        onCreateFolder={triggerInlineFolder}
        onCopy={handleCopy}
        onCut={handleCut}
        onPaste={() => void handlePaste()}
        onDuplicate={() => void handleDuplicate()}
        onRename={handleRenameInit}
        onDelete={() => void handleDelete()}
        onCopyPath={() => void handleCopyPath()}
        onOpenInExplorer={() => void handleOpenInExplorer()}
      />

      <SidebarDropPrompt
        dropPrompt={dropPrompt}
        isFa={isFa}
        onMove={() => void runDropOperation('move')}
        onCopy={() => void runDropOperation('copy')}
        onClose={() => setDropPrompt(null)}
      />
    </div>
  )
}
