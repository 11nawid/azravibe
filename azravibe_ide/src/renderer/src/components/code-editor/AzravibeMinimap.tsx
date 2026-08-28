import { useEffect, useMemo, useRef, useState } from 'react'
import { EditorView } from '@codemirror/view'
import { minimapColors, tokenizeMiniLine } from './azravibe-language'

interface AzravibeMinimapProps {
  content: string
  view: EditorView | null
  theme: 'azravibe' | 'vscode' | 'light'
  isRtl: boolean
}

export default function AzravibeMinimap({
  content,
  view,
  theme,
  isRtl
}: AzravibeMinimapProps) {
  const minimapRef = useRef<HTMLDivElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const [viewport, setViewport] = useState({ top: 0, height: 0 })
  const lines = useMemo(() => content.split(/\r?\n/), [content])
  const isLight = theme === 'light'
  const background = isLight ? '#f8fafc' : theme === 'vscode' ? '#101820' : '#071018'
  const border = isLight ? '#dbe4ef' : '#132536'
  const viewportColor = isLight ? 'rgba(0, 122, 204, 0.18)' : 'rgba(0, 245, 255, 0.14)'
  const viewportBorder = isLight ? 'rgba(0, 122, 204, 0.42)' : 'rgba(0, 245, 255, 0.38)'

  useEffect(() => {
    const canvas = canvasRef.current
    const container = minimapRef.current
    if (!canvas || !container) return

    const draw = () => {
      const width = Math.max(1, container.clientWidth)
      const height = Math.max(1, container.clientHeight)
      const dpr = window.devicePixelRatio || 1
      const ctx = canvas.getContext('2d')
      if (!ctx) return

      canvas.width = Math.floor(width * dpr)
      canvas.height = Math.floor(height * dpr)
      canvas.style.width = `${width}px`
      canvas.style.height = `${height}px`
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
      ctx.clearRect(0, 0, width, height)

      const paddingX = 5
      const paddingY = 4
      const usableWidth = Math.max(1, width - paddingX * 2)
      const usableHeight = Math.max(1, height - paddingY * 2)
      const lineStep = usableHeight / Math.max(lines.length, 1)
      const tokenHeight = Math.max(1, Math.min(2, lineStep * 0.78))
      const miniFontSize = Math.max(2, Math.min(5, lineStep * 0.72))
      const drawText = lineStep >= 3.2
      ctx.font = `${miniFontSize}px ${isRtl ? 'Tahoma, Arial, sans-serif' : 'Consolas, monospace'}`
      ctx.textBaseline = 'top'
      ctx.direction = isRtl ? 'rtl' : 'ltr'
      ctx.textAlign = isRtl ? 'right' : 'left'

      lines.forEach((line, lineIndex) => {
        const y = paddingY + lineIndex * lineStep
        if (y > height) return

        const leading = line.match(/^\s*/)?.[0].length || 0
        const indent = Math.min(usableWidth * 0.42, leading * 1.15)
        let x = isRtl ? width - paddingX - indent : paddingX + indent

        for (const token of tokenizeMiniLine(line)) {
          const tokenText = token.text
          const tokenWidth = Math.min(
            usableWidth,
            drawText ? Math.max(2, ctx.measureText(tokenText || '00').width) : token.width * 0.82
          )
          if (tokenWidth <= 0) continue
          ctx.fillStyle = minimapColors[token.kind]
          ctx.globalAlpha = token.kind === 'plain' ? 0.32 : 0.86

          if (isRtl) {
            x -= tokenWidth
            if (x < paddingX) break
            if (drawText) ctx.fillText(tokenText || ' ', x + tokenWidth, y)
            else ctx.fillRect(x, y, tokenWidth, tokenHeight)
            x -= 1
          } else {
            if (x > width - paddingX) break
            if (drawText) ctx.fillText(tokenText || ' ', x, y)
            else ctx.fillRect(x, y, Math.min(tokenWidth, width - paddingX - x), tokenHeight)
            x += tokenWidth + 1
          }
        }
      })

      ctx.globalAlpha = 1
    }

    draw()
    const resizeObserver = new ResizeObserver(draw)
    resizeObserver.observe(container)
    return () => resizeObserver.disconnect()
  }, [lines, isRtl, theme])

  useEffect(() => {
    if (!view) return

    const updateViewport = () => {
      const scroller = view.scrollDOM
      const trackHeight = minimapRef.current?.clientHeight || 1
      const scrollHeight = Math.max(scroller.scrollHeight, 1)
      const clientHeight = Math.max(scroller.clientHeight, 1)
      const maxScrollTop = Math.max(0, scrollHeight - clientHeight)
      const thumbHeight = Math.min(trackHeight, Math.max(22, (clientHeight / scrollHeight) * trackHeight))
      const maxTop = Math.max(0, trackHeight - thumbHeight)
      const top = maxScrollTop === 0 ? 0 : (scroller.scrollTop / maxScrollTop) * maxTop

      setViewport({
        top: Math.max(0, Math.min(maxTop, top)),
        height: thumbHeight
      })
    }

    updateViewport()
    view.scrollDOM.addEventListener('scroll', updateViewport)
    const resizeObserver = new ResizeObserver(updateViewport)
    resizeObserver.observe(view.scrollDOM)
    return () => {
      view.scrollDOM.removeEventListener('scroll', updateViewport)
      resizeObserver.disconnect()
    }
  }, [view, content])

  const scrollToPosition = (event: React.MouseEvent<HTMLDivElement>) => {
    if (!view || !minimapRef.current) return
    const rect = minimapRef.current.getBoundingClientRect()
    const trackHeight = Math.max(1, rect.height)
    const maxScrollTop = Math.max(0, view.scrollDOM.scrollHeight - view.scrollDOM.clientHeight)
    const maxTop = Math.max(0, trackHeight - viewport.height)
    const y = event.clientY - rect.top - viewport.height / 2
    const ratio = maxTop === 0 ? 0 : Math.max(0, Math.min(1, y / maxTop))
    view.scrollDOM.scrollTop = ratio * maxScrollTop
  }

  return (
    <div
      ref={minimapRef}
      className={`relative h-full w-[118px] shrink-0 overflow-hidden border-neutral-900/80 ${isRtl ? 'border-r' : 'border-l'}`}
      style={{ background, borderColor: border }}
      onMouseDown={scrollToPosition}
      onMouseMove={(event) => {
        if (event.buttons === 1) scrollToPosition(event)
      }}
      title={isRtl ? 'نقشه کوچک کد' : 'Code minimap'}
    >
      <canvas ref={canvasRef} className="absolute inset-0 h-full w-full" aria-hidden="true" />
      <div
        className="absolute inset-x-0 rounded-sm border"
        style={{
          top: `${viewport.top}px`,
          height: `${viewport.height}px`,
          backgroundColor: viewportColor,
          borderColor: viewportBorder
        }}
      />
    </div>
  )
}
