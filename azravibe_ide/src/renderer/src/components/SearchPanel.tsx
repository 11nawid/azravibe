import React, { useMemo, useState } from 'react'
import { CaseSensitive, FileSearch, Replace, Search } from 'lucide-react'
import { FileNode, useStore } from '../store/useStore'

const collectFiles = (nodes: FileNode[]): FileNode[] => {
  const files: FileNode[] = []
  for (const node of nodes) {
    if (node.isDirectory) files.push(...collectFiles(node.children || []))
    else files.push(node)
  }
  return files
}

const fileName = (path: string) => path.split('\\').pop() || path

export default function SearchPanel() {
  const { fileTree, language, openFile, refreshFileTree, showToastMessage } = useStore()
  const isFa = language === 'fa'
  const files = useMemo(() => collectFiles(fileTree), [fileTree])
  const [query, setQuery] = useState('')
  const [replaceText, setReplaceText] = useState('')
  const [caseSensitive, setCaseSensitive] = useState(false)
  const [busy, setBusy] = useState(false)
  const [matches, setMatches] = useState<Array<{ path: string; line: number; col: number; preview: string }>>([])

  const findMatches = async () => {
    if (!query.trim()) {
      setMatches([])
      return
    }
    setBusy(true)
    const nextMatches: Array<{ path: string; line: number; col: number; preview: string }> = []
    const needle = caseSensitive ? query : query.toLowerCase()

    for (const item of files) {
      try {
        const content = await window.api.readFile(item.path)
        content.split(/\r?\n/).forEach((line, index) => {
          const haystack = caseSensitive ? line : line.toLowerCase()
          const col = haystack.indexOf(needle)
          if (col >= 0) {
            nextMatches.push({ path: item.path, line: index + 1, col: col + 1, preview: line.trim() })
          }
        })
      } catch {}
    }

    setMatches(nextMatches.slice(0, 600))
    setBusy(false)
  }

  const replaceMatches = async () => {
    if (!query.trim() || matches.length === 0) return
    const confirmed = window.confirm(
      isFa
        ? `جایگزینی ${matches.length} مورد در پروژه انجام شود؟`
        : `Replace ${matches.length} matches in the workspace?`
    )
    if (!confirmed) return

    setBusy(true)
    const touched = new Set(matches.map((match) => match.path))
    for (const path of touched) {
      try {
        const content = await window.api.readFile(path)
        const nextContent = caseSensitive
          ? content.split(query).join(replaceText)
          : content.replace(new RegExp(query.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'gi'), replaceText)
        await window.api.writeFile(path, nextContent)
      } catch {}
    }
    await refreshFileTree()
    await findMatches()
    showToastMessage(isFa ? 'جایگزینی انجام شد' : 'Replace complete')
    setBusy(false)
  }

  return (
    <div className="flex h-full w-full flex-col bg-editor text-txt" dir={isFa ? 'rtl' : 'ltr'}>
      <div className="flex h-9 items-center gap-2 border-b border-editor bg-sidebar px-3 text-xs font-semibold">
        <Search size={14} className="text-accent" />
        <span>{isFa ? 'جستجوی پروژه' : 'Workspace Search'}</span>
      </div>

      <div className="grid gap-3 border-b border-editor bg-bg p-3 sm:grid-cols-[1fr_1fr_auto_auto_auto]">
        <input
          value={query}
          onChange={(event) => setQuery(event.target.value)}
          onKeyDown={(event) => {
            if (event.key === 'Enter') void findMatches()
          }}
          placeholder={isFa ? 'متن جستجو' : 'Find text'}
          className="border-b border-editor bg-transparent px-1 py-2 text-xs text-txt outline-none focus:border-accent"
        />
        <input
          value={replaceText}
          onChange={(event) => setReplaceText(event.target.value)}
          placeholder={isFa ? 'جایگزین با' : 'Replace with'}
          className="border-b border-editor bg-transparent px-1 py-2 text-xs text-txt outline-none focus:border-accent"
        />
        <button
          onClick={() => setCaseSensitive(!caseSensitive)}
          className={`flex items-center justify-center gap-1 border px-3 py-2 text-xs font-semibold ${
            caseSensitive ? 'border-accent bg-accent/10 text-accent' : 'border-editor bg-transparent text-neutral-400 hover:text-txt'
          }`}
          title={isFa ? 'حساس به بزرگی و کوچکی حروف' : 'Match case'}
        >
          <CaseSensitive size={14} />
        </button>
        <button onClick={findMatches} className="flex items-center justify-center gap-1 border border-accent px-3 py-2 text-xs font-semibold text-accent hover:bg-accent/10">
          <FileSearch size={14} />
          <span>{busy ? (isFa ? 'در حال جستجو...' : 'Searching...') : (isFa ? 'جستجو' : 'Find')}</span>
        </button>
        <button onClick={replaceMatches} className="flex items-center justify-center gap-1 border border-editor px-3 py-2 text-xs font-semibold text-neutral-300 hover:bg-bg hover:text-txt">
          <Replace size={14} />
          <span>{isFa ? 'جایگزینی' : 'Replace'}</span>
        </button>
      </div>

      <div className="flex-1 overflow-y-auto">
        {matches.length === 0 && (
          <div className="px-3 py-10 text-center text-xs text-neutral-500">
            {query.trim() ? (isFa ? 'موردی پیدا نشد.' : 'No matches found.') : (isFa ? 'عبارت مورد نظر را وارد کنید.' : 'Enter a search term.')}
          </div>
        )}
        {matches.map((match, index) => (
          <button
            key={`${match.path}:${match.line}:${match.col}:${index}`}
            className="block w-full border-b border-editor px-3 py-2 text-left text-xs hover:bg-bg"
            onClick={() => void openFile(match.path, fileName(match.path))}
            dir="ltr"
          >
            <div className="font-semibold text-txt">{fileName(match.path)}:{match.line}:{match.col}</div>
            <div className="truncate text-[10px] text-neutral-500">{match.preview}</div>
          </button>
        ))}
      </div>
    </div>
  )
}
