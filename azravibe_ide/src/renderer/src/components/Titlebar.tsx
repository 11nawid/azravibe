import React, { useState, useEffect, useRef } from 'react'
import { useStore } from '../store/useStore'
import { 
  Minimize2, 
  Square, 
  X, 
  FolderOpen, 
  Save, 
  FilePlus, 
  FolderPlus, 
  Terminal, 
  Play, 
  Settings,
  PanelLeft,
  Info,
  AlertCircle,
  Plus,
  Monitor
} from 'lucide-react'
import BrandMark from './BrandMark'

type MenuItem = {
  label?: string
  icon?: React.ReactNode
  action?: () => void
  disabled?: boolean
  divider?: boolean
}

export default function Titlebar() {
  const { 
    currentFolder, 
    openFolder, 
    closeFolder,
    activePath, 
    saveFile, 
    openFile,
    addTerminal, 
    language, 
    sidebarOpen,
    setSidebarOpen,
    setShowSettingsModal,
    setProblemsPanelVisible,
    problemsPanelVisible,
    setTerminalPanelVisible,
    terminalPanelVisible,
    setActiveBottomPanel,
    problemsPanelOpen,
    setProblemsPanelOpen,
    setOutputPanelVisible,
    outputPanelVisible,
    setAboutPanelOpen
  } = useStore()

  const [activeMenu, setActiveMenu] = useState<string | null>(null)
  const [menuOpen, setMenuOpen] = useState(false)
  const menuRef = useRef<HTMLDivElement>(null)

  // Handle clicking outside to close menus
  useEffect(() => {
    function handleClickOutside(event: MouseEvent) {
      if (menuRef.current && !menuRef.current.contains(event.target as Node)) {
        setActiveMenu(null)
        setMenuOpen(false)
      }
    }
    document.addEventListener('mousedown', handleClickOutside)
    return () => document.removeEventListener('mousedown', handleClickOutside)
  }, [])

  const handleOpenFolder = async () => {
    const path = await window.api.selectDirectory()
    if (path) {
      await openFolder(path)
    }
    setActiveMenu(null)
    setMenuOpen(false)
  }

  const handleSave = async () => {
    if (activePath) {
      await saveFile(activePath)
    }
    setActiveMenu(null)
    setMenuOpen(false)
  }

  const runEditorCommand = (command: 'undo' | 'redo' | 'cut' | 'copy' | 'paste') => {
    window.dispatchEvent(new CustomEvent('azravibe-editor-command', { detail: command }))
  }

  const handleNewFile = async () => {
    if (!currentFolder) return
    const name = language === 'fa' ? `فایل_جدید_${Date.now()}.azr` : `new_file_${Date.now()}.azr`
    const fullPath = `${currentFolder}\\${name}`
    await window.api.createFile(fullPath)
    await useStore.getState().refreshFileTree()
    await openFile(fullPath, name)
    setActiveMenu(null)
    setMenuOpen(false)
  }

  const handleNewFolder = async () => {
    if (!currentFolder) return
    const name = language === 'fa' ? `پوشه_جدید_${Date.now()}` : `new_folder_${Date.now()}`
    const fullPath = `${currentFolder}\\${name}`
    await window.api.createDirectory(fullPath)
    useStore.getState().refreshFileTree()
    setActiveMenu(null)
    setMenuOpen(false)
  }

  const handleRun = () => {
    if (activePath) {
      const termId = useStore.getState().activeTerminalId || addTerminal()
      saveFile(activePath).then(() => {
        window.api.writeTerminal(termId, `azr "${activePath}"\r`)
      })
    }
    setActiveMenu(null)
    setMenuOpen(false)
  }

  const isFa = language === 'fa'

  const menus: Array<{ id: string; label: string; items: MenuItem[] }> = [
    {
      id: 'file',
      label: isFa ? 'فایل' : 'File',
      items: [
        { label: isFa ? 'باز کردن پوشه...' : 'Open Folder...', icon: <FolderOpen size={13} />, action: handleOpenFolder },
        { label: isFa ? 'بستن پوشه' : 'Close Folder', icon: <FolderOpen size={13} />, action: () => closeFolder(), disabled: !currentFolder },
        { label: isFa ? 'پرونده جدید' : 'New File', icon: <FilePlus size={13} />, action: handleNewFile, disabled: !currentFolder },
        { label: isFa ? 'پوشه جدید' : 'New Folder', icon: <FolderPlus size={13} />, action: handleNewFolder, disabled: !currentFolder },
        { label: isFa ? 'ذخیره (Ctrl+S)' : 'Save (Ctrl+S)', icon: <Save size={13} />, action: handleSave, disabled: !activePath },
        { label: isFa ? 'خروج' : 'Exit App', icon: <X size={13} />, action: () => window.api.close() }
      ]
    },
    {
      id: 'edit',
      label: isFa ? 'ویرایش' : 'Edit',
      items: [
        { label: isFa ? 'واگرد (Ctrl+Z)' : 'Undo (Ctrl+Z)', action: () => runEditorCommand('undo'), disabled: !activePath },
        { label: isFa ? 'انجام مجدد (Ctrl+Y)' : 'Redo (Ctrl+Y)', action: () => runEditorCommand('redo'), disabled: !activePath },
        { label: isFa ? 'برش (Ctrl+X)' : 'Cut (Ctrl+X)', action: () => runEditorCommand('cut'), disabled: !activePath },
        { label: isFa ? 'کپی (Ctrl+C)' : 'Copy (Ctrl+C)', action: () => runEditorCommand('copy'), disabled: !activePath },
        { label: isFa ? 'چسباندن (Ctrl+V)' : 'Paste (Ctrl+V)', action: () => runEditorCommand('paste'), disabled: !activePath }
      ]
    },
    {
      id: 'view',
      label: isFa ? 'نمایش' : 'View',
      items: [
        { 
          label: isFa 
            ? (sidebarOpen ? 'پنهان‌سازی نوار کناری' : 'نمایش نوار کناری') 
            : (sidebarOpen ? 'Hide Sidebar' : 'Show Sidebar'), 
          icon: <PanelLeft size={13} />, 
          action: () => setSidebarOpen(!sidebarOpen) 
        },
        { divider: true },
        { 
          label: isFa 
            ? (terminalPanelVisible ? 'پنهان‌سازی پایانه' : 'نمایش پایانه') 
            : (terminalPanelVisible ? 'Hide Terminal' : 'Show Terminal'), 
          icon: <Terminal size={13} />, 
          action: () => {
            if (!terminalPanelVisible) {
              setTerminalPanelVisible(true)
              setActiveBottomPanel('terminal')
              setProblemsPanelOpen(true)
            } else {
              setTerminalPanelVisible(false)
            }
          }
        },
        { 
          label: isFa 
            ? (problemsPanelVisible ? 'پنهان‌سازی مشکلات' : 'نمایش مشکلات') 
            : (problemsPanelVisible ? 'Hide Problems' : 'Show Problems'), 
          icon: <AlertCircle size={13} className="text-red-400" />, 
          action: () => {
            if (!problemsPanelVisible) {
              setProblemsPanelVisible(true)
              setActiveBottomPanel('problems')
              setProblemsPanelOpen(true)
            } else {
              setProblemsPanelVisible(false)
            }
          }
        },
        { 
          label: isFa 
            ? (outputPanelVisible ? 'پنهان‌سازی خروجی' : 'نمایش خروجی') 
            : (outputPanelVisible ? 'Hide Output' : 'Show Output'), 
          icon: <Monitor size={13} className="text-green-400" />, 
          action: () => {
            if (!outputPanelVisible) {
              setOutputPanelVisible(true)
              setActiveBottomPanel('output')
              setProblemsPanelOpen(true)
            } else {
              setOutputPanelVisible(false)
            }
          }
        },
        { divider: true },
        { 
          label: isFa ? 'ترمینال جدید' : 'New Terminal', 
          icon: <Plus size={13} />, 
          action: () => {
            addTerminal(currentFolder || undefined)
            if (!terminalPanelVisible) {
              setTerminalPanelVisible(true)
              setActiveBottomPanel('terminal')
              setProblemsPanelOpen(true)
            }
          }
        }
      ]
    },
    {
      id: 'run',
      label: isFa ? 'اجرا' : 'Run',
      items: [
        { label: isFa ? 'اجرای کد (F5)' : 'Run File (F5)', icon: <Play size={13} />, action: handleRun, disabled: !activePath }
      ]
    },
    {
      id: 'help',
      label: isFa ? 'راهنما' : 'Help',
      items: [
        { label: isFa ? 'تنظیمات محیط توسعه' : 'Preferences', icon: <Settings size={13} />, action: () => setShowSettingsModal(true) },
        { label: isFa ? 'درباره آذراوایب' : 'About Azravibe IDE', icon: <Info size={13} />, action: () => setAboutPanelOpen(true) }
      ]
    }
  ]

  const getCenteredTitle = () => {
    const folderName = currentFolder ? currentFolder.split('\\').pop() : null
    const fileName = activePath ? activePath.split('\\').pop() : null
    if (fileName && folderName) {
      return `${fileName} — ${folderName} — azravibe_ide`
    } else if (folderName) {
      return `${folderName} [${isFa ? 'پوشه جاری' : 'Workspace'}] — azravibe_ide`
    }
    return isFa ? 'محیط توسعه فکری آذراوایب' : 'azravibe_ide'
  }

  const handleMenuClick = (menuId: string) => {
    if (menuOpen && activeMenu === menuId) {
      setActiveMenu(null)
      setMenuOpen(false)
    } else {
      setActiveMenu(menuId)
      setMenuOpen(true)
    }
  }

  const handleMenuMouseEnter = (menuId: string) => {
    if (menuOpen) {
      setActiveMenu(menuId)
    }
  }

  return (
    <div className="relative h-10 bg-sidebar border-b border-editor flex items-center justify-between px-3 select-none drag-region">
      
      {/* Menubar (Left on LTR, Right on RTL) */}
      <div className={`flex items-center gap-1 no-drag ${isFa ? 'flex-row-reverse' : 'flex-row'}`} ref={menuRef}>
        
        {/* Subtle branding icon/text */}
        <div className={`flex items-center gap-2 text-accent font-semibold tracking-wide text-xs px-2 ${isFa ? 'ml-3' : 'mr-3'}`}>
          <BrandMark size="sm" />
          AZRAVIBE
        </div>

        {menus.map((menu) => (
          <div key={menu.id} className="relative">
            <button
              onClick={() => handleMenuClick(menu.id)}
              onMouseEnter={() => handleMenuMouseEnter(menu.id)}
              className={`px-3 py-1 text-xs hover:bg-editor hover:text-txt transition-colors duration-150 ${
                activeMenu === menu.id ? 'bg-editor text-accent font-semibold' : 'text-neutral-300'
              }`}
            >
              {menu.label}
            </button>

            {activeMenu === menu.id && (
              <div className={`absolute mt-1 w-52 bg-sidebar border border-editor z-[9999] py-1 ${isFa ? 'right-0 text-right' : 'left-0 text-left'}`}>
                {menu.items.map((item, idx) => (
                  item.divider ? (
                    <div key={idx} className="my-1 border-t border-neutral-800" />
                  ) : (
                    <button
                      key={idx}
                      onClick={() => {
                        if (!item.disabled && item.action) {
                          item.action()
                          setActiveMenu(null)
                          setMenuOpen(false)
                        }
                      }}
                      disabled={item.disabled}
                      className={`w-full flex items-center justify-between px-3 py-2 text-xs transition-colors duration-100 ${
                        isFa ? 'flex-row-reverse text-right' : 'flex-row text-left'
                      } ${
                        item.disabled
                          ? 'opacity-30 cursor-not-allowed text-neutral-500'
                          : 'text-neutral-200 hover:bg-editor hover:text-accent'
                      }`}
                    >
                      <span className="flex-1 font-medium">{item.label}</span>
                      {item.icon && (
                        <span className={`${isFa ? 'mr-3 text-neutral-400' : 'ml-3 text-neutral-400'}`}>
                          {item.icon}
                        </span>
                      )}
                    </button>
                  )
                ))}
              </div>
            )}
          </div>
        ))}
      </div>

      {/* Beautiful centered title mimicking professional IDEs */}
      <div className="absolute left-1/2 top-1/2 transform -translate-x-1/2 -translate-y-1/2 text-[11px] font-semibold text-neutral-400 max-w-[30vw] truncate pointer-events-none text-center">
        {getCenteredTitle()}
      </div>

      {/* Layout Controllers & Windows Controls (Right on LTR, Left on RTL) */}
      <div className={`flex items-center gap-1 no-drag ${isFa ? 'flex-row-reverse' : 'flex-row'}`}>
        
        {/* IDE Layout Control Buttons */}
        <div className={`flex items-center gap-1 border-r border-neutral-800/80 px-1.5 ${isFa ? 'border-l border-r-0' : 'border-r border-l-0'}`}>
          
          {/* Toggle Sidebar */}
          <button
            onClick={() => setSidebarOpen(!sidebarOpen)}
            title={isFa ? 'تغییر وضعیت نوار کناری' : 'Toggle Explorer Sidebar'}
            className={`p-1.5 rounded-md transition-colors ${
              sidebarOpen ? 'text-accent hover:bg-editor' : 'text-neutral-500 hover:bg-editor hover:text-txt'
            }`}
          >
            <PanelLeft size={13.5} />
          </button>

        </div>

        {/* Windows Standard Controls */}
        <div className="flex h-10 items-center">
          <button
            onClick={() => window.api.minimize()}
            className="flex h-10 w-11 items-center justify-center text-neutral-400 hover:bg-editor hover:text-txt transition-colors"
            title={isFa ? 'کوچک کردن' : 'Minimize'}
          >
            <Minimize2 size={13} />
          </button>
          <button
            onClick={() => window.api.maximize()}
            className="flex h-10 w-11 items-center justify-center text-neutral-400 hover:bg-editor hover:text-txt transition-colors"
            title={isFa ? 'بزرگ کردن' : 'Maximize'}
          >
            <Square size={11} />
          </button>
          <button
            onClick={() => window.api.close()}
            className="flex h-10 w-11 items-center justify-center text-neutral-400 hover:bg-[#c42b1c] hover:text-white transition-colors"
            title={isFa ? 'بستن' : 'Close'}
          >
            <X size={13} />
          </button>
        </div>

      </div>

    </div>
  )
}
