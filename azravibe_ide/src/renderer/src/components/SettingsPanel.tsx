import React from 'react'
import { useStore } from '../store/useStore'
import { Check, Settings, Sliders, Type } from 'lucide-react'

type Option<T extends string | number> = {
  value: T
  label: string
}

function Section({ title, icon, children }: { title: string; icon: React.ReactNode; children: React.ReactNode }) {
  return (
    <section className="space-y-2">
      <h3 className="flex items-center gap-2 text-[11px] font-semibold uppercase tracking-wide text-neutral-500">
        {icon}
        <span>{title}</span>
      </h3>
      <div className="divide-y divide-editor border-y border-editor">{children}</div>
    </section>
  )
}

function Row({ label, children }: { label: string; children: React.ReactNode }) {
  return (
    <div className="flex min-h-[44px] items-center justify-between gap-4 py-2 text-xs">
      <span className="shrink-0 text-neutral-300">{label}</span>
      <div className="min-w-0">{children}</div>
    </div>
  )
}

function Choice<T extends string | number>({
  value,
  options,
  onChange
}: {
  value: T
  options: Array<Option<T>>
  onChange: (value: T) => void
}) {
  return (
    <div className="flex flex-wrap justify-end gap-x-4 gap-y-1">
      {options.map((option) => {
        const selected = option.value === value
        return (
          <button
            key={String(option.value)}
            onClick={() => onChange(option.value)}
            className={`flex items-center gap-1 text-[11px] transition-colors ${
              selected ? 'font-semibold text-accent' : 'text-neutral-500 hover:text-txt'
            }`}
          >
            {selected && <Check size={12} />}
            <span>{option.label}</span>
          </button>
        )
      })}
    </div>
  )
}

function Switch({ checked, onChange, isFa }: { checked: boolean; onChange: () => void; isFa: boolean }) {
  return (
    <button
      onClick={onChange}
      className={`relative h-5 w-9 border transition-colors ${
        checked ? 'border-accent bg-accent/15' : 'border-editor bg-transparent'
      }`}
      aria-pressed={checked}
    >
      <span
        className={`absolute left-0 top-1/2 h-3 w-3 -translate-y-1/2 bg-current transition-transform ${
          checked ? 'text-accent' : 'text-neutral-500'
        } ${checked ? (isFa ? '-translate-x-5' : 'translate-x-5') : 'translate-x-1'}`}
      />
    </button>
  )
}

export default function SettingsPanel() {
  const {
    language,
    setLanguage,
    theme,
    setTheme,
    wordWrap,
    setWordWrap,
    fontSize,
    setFontSize,
    minimap,
    setMinimap,
    editorRtl,
    setEditorRtl,
    autoSave,
    setAutoSave,
    fontFamily,
    setFontFamily,
    compilerPath,
    setCompilerPath,
    tabSize,
    setTabSize,
    insertSpaces,
    setInsertSpaces
  } = useStore()

  const isFa = language === 'fa'

  return (
    <div className="flex h-full w-full flex-col bg-editor text-txt" dir={isFa ? 'rtl' : 'ltr'}>
      <div className="flex h-9 items-center border-b border-editor bg-sidebar px-3 text-xs font-semibold">
        <Settings size={14} className="text-accent" />
        <span className="mx-2">{isFa ? 'تنظیمات' : 'Settings'}</span>
      </div>

      <div className="flex-1 overflow-y-auto px-5 py-5">
        <div className="mx-auto max-w-3xl space-y-7">
          <Section title={isFa ? 'رابط کاربری و ظاهر' : 'Interface & Appearance'} icon={<Sliders size={12} />}>
            <Row label={isFa ? 'زبان رابط کاربری' : 'Interface Language'}>
              <Choice value={language} onChange={setLanguage} options={[{ value: 'fa', label: 'فارسی' }, { value: 'en', label: 'English' }]} />
            </Row>
            <Row label={isFa ? 'جهت متن ویرایشگر' : 'Editor Text Direction'}>
              <Choice value={editorRtl ? 'rtl' : 'ltr'} onChange={(value) => setEditorRtl(value === 'rtl')} options={[{ value: 'rtl', label: isFa ? 'راست به چپ' : 'RTL' }, { value: 'ltr', label: isFa ? 'چپ به راست' : 'LTR' }]} />
            </Row>
            <Row label={isFa ? 'پوسته رنگی' : 'Color Theme'}>
              <Choice value={theme} onChange={setTheme} options={[{ value: 'azravibe', label: 'Azravibe' }, { value: 'vscode', label: 'VS Code' }, { value: 'light', label: isFa ? 'روشن' : 'Light' }]} />
            </Row>
          </Section>

          <Section title={isFa ? 'تنظیمات ویرایشگر' : 'Editor Preferences'} icon={<Type size={12} />}>
            <Row label={isFa ? 'اندازه قلم' : 'Font Size'}>
              <div className="flex items-center gap-3">
                <span className="w-10 text-end text-[11px] text-neutral-500">{fontSize}px</span>
                <input
                  type="range"
                  min="12"
                  max="24"
                  step="1"
                  value={fontSize}
                  onChange={(e) => setFontSize(Number(e.target.value))}
                  className="h-1 w-32 accent-accent"
                />
              </div>
            </Row>
            <Row label={isFa ? 'شکستن خطوط' : 'Word Wrap'}>
              <Switch checked={wordWrap} onChange={() => setWordWrap(!wordWrap)} isFa={isFa} />
            </Row>
            <Row label={isFa ? 'نقشه کوچک' : 'Minimap'}>
              <Switch checked={minimap} onChange={() => setMinimap(!minimap)} isFa={isFa} />
            </Row>
            <Row label={isFa ? 'ذخیره خودکار' : 'Auto Save'}>
              <Switch checked={autoSave} onChange={() => setAutoSave(!autoSave)} isFa={isFa} />
            </Row>
            <Row label={isFa ? 'قلم' : 'Font Family'}>
              <input
                value={fontFamily}
                onChange={(e) => setFontFamily(e.target.value)}
                className="w-72 max-w-full border-b border-editor bg-transparent px-1 py-1 text-end text-[11px] text-txt outline-none focus:border-accent"
                dir="ltr"
              />
            </Row>
            <Row label={isFa ? 'کامپایلر' : 'Compiler'}>
              <input
                value={compilerPath}
                onChange={(e) => setCompilerPath(e.target.value)}
                className="w-72 max-w-full border-b border-editor bg-transparent px-1 py-1 text-end text-[11px] text-txt outline-none focus:border-accent"
                dir="ltr"
              />
            </Row>
            <Row label={isFa ? 'اندازه تب' : 'Tab Size'}>
              <Choice value={tabSize} onChange={setTabSize} options={[2, 4, 8].map((size) => ({ value: size, label: String(size) }))} />
            </Row>
            <Row label={isFa ? 'جایگزینی تب' : 'Insert Spaces'}>
              <Switch checked={insertSpaces} onChange={() => setInsertSpaces(!insertSpaces)} isFa={isFa} />
            </Row>
          </Section>
        </div>
      </div>
    </div>
  )
}
