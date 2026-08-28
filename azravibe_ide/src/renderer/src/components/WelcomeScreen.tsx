import React from 'react'
import { useStore } from '../store/useStore'
import { FolderOpen, FilePlus, Terminal } from 'lucide-react'
import BrandMark from './BrandMark'

export default function WelcomeScreen() {
  const { openFolder, openFile, currentFolder, language, addTerminal } = useStore()

  const handleOpenFolder = async () => {
    const path = await window.api.selectDirectory()
    if (path) {
      await openFolder(path)
    }
  }

  const handleNewFile = async () => {
    if (!currentFolder) {
      alert(isFa ? 'لطفا ابتدا یک پوشه کاری را باز کنید.' : 'Please open a project folder first.')
      return
    }
    const name = isFa ? `پرونده_${Date.now()}.azr` : `new_file_${Date.now()}.azr`
    const fullPath = `${currentFolder}\\${name}`
    await window.api.createFile(fullPath)
    await useStore.getState().refreshFileTree()
    await openFile(fullPath, name)
  }

  const isFa = language === 'fa'

  return (
    <div 
      className="w-full h-full bg-editor flex flex-col items-center justify-center select-none px-6"
      dir={isFa ? 'rtl' : 'ltr'}
    >
      <div className="max-w-md w-full text-center overflow-hidden">
        
        {/* Brand Header */}
        <div className="flex flex-col items-center mb-8">
          <div className="p-3 text-accent mb-2 flex items-center justify-center">
            <BrandMark size="xl" />
          </div>
          <h1 className="text-2xl font-semibold text-txt tracking-wide mb-1.5">
            {isFa ? 'محیط توسعه azravibe_ide' : 'azravibe_ide Workspace'}
          </h1>
          <p className="text-[11px] text-neutral-400">
            {isFa 
              ? 'محیط کاری برنامه‌نویسی راست‌چین به زبان شیرین فارسی' 
              : 'Premium desktop workbench for Azravibe Persian programming.'
            }
          </p>
        </div>

        {/* Action List (Clean Links, No Boxes) */}
        <div className="space-y-4 max-w-sm mx-auto mb-8">
          <button
            onClick={handleOpenFolder}
            className={`w-full flex items-center justify-between py-2 text-neutral-300 hover:text-accent transition-colors text-xs font-semibold ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}
          >
            <span className={`flex items-center min-w-0 ${isFa ? 'space-x-reverse space-x-3' : 'space-x-3'}`}>
              <FolderOpen size={16} className="text-neutral-400" />
              <span className="truncate">{isFa ? 'باز کردن پوشه پروژه' : 'Open Project Workspace'}</span>
            </span>
            <kbd className="bg-transparent text-neutral-500 text-[10px] opacity-70 shrink-0">Ctrl + O</kbd>
          </button>

          <button
            onClick={handleNewFile}
            className={`w-full flex items-center justify-between py-2 text-neutral-300 hover:text-accent transition-colors text-xs font-semibold ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}
          >
            <span className={`flex items-center min-w-0 ${isFa ? 'space-x-reverse space-x-3' : 'space-x-3'}`}>
              <FilePlus size={16} className="text-neutral-400" />
              <span className="truncate">{isFa ? 'ایجاد پرونده کد جدید' : 'Create New Code File'}</span>
            </span>
            <kbd className="bg-transparent text-neutral-500 text-[10px] opacity-70 shrink-0">Ctrl + N</kbd>
          </button>

          <button
            onClick={() => addTerminal()}
            className={`w-full flex items-center justify-between py-2 text-neutral-300 hover:text-accent transition-colors text-xs font-semibold ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}
          >
            <span className={`flex items-center min-w-0 ${isFa ? 'space-x-reverse space-x-3' : 'space-x-3'}`}>
              <Terminal size={16} className="text-neutral-400" />
              <span className="truncate">{isFa ? 'باز کردن ترمینال خط فرمان' : 'Spawn Built-in Terminal'}</span>
            </span>
            <kbd className="bg-transparent text-neutral-500 text-[10px] opacity-70 shrink-0">Ctrl + `</kbd>
          </button>
        </div>

        {/* Separator */}
        <div className="w-16 h-[1px] bg-editor mx-auto mb-8" />

        {/* Shortcuts Map (Floating clean items, No boxes/borders) */}
        <div className="max-w-xs mx-auto">
          <div className="space-y-3 text-xs">
            <div className={`flex items-center justify-between text-neutral-400 py-1 ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}>
              <span className="truncate">{isFa ? 'ذخیره خودکار تغییرات' : 'Save Active File'}</span>
              <span className="text-[10px] text-accent font-mono opacity-80 shrink-0">Ctrl + S</span>
            </div>
            
            <div className={`flex items-center justify-between text-neutral-400 py-1 ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}>
              <span className="truncate">{isFa ? 'اجرا پرونده در خط فرمان' : 'Save & Execute Program'}</span>
              <span className="text-[10px] text-accent font-mono opacity-80 shrink-0">F5</span>
            </div>

            <div className={`flex items-center justify-between text-neutral-400 py-1 ${
              isFa ? 'flex-row-reverse' : 'flex-row'
            }`}>
              <span className="truncate">{isFa ? 'بستن زبانه فعال' : 'Close Active Tab'}</span>
              <span className="text-[10px] text-accent font-mono opacity-80 shrink-0">Ctrl + W</span>
            </div>
          </div>
        </div>

      </div>
    </div>
  )
}
