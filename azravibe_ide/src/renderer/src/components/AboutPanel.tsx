import React from 'react'
import { useStore } from '../store/useStore'
import { X } from 'lucide-react'
import BrandMark from './BrandMark'

const details = [
  ['Core Architecture', 'Electron + React'],
  ['Code Editor', 'CodeMirror 6'],
  ['Terminal Emulator', 'node-pty + xterm.js'],
  ['Programming Language', 'Azravibe (Persian RTL)']
]

export default function AboutPanel() {
  const { language, setAboutPanelOpen } = useStore()
  const isFa = language === 'fa'

  return (
    <div className="flex h-full w-full flex-col bg-editor text-txt" dir={isFa ? 'rtl' : 'ltr'}>
      <div className="flex h-9 items-center justify-between border-b border-editor bg-sidebar px-3">
        <span className="text-xs font-semibold">{isFa ? 'درباره' : 'About'}</span>
        <button
          onClick={() => setAboutPanelOpen(false)}
          className="flex h-7 w-7 items-center justify-center text-neutral-500 hover:bg-editor hover:text-txt"
          title={isFa ? 'بستن' : 'Close'}
        >
          <X size={14} />
        </button>
      </div>

      <div className="flex-1 overflow-y-auto px-8 py-10">
        <div className="mx-auto max-w-3xl">
          <header className="flex items-center gap-4 border-b border-editor pb-6">
            <div className="text-accent">
              <BrandMark size="xl" />
            </div>
            <div>
              <h2 className="text-2xl font-semibold text-txt">
                {isFa ? 'محیط توسعه azravibe_ide' : 'azravibe_ide Workspace'}
              </h2>
              <p className="mt-1 text-xs text-neutral-500">
                {isFa ? 'نسخه ۱.۰.۰' : 'Version 1.0.0'}
              </p>
            </div>
          </header>

          <p className="max-w-2xl border-b border-editor py-6 text-sm leading-7 text-neutral-400">
            {isFa
              ? 'محیط توسعه دسکتاپ برای زبان برنامه نویسی فارسی Azravibe، همراه با ویرایشگر CodeMirror، پایانه داخلی و مدیریت فایل یکپارچه.'
              : 'A desktop IDE for the Azravibe Persian programming language, with CodeMirror editing, an integrated terminal, and workspace file management.'}
          </p>

          <div className="divide-y divide-editor border-b border-editor">
            {details.map(([label, value]) => (
              <div key={label} className="flex items-center justify-between gap-4 py-3 text-sm">
                <span className="text-neutral-500">{label}</span>
                <span className="text-neutral-300">{value}</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  )
}
