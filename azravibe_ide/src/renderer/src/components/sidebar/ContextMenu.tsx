import type { RefObject } from 'react'
import { Clipboard, Copy, Edit, ExternalLink, FilePlus, FolderPlus, Scissors, Trash } from 'lucide-react'
import { FileNode } from '../../store/useStore'

interface SidebarContextMenuProps {
  currentFolder: string | null
  contextMenu: { x: number; y: number; node: FileNode } | null
  clipboard: unknown
  isFa: boolean
  menuRef: RefObject<HTMLDivElement>
  onCreateFile: () => void
  onCreateFolder: () => void
  onCopy: () => void
  onCut: () => void
  onPaste: () => void
  onDuplicate: () => void
  onRename: () => void
  onDelete: () => void
  onCopyPath: () => void
  onOpenInExplorer: () => void
}

const menuButtonClass = (isFa: boolean, extra = '') => (
  `w-full flex items-center justify-between px-3 py-1.5 text-xs ${extra} ${
    isFa ? 'flex-row-reverse text-right' : 'flex-row text-left'
  }`
)

export default function SidebarContextMenu({
  currentFolder,
  contextMenu,
  clipboard,
  isFa,
  menuRef,
  onCreateFile,
  onCreateFolder,
  onCopy,
  onCut,
  onPaste,
  onDuplicate,
  onRename,
  onDelete,
  onCopyPath,
  onOpenInExplorer
}: SidebarContextMenuProps) {
  if (!contextMenu) return null

  const isContextRoot = contextMenu.node.path === currentFolder

  return (
    <div
      ref={menuRef}
      style={{
        top: `${Math.max(8, Math.min(contextMenu.y, window.innerHeight - 260))}px`,
        left: `${Math.min(Math.max(contextMenu.x, 8), window.innerWidth - 184)}px`
      }}
      className="fixed bg-sidebar border border-neutral-800/80 rounded shadow-2xl z-[99999] py-1 w-44 context-menu text-right"
      onClick={(e) => e.stopPropagation()}
    >
      <button
        onClick={onCreateFile}
        className={menuButtonClass(isFa, 'text-txt hover:bg-editor hover:text-accent')}
      >
        <span>{isFa ? 'ایجاد فایل جدید' : 'New File'}</span>
        <FilePlus size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onCreateFolder}
        className={menuButtonClass(isFa, 'text-txt hover:bg-editor hover:text-accent')}
      >
        <span>{isFa ? 'ایجاد پوشه جدید' : 'New Folder'}</span>
        <FolderPlus size={12.5} className="text-neutral-400" />
      </button>

      <div className="border-t border-neutral-800 my-1" />

      <button
        onClick={onCopy}
        disabled={isContextRoot}
        className={menuButtonClass(isFa, `text-txt hover:bg-editor hover:text-accent ${isContextRoot ? 'opacity-30 cursor-not-allowed' : ''}`)}
      >
        <span>{isFa ? 'کپی' : 'Copy'}</span>
        <Copy size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onCut}
        disabled={isContextRoot}
        className={menuButtonClass(isFa, `text-txt hover:bg-editor hover:text-accent ${isContextRoot ? 'opacity-30 cursor-not-allowed' : ''}`)}
      >
        <span>{isFa ? 'برش' : 'Cut'}</span>
        <Scissors size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onPaste}
        disabled={!clipboard}
        className={menuButtonClass(isFa, clipboard
          ? 'text-txt hover:bg-editor hover:text-accent'
          : 'opacity-30 cursor-not-allowed text-neutral-500')}
      >
        <span>{isFa ? 'چسباندن' : 'Paste'}</span>
        <Clipboard size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onDuplicate}
        disabled={isContextRoot}
        className={menuButtonClass(isFa, `text-txt hover:bg-editor hover:text-accent ${isContextRoot ? 'opacity-30 cursor-not-allowed' : ''}`)}
      >
        <span>{isFa ? 'تکثیر' : 'Duplicate'}</span>
        <Copy size={12.5} className="text-neutral-400" />
      </button>

      <div className="border-t border-neutral-800 my-1" />

      <button
        onClick={onRename}
        disabled={isContextRoot}
        className={menuButtonClass(isFa, `text-txt hover:bg-editor hover:text-accent ${isContextRoot ? 'opacity-30 cursor-not-allowed' : ''}`)}
      >
        <span>{isFa ? 'تغییر نام' : 'Rename'}</span>
        <Edit size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onDelete}
        disabled={isContextRoot}
        className={menuButtonClass(isFa, `text-red-500 hover:bg-editor ${isContextRoot ? 'opacity-30 cursor-not-allowed' : ''}`)}
      >
        <span>{isFa ? 'انتقال به سطل بازیافت' : 'Move to Recycle Bin'}</span>
        <Trash size={12.5} className="text-red-500" />
      </button>

      <div className="border-t border-neutral-800 my-1" />

      <button
        onClick={onCopyPath}
        className={menuButtonClass(isFa, 'text-txt hover:bg-editor hover:text-accent')}
      >
        <span>{isFa ? 'کپی مسیر' : 'Copy Path'}</span>
        <Copy size={12.5} className="text-neutral-400" />
      </button>

      <button
        onClick={onOpenInExplorer}
        className={menuButtonClass(isFa, 'text-txt hover:bg-editor hover:text-accent')}
      >
        <span>{isFa ? 'باز کردن در فایل اکسپلورر' : 'Open in Explorer'}</span>
        <ExternalLink size={12.5} className="text-neutral-400" />
      </button>
    </div>
  )
}
