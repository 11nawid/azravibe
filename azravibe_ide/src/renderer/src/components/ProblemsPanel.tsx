import React from 'react'
import { AlertCircle, ChevronsLeft, ChevronsRight, LayoutDashboard } from 'lucide-react'
import { useStore } from '../store/useStore'

interface ProblemsPanelProps {
  compact?: boolean
}

export default function ProblemsPanel({ compact = false }: ProblemsPanelProps) {
  const { diagnostics, language, setActivePath, tabs, problemsPanelPosition, setProblemsPanelPosition, setProblemsPanelOpen } = useStore()
  const isFa = language === 'fa'

  const handleItemClick = (path: string) => {
    // Check if file is already open in a tab
    const existingTab = tabs.find(t => t.path === path && !t.isPanel)
    if (existingTab) {
      setActivePath(path)
    } else {
      // Open the file using window.api to read and then use store's openFile
      window.api.readFile(path).then(content => {
        const fileName = path.split('\\').pop() || path
        // We need to call the store's openFile, but we can't directly here
        // So let's just set active path and rely on user to open it
        // Actually, let's trigger through custom event or direct API
        const { openFile } = useStore.getState()
        openFile(path, fileName)
      })
    }
  }

  const handlePositionChange = (position: 'bottom' | 'right' | 'left') => {
    setProblemsPanelPosition(position)
  }

  const handleClose = () => {
    setProblemsPanelOpen(false)
  }

  return (
    <div className="w-full h-full bg-editor flex flex-col">
      {/* Header - only show position controls if not in compact mode */}
      {!compact && (
        <div className="h-9 bg-sidebar border-b border-editor flex items-center justify-between px-3 select-none">
          <div className="flex items-center gap-2 text-xs font-semibold text-txt">
            <AlertCircle size={14} className="text-accent" />
            <span>{isFa ? 'مشکلات' : 'Problems'}</span>
            <span className="text-neutral-500 text-[10px]">({diagnostics.length})</span>
          </div>
          
          {/* Position controls */}
          <div className="flex items-center gap-1">
            <button
              onClick={() => handlePositionChange('left')}
              className={`p-1 hover:bg-editor/50 transition-colors ${
                problemsPanelPosition === 'left' ? 'text-accent' : 'text-neutral-500'
              }`}
              title={isFa ? 'انتقال به چپ' : 'Move to Left'}
            >
              <ChevronsLeft size={12} />
            </button>
            <button
              onClick={() => handlePositionChange('bottom')}
              className={`p-1 hover:bg-editor/50 transition-colors ${
                problemsPanelPosition === 'bottom' ? 'text-accent' : 'text-neutral-500'
              }`}
              title={isFa ? 'انتقال به پایین' : 'Move to Bottom'}
            >
              <LayoutDashboard size={12} />
            </button>
            <button
              onClick={() => handlePositionChange('right')}
              className={`p-1 hover:bg-editor/50 transition-colors ${
                problemsPanelPosition === 'right' ? 'text-accent' : 'text-neutral-500'
              }`}
              title={isFa ? 'انتقال به راست' : 'Move to Right'}
            >
              <ChevronsRight size={12} />
            </button>
            <button
              onClick={handleClose}
              className="p-1 hover:bg-editor/50 text-neutral-500 hover:text-txt transition-colors ml-2"
              title={isFa ? 'بستن پنل' : 'Close Panel'}
            >
              <span className="text-xs">×</span>
            </button>
          </div>
        </div>
      )}

      {/* Content */}
      <div className="flex-1 overflow-y-auto">
        {diagnostics.length === 0 ? (
          <div className="flex flex-col items-center justify-center h-full text-neutral-500">
            <AlertCircle size={48} className="mb-4 opacity-20" />
            <p className="text-xs">{isFa ? 'هیچ مشکلی شناسایی نشد' : 'No problems detected'}</p>
          </div>
        ) : (
          <div className="divide-y divide-editor border-t border-editor">
            {diagnostics.map((item, index) => (
              <button
                key={`${item.path}:${item.line}:${item.col}:${index}`}
                onClick={() => handleItemClick(item.path)}
                className="w-full text-left px-3 py-2 hover:bg-sidebar transition-colors group"
              >
                <div className="flex items-start gap-2">
                  <AlertCircle 
                    size={12} 
                    className={`mt-0.5 flex-shrink-0 ${
                      item.severity === 'error' ? 'text-red-400' : 'text-yellow-400'
                    }`} 
                  />
                  <div className="flex-1 min-w-0">
                    <p className="text-xs text-txt truncate">{item.message}</p>
                    <p className="text-[10px] text-neutral-500 mt-0.5">
                      {item.path.split('\\').pop()}:{item.line}:{item.col}
                    </p>
                  </div>
                </div>
              </button>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}
