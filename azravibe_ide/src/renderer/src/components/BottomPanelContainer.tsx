import React from 'react'
import { Terminal as TerminalIcon, AlertCircle, X } from 'lucide-react'
import { useStore } from '../store/useStore'
import TerminalPanel from './TerminalPanel'
import ProblemsPanel from './ProblemsPanel'

type PanelType = 'terminal' | 'problems' | 'output'

export default function BottomPanelContainer() {
  const {
    bottomPanelTabs,
    activeBottomPanel,
    setActiveBottomPanel,
    setTerminalPanelVisible,
    setProblemsPanelVisible,
    setProblemsPanelOpen,
    language
  } = useStore()
  
  const isFa = language === 'fa'

  const getLabel = (type: PanelType) => {
    switch (type) {
      case 'terminal': return isFa ? 'ترمینال' : 'Terminal'
      case 'problems': return isFa ? 'مشکلات' : 'Problems'
      case 'output': return isFa ? 'خروجی' : 'Output'
    }
  }
  
  const getIcon = (type: PanelType) => {
    switch (type) {
      case 'terminal': return <TerminalIcon size={12} />
      case 'problems': return <AlertCircle size={12} />
      case 'output': return <TerminalIcon size={12} />
    }
  }

  const renderPanelContent = (type: PanelType) => {
    switch (type) {
      case 'terminal':
        return <TerminalPanel />
      case 'problems':
        return <ProblemsPanel compact />
      case 'output':
        return <OutputPanel />
      default:
        return null
    }
  }

  return (
    <div className="flex flex-col h-full bg-bg">
      {/* Panel Header with tabs */}
      <div className="h-9 bg-sidebar border-b border-editor flex items-center justify-between px-2 select-none">
        {/* Tabs */}
        <div className="flex items-center gap-0 overflow-x-auto">
          {bottomPanelTabs.map((tabType) => (
            <div
              key={tabType}
              onClick={() => setActiveBottomPanel(tabType)}
              className={`
                flex items-center gap-1.5 px-3 py-1.5 text-xs cursor-pointer select-none transition-all
                ${activeBottomPanel === tabType ? 'text-accent border-t-2 border-t-accent' : 'text-neutral-400 hover:text-neutral-200 hover:bg-editor/30'}
              `}
            >
              {getIcon(tabType)}
              <span>{getLabel(tabType)}</span>
            </div>
          ))}
        </div>

        {/* Controls */}
        <div className="flex items-center gap-1">
          {/* Close button */}
          <button
            onClick={() => {
              setTerminalPanelVisible(false)
              setProblemsPanelVisible(false)
              setProblemsPanelOpen(false)
            }}
            className="p-1.5 hover:bg-editor/50 text-neutral-500 hover:text-txt transition-colors"
            title={isFa ? 'بستن پنل' : 'Close Panel'}
          >
            <X size={12} />
          </button>
        </div>
      </div>

      {/* Panel Content - show only active panel */}
      <div className="flex-1 overflow-hidden">
        {activeBottomPanel && renderPanelContent(activeBottomPanel)}
      </div>
    </div>
  )
}

function OutputPanel() {
  const { runOutput, language } = useStore()
  const isFa = language === 'fa'

  return (
    <div className="w-full h-full bg-bg p-3 overflow-y-auto font-mono text-xs">
      {runOutput.length === 0 ? (
        <div className="flex flex-col items-center justify-center h-full text-neutral-500">
          <p className="text-xs">{isFa ? 'بدون خروجی' : 'No output'}</p>
        </div>
      ) : (
        <div className="space-y-1">
          {runOutput.map((line, i) => (
            <div key={i} className="text-neutral-300">{line}</div>
          ))}
        </div>
      )}
    </div>
  )
}
