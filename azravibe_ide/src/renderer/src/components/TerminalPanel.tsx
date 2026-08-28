import React, { useEffect, useRef, useState } from 'react'
import { Terminal } from 'xterm'
import { FitAddon } from 'xterm-addon-fit'
import { useStore } from '../store/useStore'
import { Plus, X, Terminal as TermIcon, Copy, Clipboard, Loader2, Minimize2, ChevronDown, ChevronUp, Command } from 'lucide-react'
import Toast from './Toast'
import 'xterm/css/xterm.css'

interface SingleTerminalProps {
  id: string
  cwd: string
  isActive: boolean
  theme: 'azravibe' | 'vscode' | 'light'
}

interface TerminalPanelProps {
  compact?: boolean // Hide internal tab bar when true
}

const terminalThemes = {
  azravibe: {
    background: '#0D0D0D',
    foreground: '#CCCCCC',
    cursor: '#00F5FF',
    cursorAccent: '#0D0D0D',
    selectionBackground: 'rgba(0, 245, 255, 0.4)',
    selectionForeground: '#FFFFFF',
    black: '#000000',
    red: '#E06C75',
    green: '#98C379',
    yellow: '#D19A66',
    blue: '#61AFEF',
    magenta: '#C678DD',
    cyan: '#56B6C2',
    white: '#ABB2BF',
    brightBlack: '#5C6370',
    brightRed: '#E06C75',
    brightGreen: '#98C379',
    brightYellow: '#D19A66',
    brightBlue: '#61AFEF',
    brightMagenta: '#C678DD',
    brightCyan: '#56B6C2',
    brightWhite: '#FFFFFF'
  },
  vscode: {
    background: '#1E1E1E',
    foreground: '#D4D4D4',
    cursor: '#007ACC',
    cursorAccent: '#1E1E1E',
    selectionBackground: 'rgba(0, 122, 204, 0.4)',
    selectionForeground: '#FFFFFF',
    black: '#000000',
    red: '#CD3131',
    green: '#0DBC79',
    yellow: '#E5E510',
    blue: '#2472C8',
    magenta: '#BC3FBC',
    cyan: '#11A8CD',
    white: '#E5E5E5',
    brightBlack: '#666666',
    brightRed: '#F14C4C',
    brightGreen: '#23D18B',
    brightYellow: '#F5F543',
    brightBlue: '#3B8EEA',
    brightMagenta: '#D670D6',
    brightCyan: '#29B8DB',
    brightWhite: '#E5E5E5'
  },
  light: {
    background: '#FFFFFF',
    foreground: '#1F2937',
    cursor: '#007ACC',
    cursorAccent: '#FFFFFF',
    selectionBackground: 'rgba(0, 122, 204, 0.25)',
    selectionForeground: '#000000',
    black: '#000000',
    red: '#CD3131',
    green: '#00BC00',
    yellow: '#949800',
    blue: '#0451A5',
    magenta: '#BC05BC',
    cyan: '#0598BC',
    white: '#555555',
    brightBlack: '#666666',
    brightRed: '#CD3131',
    brightGreen: '#14CE14',
    brightYellow: '#B5BA00',
    brightBlue: '#0451A5',
    brightMagenta: '#BC05BC',
    brightCyan: '#0598BC',
    brightWhite: '#A5A5A5'
  }
}

