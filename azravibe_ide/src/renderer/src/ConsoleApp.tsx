import React, { useEffect, useMemo, useRef, useState } from 'react'
import { Copy, Eraser, Loader2, PlusSquare } from 'lucide-react'

type ReplPhase = 'connecting' | 'ready' | 'more' | 'exit'
type EntryKind = 'input' | 'output' | 'system'

interface ConsoleEntry {
  id: string
  kind: EntryKind
  text: string
  prompt?: '<<' | '..'
}

function createId(prefix: string): string {
  return `${prefix}-${Date.now()}-${Math.random().toString(16).slice(2, 8)}`
}

function normalizeText(text: string): string {
  return text.replace(/\r\n?/g, '\n')
}

function buildPayload(input: string, phase: ReplPhase): string | null {
  const normalized = normalizeText(input)

  if (!normalized.trim()) {
    return phase === 'more' ? '\n' : null
  }

  let payload = normalized
  if (!payload.endsWith('\n')) {
    payload += '\n'
  }

  if (normalized.includes('\n') && !payload.endsWith('\n\n')) {
    payload += '\n'
  }

  return payload
}

function transcriptText(entries: ConsoleEntry[]): string {
  return entries
    .map((entry) => {
      if (entry.kind === 'input') {
        return `${entry.prompt ?? '<<'} ${entry.text}`
      }
      return entry.text
    })
    .join('\n')
}

