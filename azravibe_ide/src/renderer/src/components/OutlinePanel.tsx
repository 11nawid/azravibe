import React, { useMemo } from 'react'
import { useStore } from '../store/useStore'
import { Boxes, FileText } from 'lucide-react'

export default function OutlinePanel() {
  const { activePath, tabs, language } = useStore()
  const isFa = language === 'fa'

  const activeTab = tabs.find((tab) => tab.path === activePath)

  const outlineItems = useMemo(() => {
    if (!activeTab) return []
    return activeTab.content
      .split(/\r?\n/)
      .map((line, index) => ({ line: index + 1, text: line.trim() }))
      .filter((item) => /^(تابع|function|class|اگر|for|while)\b/.test(item.text))
      .slice(0, 80)
  }, [activeTab])

  return (
    <div className="w-full h-full bg-editor flex flex-col">
      {/* Header */}
      <div className="h-9 bg-sidebar border-b border-editor flex items-center px-3 select-none">
        <div className="flex items-center gap-2 text-xs font-semibold text-txt">
          <Boxes size={14} className="text-accent" />
          <span>{isFa ? 'ساختار' : 'Outline'}</span>
          {activeTab && (
            <span className="text-neutral-500 text-[10px]">({activeTab.name})</span>
          )}
        </div>
      </div>

      {/* Content */}
      <div className="flex-1 overflow-y-auto">
        {!activeTab || outlineItems.length === 0 ? (
          <div className="flex flex-col items-center justify-center h-full text-neutral-500">
            <Boxes size={48} className="mb-4 opacity-20" />
            <p className="text-xs">
              {isFa ? 'هیچ نمادی در فایل فعال نیست' : 'No symbols in active file'}
            </p>
            <p className="text-[10px] text-neutral-600 mt-1">
              {isFa ? 'تابع‌ها، کلاس‌ها و حلقه‌ها اینجا نمایش داده می‌شوند' : 'Functions, classes, and loops will appear here'}
            </p>
          </div>
        ) : (
          <div className="divide-y divide-editor border-t border-editor">
            {outlineItems.map((item, index) => (
              <div
                key={`${item.line}:${item.text}`}
                className="px-3 py-2 hover:bg-sidebar transition-colors cursor-pointer group"
              >
                <div className="flex items-center gap-2">
                  <FileText size={10} className="text-accent opacity-60" />
                  <div className="flex-1 min-w-0">
                    <div className="text-xs text-txt font-medium truncate">{item.text}</div>
                    <div className="text-[10px] text-neutral-500 mt-0.5">
                      {isFa ? `سطر ${item.line}` : `Line ${item.line}`}
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}
