import React from 'react'
import { useStore } from '../store/useStore'
import { X } from 'lucide-react'
import SettingsPanel from './SettingsPanel'

export default function SettingsModal() {
  const { showSettingsModal, setShowSettingsModal, language } = useStore()

  if (!showSettingsModal) return null

  const isFa = language === 'fa'

  return (
    <div
      className="fixed inset-0 z-[99999] flex items-start justify-center bg-black/55 pt-14 select-none"
      dir={isFa ? 'rtl' : 'ltr'}
      onClick={() => setShowSettingsModal(false)}
    >
      <div
        className="h-[min(760px,calc(100vh-96px))] w-[min(820px,calc(100vw-40px))] border border-editor bg-editor"
        onClick={(event) => event.stopPropagation()}
      >
        <div className="flex h-9 items-center justify-between border-b border-editor bg-sidebar px-3">
          <span className="text-xs font-semibold text-txt">{isFa ? 'تنظیمات' : 'Settings'}</span>
          <button
            onClick={() => setShowSettingsModal(false)}
            className="flex h-7 w-7 items-center justify-center text-neutral-500 hover:bg-editor hover:text-txt"
            title={isFa ? 'بستن' : 'Close'}
          >
            <X size={14} />
          </button>
        </div>
        <div className="h-[calc(100%-36px)]">
          <SettingsPanel />
        </div>
      </div>
    </div>
  )
}
