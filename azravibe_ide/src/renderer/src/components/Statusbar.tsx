import React from 'react'
import { useStore } from '../store/useStore'
import { Code, AlignLeft, Save, GitBranch, Clock, Zap } from 'lucide-react'

export default function Statusbar() {
  const { 
    activePath, cursorLine, cursorCol, language, autoSave, 
    tabSize, insertSpaces,
    currentFolder
  } = useStore()

  const isFa = language === 'fa'

  // Get active language
  const ext = activePath?.split('.').pop()
  const langLabel = ext === 'azr' ? 'Azravibe' : activePath ? ext?.toUpperCase() : (isFa ? 'بدون پرونده' : 'No File')

  // Get current time
  const currentTime = new Date().toLocaleTimeString(isFa ? 'fa-IR' : 'en-US', { 
    hour: '2-digit', 
    minute: '2-digit' 
  })

  return (
    <div className={`h-6 bg-sidebar border-t border-editor flex items-center justify-between px-3 select-none text-xs text-neutral-200 ${
      isFa ? 'flex-row' : 'flex-row-reverse'
    }`}>
      {/* Right side in RTL (Left side in LTR): Editor Info */}
      <div className={`flex items-center space-x-3 ${isFa ? 'flex-row' : 'flex-row-reverse'}`}>
        <div className={`flex items-center ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <Code size={12} className="text-accent" />
          <span className="font-medium">{isFa ? `زبان: ${langLabel}` : `Lang: ${langLabel}`}</span>
        </div>

        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <span className="text-[10px]">UTF-8</span>
        </div>

        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <span className="text-[10px]">CRLF</span>
        </div>

        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <span className="text-[10px]">{insertSpaces ? `${tabSize} ${isFa ? 'فضا' : 'Spaces'}` : 'Tab'}</span>
        </div>

        {autoSave && (
          <div className={`flex items-center text-accent ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
            <Save size={12} />
            <span className="font-medium text-[10px]">{isFa ? 'ذخیره خودکار' : 'Auto Save'}</span>
          </div>
        )}

        {currentFolder && (
          <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
            <GitBranch size={11} />
            <span className="text-[10px] truncate max-w-[150px]">{currentFolder.split('\\').pop()}</span>
          </div>
        )}
      </div>

      <div aria-hidden="true" />

      {/* Left side in RTL (Right side in LTR): Cursor and workspace state */}
      <div className={`flex items-center space-x-2 ${isFa ? 'flex-row-reverse' : 'flex-row'}`}>
        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <AlignLeft size={11} />
          <span className="text-[10px]">
            {isFa 
              ? `سطر ${cursorLine}، ستون ${cursorCol}` 
              : `Ln ${cursorLine}, Col ${cursorCol}`
            }
          </span>
        </div>

        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <Clock size={11} />
          <span className="text-[10px]">{currentTime}</span>
        </div>

        <div className={`flex items-center text-neutral-400 ${isFa ? 'space-x-reverse space-x-1' : 'space-x-1'}`}>
          <Zap size={11} className="text-yellow-500" />
          <span className="text-[10px]">{isFa ? 'آماده' : 'Ready'}</span>
        </div>

      </div>
    </div>
  )
}
