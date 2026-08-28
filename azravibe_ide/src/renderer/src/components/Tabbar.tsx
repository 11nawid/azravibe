import React, { useState } from 'react'
import { useStore, TabFile } from '../store/useStore'
import { X, AlertCircle, Settings, Search, Boxes, Info, FileCode2, FileText } from 'lucide-react'

export default function Tabbar() {
  const { tabs, activePath, setActivePath, closeFile, setTabs, language } = useStore()
  const [draggedIdx, setDraggedIdx] = useState<number | null>(null)
  const isFa = language === 'fa'

  const handleDragStart = (idx: number) => {
    setDraggedIdx(idx)
  }

  const handleDragOver = (e: React.DragEvent, idx: number) => {
    e.preventDefault()
  }

  const handleDrop = (idx: number) => {
    if (draggedIdx === null || draggedIdx === idx) return

    const newTabs = [...tabs]
    const [draggedTab] = newTabs.splice(draggedIdx, 1)
    newTabs.splice(idx, 0, draggedTab)

    setTabs(newTabs)
    setDraggedIdx(null)
  }

  // Get icon for panel tabs
  const getPanelIcon = (panelType?: string) => {
    switch (panelType) {
      case 'problems': return <AlertCircle size={15} className="shrink-0 text-accent" />
      case 'settings': return <Settings size={15} className="shrink-0 text-accent" />
      case 'search': return <Search size={15} className="shrink-0 text-accent" />
      case 'outline': return <Boxes size={15} className="shrink-0 text-accent" />
      case 'about': return <Info size={15} className="shrink-0 text-accent" />
      default: return null
    }
  }

  const getFileIcon = (tab: TabFile, isActive: boolean) => {
    if (tab.isPanel) return getPanelIcon(tab.panelType)
    const iconClass = `shrink-0 ${isActive ? 'text-accent' : 'text-neutral-500 group-hover:text-neutral-300'}`
    return tab.name.toLowerCase().endsWith('.azr')
      ? <FileCode2 size={15} className={iconClass} />
      : <FileText size={15} className={iconClass} />
  }

  if (tabs.length === 0) return null

  return (
    <div className="flex h-11 items-end overflow-x-auto border-b border-editor bg-sidebar px-1 pt-1 select-none">
      {tabs.map((tab, idx) => {
        const isActive = activePath === tab.path
        return (
          <div
            key={tab.path}
            draggable
            onDragStart={() => handleDragStart(idx)}
            onDragOver={(e) => handleDragOver(e, idx)}
            onDrop={() => handleDrop(idx)}
            onClick={() => setActivePath(tab.path)}
            title={tab.path}
            className={`group relative flex h-10 min-w-[138px] max-w-[220px] cursor-pointer items-center gap-2 border-x border-transparent px-3 text-[13px] transition-all duration-150 ${
              isActive 
                ? 'bg-editor text-neutral-100 border-x-editor border-t-2 border-t-accent' 
                : 'text-neutral-400 hover:bg-editor hover:text-neutral-100'
            }`}
          >
            {getFileIcon(tab, isActive)}

            <div className="flex min-w-0 flex-1 items-center gap-2">
              
              {/* Unsaved indicator */}
              {!tab.isPanel && tab.isUnsaved && (
                <span className="h-2 w-2 shrink-0 bg-accent" />
              )}

              <span className={`truncate ${isActive ? 'font-semibold text-neutral-50' : ''}`}>
                {tab.name}
              </span>
            </div>

            {/* Close button */}
            <button
              onClick={(e) => {
                e.stopPropagation()
                if (tab.isPanel) {
                  // Close panel tab
                  const { setShowProblemsPanel, setShowSettingsModal, setShowSearchPanel, setShowOutlinePanel, setAboutPanelOpen } = useStore.getState()
                  if (tab.panelType === 'problems') setShowProblemsPanel(false)
                  if (tab.panelType === 'settings') setShowSettingsModal(false)
                  if (tab.panelType === 'search') setShowSearchPanel(false)
                  if (tab.panelType === 'outline') setShowOutlinePanel(false)
                  if (tab.panelType === 'about') setAboutPanelOpen(false)
                } else if (tab.isUnsaved) {
                  const ok = window.confirm(isFa
                    ? `پرونده "${tab.name}" ذخیره نشده است. بدون ذخیره بسته شود؟`
                    : `"${tab.name}" has unsaved changes. Close without saving?`
                  )
                  if (!ok) return
                  closeFile(tab.path)
                } else {
                  closeFile(tab.path)
                }
              }}
              className={`grid h-6 w-6 shrink-0 place-items-center transition-colors ${
                isActive
                  ? 'text-neutral-400 hover:bg-sidebar hover:text-white'
                  : 'text-neutral-600 opacity-70 hover:bg-editor hover:text-white group-hover:opacity-100'
              }`}
              title={isFa ? 'بستن زبانه' : 'Close tab'}
            >
              <X size={14} />
            </button>
          </div>
        )
      })}
    </div>
  )
}
