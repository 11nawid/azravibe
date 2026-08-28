import React from 'react'
import { useStore } from '../store/useStore'
import { X } from 'lucide-react'
import BrandMark from './BrandMark'

export default function AboutModal() {
  const { showAboutModal, setShowAboutModal, language } = useStore()

  if (!showAboutModal) return null

  const isFa = language === 'fa'

  return (
    <div
      className="fixed inset-0 z-[10000] flex items-start justify-center bg-black/60 pt-16 select-none"
      dir={isFa ? 'rtl' : 'ltr'}
      onClick={() => setShowAboutModal(false)}
    >
      <div
        className="w-[min(620px,calc(100vw-40px))] border border-editor bg-editor"
        onClick={(event) => event.stopPropagation()}
      >
        <div className="flex h-9 items-center justify-between border-b border-editor bg-sidebar px-3">
          <span className="text-xs font-semibold text-txt">{isFa ? 'درباره آذراوایب' : 'About Azravibe IDE'}</span>
          <button
            onClick={() => setShowAboutModal(false)}
            className="flex h-7 w-7 items-center justify-center text-neutral-500 hover:bg-editor hover:text-txt"
            title={isFa ? 'بستن' : 'Close'}
          >
            <X size={14} />
          </button>
        </div>

        <div className="p-6">
          <div className="flex items-center gap-4 border-b border-editor pb-5">
            <div className="text-accent">
              <BrandMark size="lg" />
            </div>
            <div>
              <h2 className="text-lg font-semibold text-txt">
                {isFa ? 'محیط توسعه azravibe_ide' : 'azravibe_ide Workspace'}
              </h2>
              <p className="mt-1 text-[11px] text-neutral-500">{isFa ? 'نسخه ۱.۰.۰' : 'Version 1.0.0'}</p>
            </div>
          </div>

          <p className="border-b border-editor py-5 text-xs leading-6 text-neutral-400">
            {isFa
              ? 'محیط توسعه دسکتاپ برای زبان برنامه نویسی فارسی Azravibe، همراه با ویرایشگر CodeMirror، پایانه داخلی و مدیریت فایل یکپارچه.'
              : 'A desktop IDE for the Azravibe Persian programming language, with CodeMirror editing, an integrated terminal, and workspace file management.'}
          </p>

          <div className="divide-y divide-editor text-xs">
            <div className="flex justify-between py-3"><span className="text-neutral-500">Core</span><span className="text-neutral-300">Electron + React</span></div>
            <div className="flex justify-between py-3"><span className="text-neutral-500">Editor</span><span className="text-neutral-300">CodeMirror 6</span></div>
            <div className="flex justify-between py-3"><span className="text-neutral-500">Terminal</span><span className="text-neutral-300">node-pty + xterm.js</span></div>
          </div>
        </div>
      </div>
    </div>
  )
}
