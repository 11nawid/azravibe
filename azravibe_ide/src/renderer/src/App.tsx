import React, { useEffect } from 'react'
import Titlebar from './components/Titlebar'
import Layout from './components/Layout'
import Statusbar from './components/Statusbar'
import Onboarding from './components/Onboarding'
import SettingsModal from './components/SettingsModal'
import WorkbenchOverlays from './components/WorkbenchOverlays'
import { useStore } from './store/useStore'
import ConsoleApp from './ConsoleApp'

export default function App() {
  const mode = new URLSearchParams(window.location.search).get('mode')
  const { 
    saveFile, 
    reopenClosedTab,
    activePath, 
    addTerminal, 
    theme, 
    onboarded, 
    language,
    currentFolder,
    refreshFileTree,
    fontSize,
    setFontSize,
    setSidebarOpen,
    sidebarOpen,
    setTerminalOpen,
    terminalOpen,
    setShowCommandPalette,
    setShowQuickOpen,
    setShowSearchPanel,
    setShowProblemsPanel,
    setShowOutlinePanel,
    setShowSettingsModal
  } = useStore()

  // Apply theme dynamically to body element
  useEffect(() => {
    document.body.className = `theme-${theme}`
  }, [theme])

  // Load tree initially if there's a saved workspace
  useEffect(() => {
    if (currentFolder) {
      refreshFileTree()
    }
  }, [])

  // Open .azr files launched from Explorer/file association.
  useEffect(() => {
    const cleanup = window.api.onOpenFilePath(async (filePath) => {
      const state = useStore.getState()
      const name = filePath.split('\\').pop() || filePath
      const folder = filePath.slice(0, Math.max(0, filePath.lastIndexOf('\\')))

      if (folder && state.currentFolder !== folder) {
        await state.openFolder(folder)
      } else {
        await state.refreshFileTree()
      }

      await state.openFile(filePath, name)
    })

    return cleanup
  }, [])

  // Handle global keyboard shortcuts matching VS Code
  useEffect(() => {
    const handleKeyDown = async (e: KeyboardEvent) => {
      const state = useStore.getState()

      // Check if we're currently focused on the terminal
      // If so, don't intercept most shortcuts (except critical ones)
      const activeElement = document.activeElement
      const isTerminalFocused = activeElement?.classList.contains('xterm') || 
                                activeElement?.closest('.xterm') ||
                                activeElement?.closest('[class*="xterm"]')

      // Ctrl+O: Open workspace folder (always allowed)
      if (e.ctrlKey && e.key.toLowerCase() === 'o') {
        e.preventDefault()
        const path = await window.api.selectDirectory()
        if (path) {
          await state.openFolder(path)
        }
        return
      }

      // Ctrl+`: Toggle integrated terminal (always allowed)
      if (e.ctrlKey && e.key === '`') {
        e.preventDefault()
        const nextOpen = !state.terminalOpen
        state.setTerminalOpen(nextOpen)
        if (nextOpen && state.terminals.length === 0) {
          state.addTerminal()
        }
        return
      }

      // Ctrl+B: Toggle sidebar (always allowed)
      if (e.ctrlKey && e.key.toLowerCase() === 'b') {
        e.preventDefault()
        state.setSidebarOpen(!state.sidebarOpen)
        return
      }

      // Ctrl+J: Toggle panel (terminal) (always allowed)
      if (e.ctrlKey && e.key.toLowerCase() === 'j') {
        e.preventDefault()
        const nextOpen = !state.terminalOpen
        state.setTerminalOpen(nextOpen)
        if (nextOpen && state.terminals.length === 0) {
          state.addTerminal()
        }
        return
      }

      // If terminal is focused, only allow specific shortcuts, ignore the rest
      if (isTerminalFocused) {
        // Allow these shortcuts even when terminal is focused
        // Ctrl+W: Close active tab
        if (e.ctrlKey && e.key.toLowerCase() === 'w') {
          e.preventDefault()
          if (state.activePath) {
            state.closeFile(state.activePath)
          }
          return
        }

        // F5: Run in terminal
        if (e.key === 'F5') {
          e.preventDefault()
          if (activePath) {
            const termId = useStore.getState().activeTerminalId || addTerminal()
            await saveFile(activePath)
            window.api.writeTerminal(termId, `${useStore.getState().compilerPath} "${activePath}"\r`)
          }
          return
        }

        // For all other keys when terminal is focused, let them pass through to terminal
        return
      }

      // Below shortcuts only work when NOT in terminal
      // Ctrl+N: Create and open a new Azravibe file
      if (e.ctrlKey && e.key.toLowerCase() === 'n') {
        e.preventDefault()
        const folder = state.currentFolder
        if (folder) {
          const name = state.language === 'fa' ? `پرونده_${Date.now()}.azr` : `new_file_${Date.now()}.azr`
          const fullPath = `${folder}\\${name}`
          await window.api.createFile(fullPath)
          await state.refreshFileTree()
          await state.openFile(fullPath, name)
        }
      }

      // Ctrl+W: Close active tab
      if (e.ctrlKey && e.key.toLowerCase() === 'w') {
        e.preventDefault()
        if (state.activePath) {
          state.closeFile(state.activePath)
        }
      }

      // Ctrl+S: Save file
      if (e.ctrlKey && e.key === 's') {
        e.preventDefault()
        if (activePath) {
          await saveFile(activePath)
        }
      }

      // Ctrl+Shift+P: Command palette
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'p') {
        e.preventDefault()
        state.setShowCommandPalette(true)
      }

      // Ctrl+P: Quick file open
      if (e.ctrlKey && !e.shiftKey && e.key.toLowerCase() === 'p') {
        e.preventDefault()
        state.setShowQuickOpen(true)
      }

      // Ctrl+Shift+F: Workspace search
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'f') {
        e.preventDefault()
        state.setShowSearchPanel(true)
      }

      // Ctrl+Shift+M: Problems
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'm') {
        e.preventDefault()
        state.setShowProblemsPanel(true)
      }

      // Ctrl+Shift+O: Outline
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'o') {
        e.preventDefault()
        state.setShowOutlinePanel(true)
      }

      // Ctrl+,: Settings
      if (e.ctrlKey && e.key === ',') {
        e.preventDefault()
        state.setShowSettingsModal(true)
      }

      // F5: Run in terminal
      if (e.key === 'F5') {
        e.preventDefault()
        if (activePath) {
          const termId = useStore.getState().activeTerminalId || addTerminal()
          await saveFile(activePath)
          window.api.writeTerminal(termId, `${useStore.getState().compilerPath} "${activePath}"\r`)
        }
      }

      // Ctrl+Tab: Next tab
      if (e.ctrlKey && e.key === 'Tab' && !e.shiftKey) {
        e.preventDefault()
        const currentIndex = state.tabs.findIndex(tab => tab.path === state.activePath)
        if (currentIndex >= 0 && currentIndex < state.tabs.length - 1) {
          state.setActivePath(state.tabs[currentIndex + 1].path)
        } else if (state.tabs.length > 0) {
          state.setActivePath(state.tabs[0].path)
        }
      }

      // Ctrl+Shift+Tab: Previous tab
      if (e.ctrlKey && e.shiftKey && e.key === 'Tab') {
        e.preventDefault()
        const currentIndex = state.tabs.findIndex(tab => tab.path === state.activePath)
        if (currentIndex > 0) {
          state.setActivePath(state.tabs[currentIndex - 1].path)
        } else if (state.tabs.length > 0) {
          state.setActivePath(state.tabs[state.tabs.length - 1].path)
        }
      }

      // Ctrl+K Ctrl+C is reserved for future chord handling.
      if (e.ctrlKey && !e.shiftKey && e.key.toLowerCase() === 'k') {
        return
      }

      // Ctrl+/: Toggle line comment
      if (e.ctrlKey && e.key === '/') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'toggleComment' }))
      }

      // Ctrl+Z: Undo (handled by editor, but we can add global handler if needed)
      // Ctrl+Y or Ctrl+Shift+Z: Redo (handled by editor)

      // Ctrl+X: Cut (handled by editor)
      // Ctrl+C: Copy (handled by editor)
      // Ctrl+V: Paste (handled by editor)

      // Ctrl+A: Select All (handled by editor)

      // Ctrl+F: Find in file
      if (e.ctrlKey && !e.shiftKey && e.key.toLowerCase() === 'f') {
        e.preventDefault()
        // Trigger the editor find panel
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'find' }))
      }

      // Ctrl+H: Replace in file
      if (e.ctrlKey && e.key.toLowerCase() === 'h') {
        e.preventDefault()
        // Trigger the editor replace panel
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'replace' }))
      }

      // Ctrl+D: Add selection to next find match
      if (e.ctrlKey && e.key.toLowerCase() === 'd') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'addSelectionToNextFindMatch' }))
      }

      // Alt+Up/Down: Move line up/down
      if (e.altKey && (e.key === 'ArrowUp' || e.key === 'ArrowDown')) {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: e.key === 'ArrowUp' ? 'moveLineUp' : 'moveLineDown' }))
      }

      // Shift+Alt+Up/Down: Copy line up/down
      if (e.shiftKey && e.altKey && (e.key === 'ArrowUp' || e.key === 'ArrowDown')) {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: e.key === 'ArrowUp' ? 'copyLineUp' : 'copyLineDown' }))
      }

      // Ctrl+L: Select current line
      if (e.ctrlKey && e.key.toLowerCase() === 'l') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'selectLine' }))
      }

      // Ctrl+Shift+K: Delete line
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'k') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'deleteLine' }))
      }

      // Ctrl+Enter: Insert line below
      if (e.ctrlKey && e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'insertLineBelow' }))
      }

      // Ctrl+Shift+Enter: Insert line above
      if (e.ctrlKey && e.shiftKey && e.key === 'Enter') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'insertLineAbove' }))
      }

      // Ctrl+Shift+\: Jump to matching bracket
      if (e.ctrlKey && e.shiftKey && e.key === '\\') {
        e.preventDefault()
        window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: 'jumpToMatchingBracket' }))
      }

      // Ctrl+Shift+E: Focus explorer (sidebar)
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'e') {
        e.preventDefault()
        if (!state.sidebarOpen) {
          state.setSidebarOpen(true)
        }
      }

      // Ctrl+Shift+T: Reopen closed tab
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 't') {
        e.preventDefault()
        state.reopenClosedTab()
      }

      // Ctrl+K S is reserved for future save-all chord handling.
    };

    window.addEventListener('keydown', handleKeyDown)
    return () => window.removeEventListener('keydown', handleKeyDown)
  }, [activePath])

  // Handle Ctrl + Scroll Wheel for zooming (like VSCode)
  useEffect(() => {
    const handleWheel = (e: WheelEvent) => {
      if (e.ctrlKey) {
        e.preventDefault()
        const state = useStore.getState()
        const currentSize = state.fontSize
        
        // Scroll up (negative deltaY) = zoom in, Scroll down (positive deltaY) = zoom out
        if (e.deltaY < 0) {
          // Zoom in - increase font size
          const newSize = Math.min(currentSize + 1, 72) // Max 72px
          setFontSize(newSize)
        } else {
          // Zoom out - decrease font size
          const newSize = Math.max(currentSize - 1, 8) // Min 8px
          setFontSize(newSize)
        }
      }
    }

    // Add wheel event listener with passive: false to allow preventDefault
    window.addEventListener('wheel', handleWheel, { passive: false })
    return () => window.removeEventListener('wheel', handleWheel)
  }, [setFontSize])

  const isFa = language === 'fa'

  if (mode === 'console') {
    return <ConsoleApp />
  }

  return (
    <div 
      className="h-screen w-screen flex flex-col overflow-hidden bg-bg text-txt" 
      dir={isFa ? 'rtl' : 'ltr'}
    >
      {/* Onboarding Overlay Screen */}
      {!onboarded && <Onboarding />}

      {/* Settings Modal Overlay */}
      <SettingsModal />

      {/* Command palette, quick-open, search, problems and outline */}
      <WorkbenchOverlays />

      {/* Titlebar */}
      <Titlebar />

      {/* Main Workspace Resizable Panels */}
      <Layout />

      {/* Status Bar */}
      <Statusbar />
    </div>
  )
}