const SingleTerminal: React.FC<SingleTerminalProps & { isLoading: boolean; setLoading: (loading: boolean) => void }> = ({ id, cwd, isActive, theme, isLoading, setLoading }) => {
  const containerRef = useRef<HTMLDivElement>(null)
  const xtermRef = useRef<Terminal | null>(null)
  const fitAddonRef = useRef<FitAddon | null>(null)
  const [contextMenu, setContextMenu] = useState<{ x: number; y: number } | null>(null)
  const [canPaste, setCanPaste] = useState(false)
  const { showToastMessage, language, terminals } = useStore()
  const isFa = language === 'fa'
  const initializedRef = useRef(false)
  const cleanupFnsRef = useRef<Array<() => void>>([])

  useEffect(() => {
    if (!containerRef.current || initializedRef.current) return
    initializedRef.current = true

    // 1. Initialize XTerm
    const term = new Terminal({
      cursorBlink: true,
      convertEol: true,
      scrollback: 10000,
      theme: {
        ...terminalThemes[theme],
        black: '#000000',
        red: '#E06C75',
        green: '#98C379',
        yellow: '#D19A66',
        blue: '#61AFEF',
        magenta: '#C678DD',
        cyan: '#56B6C2',
        white: '#ABB2BF'
      },
      fontFamily: 'Consolas, "Courier New", monospace',
      fontSize: 14,
      lineHeight: 1.3,
      letterSpacing: 0,
      drawBoldTextInBrightColors: true,
      fontWeight: 'normal',
      fontWeightBold: 'bold',
      allowTransparency: false,
      macOptionIsMeta: false,
      rightClickSelectsWord: false,
      fastScrollModifier: 'alt'
    })

    const fitAddon = new FitAddon()
    term.loadAddon(fitAddon)
    
    // Wait for DOM to be ready before opening
    requestAnimationFrame(() => {
      if (!containerRef.current) return
      
      try {
        term.open(containerRef.current)
        fitAddon.fit()

        xtermRef.current = term
        fitAddonRef.current = fitAddon
        
        // Mark as initialized in store
        const { terminals: currentTerminals } = useStore.getState()
        const updatedTerminals = currentTerminals.map(t => 
          t.id === id ? { ...t, xtermInstance: term, isInitialized: true } : t
        )
        useStore.setState({ terminals: updatedTerminals })

        // 2. Spawn process on Main Process side
        const cols = term.cols
        const rows = term.rows
        window.api.spawnTerminal(id, cols, rows, cwd)

        // 3. Connect IO streams
        const cleanupData = window.api.onTerminalData(id, (data) => {
          if (term.element && term.element.isConnected) {
            term.write(data)
            // First data received means terminal is ready
            if (isLoading) {
              setLoading(false)
            }
          }
        })
        cleanupFnsRef.current.push(cleanupData)

        const cleanupExit = window.api.onTerminalExit(id, () => {
          if (term.element && term.element.isConnected) {
            term.write('\r\n[ترمینال بسته شد]\r\n')
            setLoading(false)
          }
        })
        cleanupFnsRef.current.push(cleanupExit)

        term.onData((data) => {
          if (term.element && term.element.isConnected) {
            window.api.writeTerminal(id, data)
          }
        })

        term.attachCustomKeyEventHandler((event) => {
          if (event.type === 'keydown') {
            // Ctrl+C (when text is selected) - copy selection
            if (event.ctrlKey && (event.key === 'C' || event.key === 'c')) {
              const selection = term.getSelection()
              if (selection) {
                navigator.clipboard.writeText(selection)
                return false // Prevent passing to PTY, we handled it
              }
            }
            // Allow all other keys to pass through to the terminal
            return true
          }
          // Allow all other events (keyup, etc.)
          return true
        })

        // Context menu handling
        const handleContextMenu = async (e: MouseEvent) => {
          e.preventDefault()
          e.stopPropagation()
          
          const selection = term.getSelection()
          if (selection) {
            // If text is selected, copy it and clear selection
            navigator.clipboard.writeText(selection)
            term.clearSelection()
            term.focus()
            showToastMessage(isFa ? 'کپی شد' : 'Copied')
          } else {
            // If no text selected, check clipboard and show context menu
            try {
              const clipboardText = await navigator.clipboard.readText()
              setCanPaste(!!clipboardText && clipboardText.length > 0)
            } catch (err) {
              setCanPaste(false)
            }
            setContextMenu({ x: e.clientX, y: e.clientY })
          }
        }

        if (containerRef.current) {
          containerRef.current.addEventListener('contextmenu', handleContextMenu)
        }

        // Close context menu when clicking elsewhere
        const handleClickOutside = () => {
          setContextMenu(null)
        }
        document.addEventListener('click', handleClickOutside)

        // Cleanup function to remove event listeners
        cleanupFnsRef.current.push(() => {
          if (containerRef.current) {
            containerRef.current.removeEventListener('contextmenu', handleContextMenu)
          }
          document.removeEventListener('click', handleClickOutside)
        })

        // 4. Handle resizing
        const resizeObserver = new ResizeObserver(() => {
          if (isActive && fitAddonRef.current && xtermRef.current) {
            setTimeout(() => {
              try {
                fitAddonRef.current?.fit()
                const currentCols = xtermRef.current?.cols || 80
                const currentRows = xtermRef.current?.rows || 24
                if (currentCols > 0 && currentRows > 0) {
                  window.api.resizeTerminal(id, currentCols, currentRows)
                }
              } catch (e) {}
            }, 50)
          }
        })

        resizeObserver.observe(containerRef.current)
        
        cleanupFnsRef.current.push(() => {
          resizeObserver.disconnect()
        })

        // Focus terminal if active
        if (isActive) {
          setTimeout(() => {
            term.focus()
          }, 100)
        }
      } catch (error) {
        console.error('Failed to initialize terminal:', error)
        setLoading(false)
      }
    })

    return () => {
      // Clean up all event listeners
      cleanupFnsRef.current.forEach(cleanup => cleanup())
      cleanupFnsRef.current = []
      // DO NOT dispose terminal - keep it alive for persistence
      // Terminal will only be disposed when explicitly removed from store
    }
  }, [id])

  // Update theme when it changes
  useEffect(() => {
    const term = xtermRef.current
    if (!term) return
    term.options.theme = {
      ...terminalThemes[theme],
      black: '#000000',
      red: '#E06C75',
      green: '#98C379',
      yellow: '#D19A66',
      blue: '#61AFEF',
      magenta: '#C678DD',
      cyan: '#56B6C2',
      white: '#ABB2BF'
    }
  }, [theme])

  // Fit and focus again when activated
  useEffect(() => {
    if (isActive && fitAddonRef.current && xtermRef.current) {
      setTimeout(() => {
        try {
          // Only fit if terminal is still attached to DOM
          if (containerRef.current && xtermRef.current!.element) {
            fitAddonRef.current?.fit()
            const currentCols = xtermRef.current?.cols || 80
            const currentRows = xtermRef.current?.rows || 24
            if (currentCols > 0 && currentRows > 0) {
              window.api.resizeTerminal(id, currentCols, currentRows)
            }
            xtermRef.current?.focus()
          }
        } catch (e) {
          console.error('Error focusing terminal:', e)
        }
      }, 100)
    }
  }, [isActive, id])

  // Recover focus when window is refocused
  useEffect(() => {
    const handleWindowFocus = () => {
      if (isActive) {
        // Try to focus the container first, which will trigger xterm focus
        if (containerRef.current) {
          containerRef.current.focus()
        }
        // Also directly focus xterm
        setTimeout(() => {
          xtermRef.current?.focus()
        }, 50)
      }
    }
    window.addEventListener('focus', handleWindowFocus)
    return () => {
      window.removeEventListener('focus', handleWindowFocus)
    }
  }, [isActive])

  // Handle paste from context menu
  const handlePaste = async () => {
    try {
      const text = await navigator.clipboard.readText()
      if (text && xtermRef.current) {
        // Paste the text without adding Enter - user can press Enter manually
        window.api.writeTerminal(id, text)
      }
    } catch (err) {
      console.error('Failed to paste:', err)
    }
    setContextMenu(null)
  }

  // Handle copy from context menu
  const handleCopy = () => {
    const selection = xtermRef.current?.getSelection()
    if (selection) {
      navigator.clipboard.writeText(selection)
    }
    setContextMenu(null)
  }

  return (
    <>
      {/* Loading Overlay */}
      {isLoading && (
        <div className={`absolute inset-0 bg-bg flex items-center justify-center z-10 ${isActive ? 'block' : 'hidden'}`}>
          <div className="flex flex-col items-center gap-3">
            <Loader2 size={32} className="text-accent animate-spin" />
            <span className="text-xs text-neutral-400">{isFa ? 'در حال راه‌اندازی ترمینال...' : 'Starting terminal...'}</span>
          </div>
        </div>
      )}
      
      <div 
        ref={containerRef} 
        className={`w-full h-full text-left ${isActive ? 'block' : 'hidden'}`}
        dir="ltr" // Shells (cmd/powershell/bash) stream output in LTR
        tabIndex={0} // Make div focusable
        onFocus={() => {
          // Ensure xterm gets focus when container is focused
          xtermRef.current?.focus()
        }}
        onMouseDown={(e) => {
          // Focus terminal on click but don't stop propagation - let xterm handle selection
          xtermRef.current?.focus()
        }}
        style={{ 
          outline: 'none',
          userSelect: 'text',
          WebkitUserSelect: 'text',
          MozUserSelect: 'text',
          msUserSelect: 'text'
        }} // Remove focus outline and enable text selection
      />
      
      {/* Context Menu */}
      {contextMenu && (
        <div
          className="fixed bg-sidebar border border-neutral-700 rounded-md shadow-xl py-1 z-[99999] min-w-[150px]"
          style={{
            left: `${contextMenu.x}px`,
            top: `${contextMenu.y}px`
          }}
        >
          <button
            onClick={handleCopy}
            className="w-full px-3 py-2 text-xs text-left text-neutral-200 hover:bg-editor hover:text-accent flex items-center gap-2 transition-colors"
          >
            <Copy size={14} />
            <span>Copy</span>
          </button>
          <button
            onClick={canPaste ? handlePaste : undefined}
            disabled={!canPaste}
            className={`w-full px-3 py-2 text-xs text-left flex items-center gap-2 transition-colors ${
              canPaste 
                ? 'text-neutral-200 hover:bg-editor hover:text-accent cursor-pointer' 
                : 'text-neutral-600 cursor-not-allowed opacity-50'
            }`}
          >
            <Clipboard size={14} />
            <span>Paste</span>
          </button>
        </div>
      )}
    </>
  )
}

