interface SidebarDropPromptProps {
  dropPrompt: { sources: string[]; targetDir: string } | null
  isFa: boolean
  onMove: () => void
  onCopy: () => void
  onClose: () => void
}

export default function SidebarDropPrompt({
  dropPrompt,
  isFa,
  onMove,
  onCopy,
  onClose
}: SidebarDropPromptProps) {
  if (!dropPrompt) return null

  return (
    <div className="fixed inset-0 z-[100000] flex items-center justify-center bg-black/45" onMouseDown={onClose}>
      <div
        className="w-[min(360px,calc(100vw-32px))] rounded-md border border-neutral-700 bg-[#252526] p-4 text-left shadow-2xl"
        dir={isFa ? 'rtl' : 'ltr'}
        onMouseDown={(event) => event.stopPropagation()}
      >
        <div className="mb-2 text-sm font-semibold text-white">
          {isFa ? 'انتقال یا کپی؟' : 'Move or copy items?'}
        </div>
        <div className="mb-4 text-xs leading-relaxed text-neutral-300">
          {isFa
            ? `${dropPrompt.sources.length} مورد به "${dropPrompt.targetDir.split('\\').pop()}" کشیده شد.`
            : `${dropPrompt.sources.length} item${dropPrompt.sources.length === 1 ? '' : 's'} dropped into "${dropPrompt.targetDir.split('\\').pop()}".`}
        </div>
        <div className={`flex gap-2 ${isFa ? 'justify-start' : 'justify-end'}`}>
          <button
            onClick={onMove}
            className="rounded bg-[#0e639c] px-3 py-1.5 text-xs font-semibold text-white hover:bg-[#1177bb]"
          >
            {isFa ? 'انتقال' : 'Move'}
          </button>
          <button
            onClick={onCopy}
            className="rounded bg-neutral-700 px-3 py-1.5 text-xs font-semibold text-white hover:bg-neutral-600"
          >
            {isFa ? 'کپی' : 'Copy'}
          </button>
          <button
            onClick={onClose}
            className="rounded px-3 py-1.5 text-xs font-semibold text-neutral-300 hover:bg-neutral-700"
          >
            {isFa ? 'هیچ‌کدام' : 'Do Nothing'}
          </button>
        </div>
      </div>
    </div>
  )
}