export default function ConsoleApp() {
  const sessionIdRef = useRef<string>(createId('console-repl'))
  const scrollRef = useRef<HTMLDivElement | null>(null)
  const textareaRef = useRef<HTMLTextAreaElement | null>(null)

  const [entries, setEntries] = useState<ConsoleEntry[]>([
    {
      id: createId('system'),
      kind: 'system',
      text: 'کنسول آذر وایب آماده است. این نما برای فارسی RTL ساخته شده و خروجی Azravibe را مستقیم از REPL نشان می‌دهد.'
    }
  ])
  const [phase, setPhase] = useState<ReplPhase>('connecting')
  const [input, setInput] = useState('')

  const prompt = phase === 'more' ? '..' : '<<'
  const canSubmit = phase === 'ready' || phase === 'more'
  const placeholder = phase === 'more'
    ? 'ادامه بلوک را بنویسید. برای خط جدید Shift+Enter و برای پایان بلوک Enter روی خط خالی بزنید.'
    : 'کد آذر وایب را بنویسید. Enter اجرا، Shift+Enter خط جدید.'

  useEffect(() => {
    window.api.setWindowTitle('Azravibe Console')

    const sessionId = sessionIdRef.current

    const cleanupData = window.api.onReplData(sessionId, (chunk) => {
      const text = normalizeText(chunk)
      if (!text) return

      setEntries((current) => {
        const next = [...current]
        const last = next[next.length - 1]
        if (last?.kind === 'output') {
          next[next.length - 1] = { ...last, text: last.text + text }
          return next
        }
        next.push({ id: createId('output'), kind: 'output', text })
        return next
      })
    })

    const cleanupState = window.api.onReplState(sessionId, (state) => {
      setPhase(state)
      if (state === 'exit') {
        setEntries((current) => [
          ...current,
          {
            id: createId('system'),
            kind: 'system',
            text: 'جلسه REPL بسته شد. برای یک پنجره جدید از دکمه پنجره تازه استفاده کنید.'
          }
        ])
      }
    })

    void window.api.createReplSession(sessionId)

    return () => {
      cleanupData()
      cleanupState()
      window.api.killReplSession(sessionId)
    }
  }, [])

  useEffect(() => {
    if (!scrollRef.current) return
    scrollRef.current.scrollTop = scrollRef.current.scrollHeight
  }, [entries, phase])

  useEffect(() => {
    if (!textareaRef.current || phase === 'exit') return
    textareaRef.current.focus()
  }, [phase])

  const statusLabel = useMemo(() => {
    switch (phase) {
      case 'connecting':
        return 'در حال اتصال به REPL'
      case 'more':
        return 'ادامه بلوک'
      case 'exit':
        return 'جلسه بسته شد'
      default:
        return 'آماده اجرا'
    }
  }, [phase])

  const submit = () => {
    const payload = buildPayload(input, phase)
    if (!payload || phase === 'connecting' || phase === 'exit') return

    const visibleInput = normalizeText(input).trimEnd()
    if (visibleInput) {
      setEntries((current) => [
        ...current,
        {
          id: createId('input'),
          kind: 'input',
          prompt,
          text: visibleInput
        }
      ])
    }

    window.api.writeReplSession(sessionIdRef.current, payload)
    setInput('')
  }

  const copyTranscript = async () => {
    const text = transcriptText(entries)
    if (!text) return
    await navigator.clipboard.writeText(text)
  }

  return (
    <div className="azr-console-shell azr-console-text flex h-screen w-screen flex-col overflow-hidden bg-[#f6f1e8] text-[#1b1815]" dir="rtl">
      <div className="pointer-events-none absolute inset-0 overflow-hidden">
        <div className="absolute -top-24 right-[-5%] h-72 w-72 rounded-full bg-[#a6d5d2]/35 blur-3xl" />
        <div className="absolute bottom-[-8rem] left-[-4rem] h-80 w-80 rounded-full bg-[#f1d2a8]/35 blur-3xl" />
      </div>

      <header className="relative z-10 flex items-center justify-between border-b border-[#d8cfc2] bg-white/80 px-6 py-4 backdrop-blur">
        <div className="flex items-center gap-3">
          <div className="flex h-11 w-11 items-center justify-center rounded-2xl bg-[#0f766e] text-sm font-black text-white shadow-sm">
            AZR
          </div>
          <div>
            <div className="text-lg font-semibold tracking-tight">Azravibe Console</div>
            <div className="text-xs text-[#6b6258]">ترمینال نصب‌شده زبان با رندر فارسی RTL</div>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <span className="rounded-full border border-[#d8cfc2] bg-[#faf7f2] px-3 py-1 text-xs text-[#5f564c]">
            {statusLabel}
          </span>
          <button
            type="button"
            onClick={() => window.api.openConsoleWindow()}
            className="inline-flex items-center gap-2 rounded-xl border border-[#d8cfc2] bg-white px-3 py-2 text-xs font-semibold text-[#3d3730] transition hover:border-[#0f766e] hover:text-[#0f766e]"
          >
            <PlusSquare size={14} />
            <span>پنجره تازه</span>
          </button>
          <button
            type="button"
            onClick={copyTranscript}
            className="inline-flex items-center gap-2 rounded-xl border border-[#d8cfc2] bg-white px-3 py-2 text-xs font-semibold text-[#3d3730] transition hover:border-[#0f766e] hover:text-[#0f766e]"
          >
            <Copy size={14} />
            <span>کپی خروجی</span>
          </button>
          <button
            type="button"
            onClick={() => setEntries([])}
            className="inline-flex items-center gap-2 rounded-xl border border-[#d8cfc2] bg-white px-3 py-2 text-xs font-semibold text-[#3d3730] transition hover:border-[#0f766e] hover:text-[#0f766e]"
          >
            <Eraser size={14} />
            <span>پاک‌سازی</span>
          </button>
        </div>
      </header>

      <main className="relative z-10 flex min-h-0 flex-1 flex-col px-5 py-5">
        <section className="mb-4 grid gap-3 md:grid-cols-[1.4fr_minmax(280px,0.6fr)]">
          <div className="rounded-3xl border border-[#e0d6c8] bg-white/88 p-4 shadow-[0_18px_60px_rgba(49,38,24,0.08)] backdrop-blur">
            <div className="mb-2 text-sm font-semibold text-[#2f2a25]">رندر متن</div>
            <div className="text-xs leading-6 text-[#6a6258]">
              فارسی به‌صورت RTL و با فونت Vazir Code نمایش داده می‌شود. حروف لاتین و متن انگلیسی با Arial رندر می‌شوند تا کنسول تمیز و خوانا بماند.
            </div>
          </div>
          <div className="rounded-3xl border border-[#e0d6c8] bg-white/88 p-4 shadow-[0_18px_60px_rgba(49,38,24,0.08)] backdrop-blur">
            <div className="mb-2 text-sm font-semibold text-[#2f2a25]">میانبرها</div>
            <div className="space-y-1 text-xs leading-6 text-[#6a6258]">
              <div><span className="font-semibold text-[#2f2a25]">Enter</span> اجرا</div>
              <div><span className="font-semibold text-[#2f2a25]">Shift+Enter</span> خط جدید</div>
              <div><span className="font-semibold text-[#2f2a25]">Ctrl+Shift+N</span> پنجره جدید</div>
              <div><span className="font-semibold text-[#2f2a25]">Ctrl+L</span> پاک‌سازی تاریخچه</div>
            </div>
          </div>
        </section>

        <section
          ref={scrollRef}
          className="min-h-0 flex-1 overflow-y-auto rounded-[2rem] border border-[#ddcfbf] bg-white/90 p-5 shadow-[0_24px_80px_rgba(38,27,15,0.08)] backdrop-blur"
        >
          <div className="space-y-4">
            {entries.length === 0 ? (
              <div className="rounded-2xl border border-dashed border-[#d7c8b7] bg-[#fbf8f3] px-5 py-6 text-sm text-[#756b60]">
                تاریخچه پاک شد. یک دستور جدید بنویسید تا خروجی اینجا ظاهر شود.
              </div>
            ) : (
              entries.map((entry) => {
                if (entry.kind === 'input') {
                  return (
                    <div key={entry.id} className="flex justify-end">
                      <div className="max-w-[90%] rounded-[1.6rem] bg-[#0f766e] px-5 py-3 text-white shadow-sm">
                        <div className="mb-1 text-[11px] font-semibold uppercase tracking-[0.18em] text-white/70" dir="ltr">
                          {entry.prompt}
                        </div>
                        <div className="whitespace-pre-wrap text-sm leading-7 [unicode-bidi:plaintext]" dir="auto">
                          {entry.text || ' '}
                        </div>
                      </div>
                    </div>
                  )
                }

                if (entry.kind === 'system') {
                  return (
                    <div key={entry.id} className="rounded-2xl border border-[#eadfce] bg-[#faf5ee] px-4 py-3 text-sm leading-7 text-[#655b50]">
                      <div className="whitespace-pre-wrap [unicode-bidi:plaintext]" dir="auto">
                        {entry.text}
                      </div>
                    </div>
                  )
                }

                return (
                  <div key={entry.id} className="rounded-[1.6rem] border border-[#eee4d8] bg-[#fcfaf7] px-5 py-4 text-sm leading-7 text-[#221f1a]">
                    <div className="whitespace-pre-wrap [unicode-bidi:plaintext]" dir="auto">
                      {entry.text}
                    </div>
                  </div>
                )
              })
            )}
          </div>
        </section>

        <section className="mt-4 rounded-[2rem] border border-[#dbcdbd] bg-white/92 p-4 shadow-[0_18px_60px_rgba(49,38,24,0.08)] backdrop-blur">
          <div className="mb-3 flex items-center justify-between">
            <div className="text-sm font-semibold text-[#2e2923]">ورودی REPL</div>
            <div className="rounded-full bg-[#f4eee5] px-3 py-1 text-[11px] text-[#5f564c]" dir="ltr">
              {prompt}
            </div>
          </div>

          <div className="flex gap-3">
            <textarea
              ref={textareaRef}
              value={input}
              onChange={(event) => setInput(event.target.value)}
              onKeyDown={(event) => {
                if (event.ctrlKey && event.shiftKey && event.key.toLowerCase() === 'n') {
                  event.preventDefault()
                  window.api.openConsoleWindow()
                  return
                }

                if (event.ctrlKey && event.key.toLowerCase() === 'l') {
                  event.preventDefault()
                  setEntries([])
                  return
                }

                if (event.key === 'Enter' && !event.shiftKey) {
                  event.preventDefault()
                  submit()
                }
              }}
              disabled={!canSubmit}
              rows={3}
              dir="auto"
              placeholder={placeholder}
              className="azr-console-text min-h-[98px] flex-1 resize-none rounded-[1.6rem] border border-[#d9ccbc] bg-[#fbf9f5] px-5 py-4 text-sm leading-7 text-[#1f1b17] outline-none transition placeholder:text-[#91867b] focus:border-[#0f766e] focus:bg-white disabled:cursor-not-allowed disabled:opacity-60"
            />

            <button
              type="button"
              disabled={!canSubmit}
              onClick={submit}
              className="inline-flex min-w-[132px] items-center justify-center rounded-[1.6rem] bg-[#0f766e] px-5 py-4 text-sm font-semibold text-white shadow-sm transition hover:bg-[#0b5e58] disabled:cursor-not-allowed disabled:bg-[#97b8b3]"
            >
              {phase === 'connecting' ? (
                <span className="inline-flex items-center gap-2">
                  <Loader2 size={16} className="animate-spin" />
                  <span>در حال اتصال</span>
                </span>
              ) : phase === 'more' ? 'ارسال ادامه' : 'اجرا'}
            </button>
          </div>
        </section>
      </main>
    </div>
  )
}