export default function TerminalPanel({ compact = false }: { compact?: boolean }) {
  const { terminals, activeTerminalId, addTerminal, removeTerminal, setActiveTerminalId, currentFolder, language, theme, toastMessage, showToast, hideToast, setTerminalOpen } = useStore()
  const openedInitialTerminal = useRef(false)
  const isFa = language === 'fa'

  // Show internal tabs only when compact mode is off OR when there are multiple terminals
  const showInternalTabs = !compact || terminals.length > 1

  // Add initial terminal if none open
  useEffect(() => {
    if (!openedInitialTerminal.current && terminals.length === 0) {
      openedInitialTerminal.current = true
      addTerminal(currentFolder || undefined)
    }
  }, [])

  // Function to update terminal loading state
  const setTerminalLoading = (id: string, loading: boolean) => {
    const { terminals } = useStore.getState()
    const updatedTerminals = terminals.map(term => 
      term.id === id ? { ...term, isLoading: loading } : term
    )
    useStore.setState({ terminals: updatedTerminals })
  }

  // Close/minimize terminal panel
  const handleClosePanel = () => {
    setTerminalOpen(false)
  }

  return (
    <div className="w-full h-full bg-bg flex">
      {/* Main Terminal Area */}
      <div className="flex-1 flex flex-col">
        {/* Terminal Viewports */}
        <div 
          className="flex-1 bg-[#0D0D0D] relative overflow-hidden" 
          style={{
            userSelect: 'text',
            WebkitUserSelect: 'text',
            MozUserSelect: 'text',
            msUserSelect: 'text'
          }}
        >
          {terminals.length === 0 ? (
            // Empty state placeholder when no terminals
            <div className="absolute inset-0 flex items-center justify-center">
              <div className="text-center">
                <Command size={48} className="text-neutral-700 mx-auto mb-4" />
                <p className="text-neutral-500 text-sm mb-2">{isFa ? 'هیچ ترمینالی باز نیست' : 'No terminals open'}</p>
                <p className="text-neutral-600 text-xs">{isFa ? 'برای شروع روی + کلیک کنید' : 'Click + to start a new terminal'}</p>
              </div>
            </div>
          ) : (
            terminals.map((term) => (
              <SingleTerminal
                key={term.id}
                id={term.id}
                cwd={term.cwd}
                isActive={activeTerminalId === term.id}
                theme={theme}
                isLoading={term.isLoading}
                setLoading={(loading) => setTerminalLoading(term.id, loading)}
              />
            ))
          )}
        </div>
      </div>

      {/* Terminal Instances Sidebar (Right Side) */}
      <div className="w-60 border-l border-editor bg-sidebar flex flex-col">
        {/* Sidebar Header */}
        <div className="h-9 flex items-center justify-between px-3 border-b border-editor">
          <span className="text-[11px] text-neutral-400 font-semibold uppercase tracking-wider">{isFa ? 'ترمینال‌ها' : 'TERMINALS'}</span>
          <button
            onClick={() => addTerminal(currentFolder || undefined)}
            className="p-1 hover:bg-editor/50 rounded text-neutral-400 hover:text-accent transition-colors"
            title={isFa ? 'ترمینال جدید' : 'New Terminal'}
          >
            <Plus size={14} />
          </button>
        </div>

        {/* Terminal List */}
        <div className="flex-1 overflow-y-auto py-1">
          {terminals.map((term) => {
            const isActive = activeTerminalId === term.id
            return (
              <div
                key={term.id}
                onClick={() => setActiveTerminalId(term.id)}
                className={`group flex items-center justify-between px-3 py-1.5 cursor-pointer text-xs transition-colors ${
                  isActive 
                    ? 'bg-editor/60 text-white' 
                    : 'text-neutral-400 hover:bg-editor/30 hover:text-neutral-200'
                }`}
              >
                <div className="flex items-center gap-2 flex-1 min-w-0">
                  <TermIcon size={12} className="text-neutral-500 shrink-0" />
                  <span className="truncate">{term.name}</span>
                </div>
                <button
                  onClick={(e) => {
                    e.stopPropagation()
                    removeTerminal(term.id)
                  }}
                  className="p-0.5 rounded hover:bg-neutral-800 text-neutral-500 hover:text-red-400 transition-colors opacity-0 group-hover:opacity-100 shrink-0"
                >
                  <X size={10} />
                </button>
              </div>
            )
          })}
        </div>
      </div>
      
      {/* Toast Notification */}
      <Toast 
        message={toastMessage || ''} 
        isVisible={showToast} 
        onClose={hideToast} 
      />
    </div>
  )
}
