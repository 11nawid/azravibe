import * as React from 'react'
import { ChevronDown, ChevronRight, Folder, FolderOpen } from 'lucide-react'
import { FileNode } from '../../store/useStore'
import InlineInputNode from './InlineInputNode'
import { getFileIcon, pathParent, validatePathName } from './shared'

interface TreeNodeProps {
  node: FileNode
  depth: number
  isRoot?: boolean
  onContextMenu: (e: React.MouseEvent, node: FileNode) => void
  renamingPath: string | null
  setRenamingPath: (path: string | null) => void
  renameValue: string
  setRenameValue: (val: string) => void
  renameError: string | null
  setRenameError: (val: string | null) => void
  onRenameSubmit: (node: FileNode) => void
  expandedPaths: Set<string>
  toggleExpand: (path: string) => void
  inlineInput: { parentPath: string; isDirectory: boolean } | null
  onInlineSubmit: (parent: string, name: string, isDir: boolean) => void
  onInlineCancel: () => void
  selectedPath: string | null
  selectedPaths: Set<string>
  onNodeClick: (e: React.MouseEvent, node: FileNode) => void
  onDragStart: (e: React.DragEvent, node: FileNode) => void
  onDragEnd: () => void
  onDropOnNode: (e: React.DragEvent, node: FileNode) => void
  dropTargetPath: string | null
  setDropTargetPath: (path: string | null) => void
  isFa: boolean
}

export default function TreeNode({
  node,
  depth,
  isRoot = false,
  onContextMenu,
  renamingPath,
  setRenamingPath,
  renameValue,
  setRenameValue,
  renameError,
  setRenameError,
  onRenameSubmit,
  expandedPaths,
  toggleExpand,
  inlineInput,
  onInlineSubmit,
  onInlineCancel,
  selectedPath,
  selectedPaths,
  onNodeClick,
  onDragStart,
  onDragEnd,
  onDropOnNode,
  dropTargetPath,
  setDropTargetPath,
  isFa
}: TreeNodeProps) {
  const isSelected = selectedPaths.has(node.path) || selectedPath === node.path
  const isRenaming = renamingPath === node.path
  const isOpen = expandedPaths.has(node.path)
  const isDropTarget = dropTargetPath === node.path

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      onRenameSubmit(node)
    } else if (e.key === 'Escape') {
      setRenamingPath(null)
    }
  }

  return (
    <div className="w-full explorer-tree-item">
      <div
        onContextMenu={(e) => onContextMenu(e, node)}
        onClick={(e) => onNodeClick(e, node)}
        draggable
        onDragStart={(e) => onDragStart(e, node)}
        onDragEnd={onDragEnd}
        onDragOver={(e) => {
          e.preventDefault()
          e.stopPropagation()
          setDropTargetPath(node.isDirectory ? node.path : pathParent(node.path))
        }}
        onDragLeave={(e) => {
          e.stopPropagation()
          setDropTargetPath(null)
        }}
        onDrop={(e) => onDropOnNode(e, node)}
        style={isFa ? { paddingRight: `${depth * 12 + 8}px` } : { paddingLeft: `${depth * 12 + 8}px` }}
        className={`explorer-tree-row ${isRoot ? 'explorer-tree-row-root' : ''} ${node.isDirectory && isOpen ? 'explorer-tree-row-open' : ''} flex flex-row items-center cursor-pointer text-xs select-none transition-colors w-full ${
          isFa ? 'text-right justify-start' : 'text-left justify-start'
        } ${
          isSelected ? 'bg-[#04395e] text-[#ffffff]' : 'text-neutral-300 hover:bg-white/5'
        } ${
          isDropTarget ? 'bg-[#0e639c]/50 outline outline-1 outline-[#3794ff]' : ''
        }`}
      >
        {node.isDirectory ? (
          <span
            className={`shrink-0 ${isFa ? 'ml-0.5' : 'mr-0.5'} text-neutral-400 hover:text-white`}
            onClick={(event) => {
              event.stopPropagation()
              toggleExpand(node.path)
            }}
          >
            {isOpen ? <ChevronDown size={13} /> : (isFa ? <ChevronRight size={13} className="rotate-180" /> : <ChevronRight size={13} />)}
          </span>
        ) : (
          <span className="w-[13px] shrink-0" />
        )}

        <span className={`shrink-0 ${isFa ? 'ml-1.5' : 'mr-1.5'}`}>
          {node.isDirectory
            ? (isOpen ? <FolderOpen size={14} className="text-[#dcb67a]" /> : <Folder size={14} className="text-[#c5a66a]" />)
            : getFileIcon(node.name)}
        </span>

        {isRenaming ? (
          <div className="flex-1 min-w-0">
            <input
              autoFocus
              type="text"
              value={renameValue}
              onChange={(e) => {
                setRenameValue(e.target.value)
                setRenameError(validatePathName(e.target.value, isFa))
              }}
              onBlur={() => {
                if (!renameError) onRenameSubmit(node)
              }}
              onKeyDown={handleKeyDown}
              onClick={(e) => e.stopPropagation()}
              className={`bg-neutral-900 border text-white px-1.5 py-0.5 text-[11px] outline-none rounded w-full mx-1 ${
                renameError ? 'border-red-500/80' : 'border-accent'
              }`}
            />
            {renameError && (
              <div className={`mt-1 mx-1 text-[10px] text-red-300 leading-snug ${isFa ? 'text-right' : 'text-left'}`}>
                {renameError}
              </div>
            )}
          </div>
        ) : (
          <span className={`truncate flex-1 ${isRoot ? 'font-semibold text-[#cccccc]' : 'font-normal'} ${isFa ? 'text-right' : 'text-left'}`}>
            {node.name}
          </span>
        )}
      </div>

      {node.isDirectory && isOpen && (
        <div
          className={`explorer-tree-group flex flex-col w-full ${
            isFa ? 'explorer-tree-group-rtl' : 'explorer-tree-group-ltr'
          }`}
          style={{ ['--tree-depth' as any]: depth + 1 }}
        >
          {inlineInput && inlineInput.parentPath === node.path && (
            <InlineInputNode
              isDirectory={inlineInput.isDirectory}
              depth={depth + 1}
              onSubmit={(name: string) => onInlineSubmit(node.path, name, inlineInput.isDirectory)}
              onCancel={onInlineCancel}
              isFa={isFa}
            />
          )}

          {node.children?.map((child) => (
            <TreeNode
              key={child.path}
              node={child}
              depth={depth + 1}
              onContextMenu={onContextMenu}
              renamingPath={renamingPath}
              setRenamingPath={setRenamingPath}
              renameValue={renameValue}
              setRenameValue={setRenameValue}
              renameError={renameError}
              setRenameError={setRenameError}
              onRenameSubmit={onRenameSubmit}
              expandedPaths={expandedPaths}
              toggleExpand={toggleExpand}
              inlineInput={inlineInput}
              onInlineSubmit={onInlineSubmit}
              onInlineCancel={onInlineCancel}
              selectedPath={selectedPath}
              selectedPaths={selectedPaths}
              onNodeClick={onNodeClick}
              onDragStart={onDragStart}
              onDragEnd={onDragEnd}
              onDropOnNode={onDropOnNode}
              dropTargetPath={dropTargetPath}
              setDropTargetPath={setDropTargetPath}
              isFa={isFa}
            />
          ))}
        </div>
      )}
    </div>
  )
}
