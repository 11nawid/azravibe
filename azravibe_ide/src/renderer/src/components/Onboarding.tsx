import React, { useState } from 'react'
import { useStore } from '../store/useStore'
import { Check, ChevronRight, Globe, Minus, Palette, Square, X } from 'lucide-react'
import BrandMark from './BrandMark'

export default function Onboarding() {
  const { setOnboarded, language, setLanguage, theme, setTheme } = useStore()
  const [step, setStep] = useState(1)

  const handleNext = () => {
    if (step === 1) {
      setStep(2)
    } else {
      setOnboarded(true)
    }
  }

  // Set selected theme on body dynamically to show preview
  const handleThemeChange = (selectedTheme: 'azravibe' | 'vscode' | 'light') => {
    setTheme(selectedTheme)
    document.body.className = `theme-${selectedTheme}`
  }

  const handleLangChange = (lang: 'fa' | 'en') => {
    setLanguage(lang)
  }

  const isFa = language === 'fa'
  const languageOptions = [
    { id: 'fa' as const, title: 'فارسی', detail: 'راست چین کامل (RTL)' },
    { id: 'en' as const, title: 'English', detail: 'Standard layout (LTR)' }
  ]
  const themeOptions = [
    { id: 'azravibe' as const, title: isFa ? 'Azravibe Dark' : 'Azravibe Dark', detail: isFa ? 'پوسته اصلی آذراوایب' : 'Primary Azravibe theme' },
    { id: 'vscode' as const, title: isFa ? 'Classic Dark' : 'Classic Dark', detail: isFa ? 'پوسته تیره کلاسیک' : 'Classic dark editor theme' },
    { id: 'light' as const, title: isFa ? 'Modern Light' : 'Modern Light', detail: isFa ? 'پوسته روشن ساده' : 'Clean light theme' }
  ]

  return (
    <div 
      className="fixed inset-0 z-[9999] bg-bg select-none"
      dir={isFa ? 'rtl' : 'ltr'}
    >
      <div className={`absolute top-0 ${isFa ? 'left-0' : 'right-0'} z-10 flex h-10 items-center no-drag`}>
        <button
          onClick={() => window.api.minimize()}
          className="flex h-10 w-11 items-center justify-center text-neutral-500 hover:bg-editor hover:text-txt"
          title={isFa ? 'کوچک کردن' : 'Minimize'}
        >
          <Minus size={14} />
        </button>
        <button
          onClick={() => window.api.maximize()}
          className="flex h-10 w-11 items-center justify-center text-neutral-500 hover:bg-editor hover:text-txt"
          title={isFa ? 'بزرگ کردن' : 'Maximize'}
        >
          <Square size={12} />
        </button>
        <button
          onClick={() => window.api.close()}
          className="flex h-10 w-11 items-center justify-center text-neutral-500 hover:bg-[#c42b1c] hover:text-white"
          title={isFa ? 'بستن' : 'Close'}
        >
          <X size={15} />
        </button>
      </div>

      <main className="flex h-full items-center justify-center px-8">
        <div className="w-full max-w-[760px]">
          <header className="mb-8 flex items-center gap-4 border-b border-editor pb-5">
            <div className="text-accent">
              <BrandMark size="lg" />
            </div>
            <div className="min-w-0">
              <h1 className="text-xl font-semibold text-txt">
                {isFa ? 'خوش آمدید به azravibe_ide' : 'Welcome to azravibe_ide'}
              </h1>
              <p className="mt-1 text-xs text-neutral-500">
                {isFa ? 'تنظیمات اولیه محیط توسعه خود را مشخص کنید' : 'Choose the first-run settings for your workspace'}
              </p>
            </div>
          </header>

          <div className="mb-7 flex items-center gap-4 text-[11px]">
            <div className={`flex items-center gap-2 ${step === 1 ? 'text-accent' : 'text-neutral-500'}`}>
              <Globe size={13} />
              <span>{isFa ? '۱. زبان رابط کاربری' : '1. UI Language'}</span>
            </div>
            <div className="h-px flex-1 bg-editor" />
            <div className={`flex items-center gap-2 ${step === 2 ? 'text-accent' : 'text-neutral-500'}`}>
              <Palette size={13} />
              <span>{isFa ? '۲. پوسته کاربری' : '2. Theme'}</span>
            </div>
          </div>

          {step === 1 && (
            <section>
              <h2 className="mb-3 text-sm font-medium text-neutral-300">
                {isFa ? 'زبان مورد نظر خود را انتخاب کنید' : 'Select your preferred language'}
              </h2>
              <div className="divide-y divide-editor border-y border-editor">
                {languageOptions.map((option) => {
                  const selected = language === option.id
                  return (
                    <button
                      key={option.id}
                      onClick={() => handleLangChange(option.id)}
                      className="flex w-full items-center justify-between gap-4 py-4 text-start text-xs text-neutral-400 transition-colors hover:text-txt"
                    >
                      <span>
                        <span className={selected ? 'font-semibold text-accent' : 'font-semibold text-txt'}>{option.title}</span>
                        <span className="mt-1 block text-[11px] text-neutral-500">{option.detail}</span>
                      </span>
                      {selected && <Check size={16} className="shrink-0 text-accent" />}
                    </button>
                  )
                })}
              </div>
            </section>
          )}

          {step === 2 && (
            <section>
              <h2 className="mb-3 text-sm font-medium text-neutral-300">
                {isFa ? 'پوسته ویرایشگر خود را انتخاب کنید' : 'Choose your editor theme'}
              </h2>
              <div className="divide-y divide-editor border-y border-editor">
                {themeOptions.map((option) => {
                  const selected = theme === option.id
                  return (
                    <button
                      key={option.id}
                      onClick={() => handleThemeChange(option.id)}
                      className="flex w-full items-center justify-between gap-4 py-4 text-start text-xs text-neutral-400 transition-colors hover:text-txt"
                    >
                      <span>
                        <span className={selected ? 'font-semibold text-accent' : 'font-semibold text-txt'}>{option.title}</span>
                        <span className="mt-1 block text-[11px] text-neutral-500">{option.detail}</span>
                      </span>
                      {selected && <Check size={16} className="shrink-0 text-accent" />}
                    </button>
                  )
                })}
              </div>
            </section>
          )}

          <footer className="mt-8 flex items-center justify-between border-t border-editor pt-4">
            <div>
              {step === 2 && (
                <button
                  onClick={() => setStep(1)}
                  className="text-xs text-neutral-500 transition-colors hover:text-txt"
                >
                  {isFa ? 'قبلی' : 'Back'}
                </button>
              )}
            </div>

            <button
              onClick={handleNext}
              className="flex items-center gap-2 border border-editor px-4 py-2 text-xs font-semibold text-accent transition-colors hover:border-accent hover:bg-editor"
            >
              <span>{step === 1 ? (isFa ? 'بعدی' : 'Next') : (isFa ? 'ورود به ویرایشگر' : 'Start Coding')}</span>
              <ChevronRight size={14} className={isFa ? 'rotate-180' : ''} />
            </button>
          </footer>
        </div>
      </main>
    </div>
  )
}
