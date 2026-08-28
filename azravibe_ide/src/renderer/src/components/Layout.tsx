import React from 'react'
import { PanelGroup, Panel, PanelResizeHandle } from 'react-resizable-panels'
import Sidebar from './Sidebar'
import Tabbar from './Tabbar'
import CodeEditor from './CodeEditor'
import BottomPanelContainer from './BottomPanelContainer'
import WelcomeScreen from './WelcomeScreen'
import { useStore } from '../store/useStore'

export default function Layout() {
  const { activePath, sidebarOpen, problemsPanelOpen, problemsPanelPosition, problemsPanelSize } = useStore()

  // Helper to render editor area (tabs + editor)
  const renderEditorArea = () => (
    <div className="flex flex-col h-full bg-editor">
      <Tabbar />
      <div className="flex-1 overflow-hidden">
        {activePath ? <CodeEditor /> : <WelcomeScreen />}
      </div>
    </div>
  )

  // Render based on problems panel position
  if (problemsPanelOpen && problemsPanelPosition === 'bottom') {
    return (
      <div className="flex-1 flex overflow-hidden">
        {sidebarOpen ? (
          <PanelGroup direction="horizontal" autoSaveId="azravibe-explorer-horizontal-v2" dir="ltr">
            <Panel defaultSize={20} minSize={15} maxSize={40} order={1} className="h-full">
              <Sidebar />
            </Panel>
            <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />
            <Panel order={2} className="h-full flex flex-col">
              <PanelGroup direction="vertical" autoSaveId="persistence-vertical">
                <Panel defaultSize={70} minSize={30} order={1} className="flex flex-col h-full bg-editor">
                  {renderEditorArea()}
                </Panel>
                <PanelResizeHandle className="h-1.5 hover:bg-accent/40 bg-editor border-t border-b border-neutral-900 cursor-row-resize transition-colors duration-150" />
                <Panel defaultSize={problemsPanelSize} minSize={15} order={2} className="h-full bg-bg">
                  <BottomPanelContainer />
                </Panel>
              </PanelGroup>
            </Panel>
          </PanelGroup>
        ) : (
          <div className="flex-1 h-full flex flex-col">
            <PanelGroup direction="vertical" autoSaveId="persistence-vertical">
              <Panel defaultSize={70} minSize={30} order={1} className="flex flex-col h-full bg-editor">
                {renderEditorArea()}
              </Panel>
              <PanelResizeHandle className="h-1.5 hover:bg-accent/40 bg-editor border-t border-b border-neutral-900 cursor-row-resize transition-colors duration-150" />
              <Panel defaultSize={problemsPanelSize} minSize={15} order={2} className="h-full bg-bg">
                <BottomPanelContainer />
              </Panel>
            </PanelGroup>
          </div>
        )}
      </div>
    )
  }

  // Problems panel on the right side
  if (problemsPanelOpen && problemsPanelPosition === 'right') {
    return (
      <div className="flex-1 flex overflow-hidden">
        <PanelGroup direction="horizontal" autoSaveId="azravibe-explorer-horizontal-v2" dir="ltr">
          {/* Sidebar */}
          {sidebarOpen && (
            <>
              <Panel defaultSize={20} minSize={15} maxSize={40} order={1} className="h-full">
                <Sidebar />
              </Panel>
              <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />
            </>
          )}
          
          {/* Editor + Terminal area */}
          <Panel order={sidebarOpen ? 2 : 1} className="h-full flex flex-col">
            {renderEditorArea()}
          </Panel>
          
          <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />
          
          {/* Problems Panel on right */}
          <Panel defaultSize={problemsPanelSize} minSize={15} maxSize={50} order={sidebarOpen ? 3 : 2} className="h-full bg-bg">
            <BottomPanelContainer />
          </Panel>
        </PanelGroup>
      </div>
    )
  }

  // Problems panel on the left side
  if (problemsPanelOpen && problemsPanelPosition === 'left') {
    return (
      <div className="flex-1 flex overflow-hidden">
        <PanelGroup direction="horizontal" autoSaveId="azravibe-explorer-horizontal-v2" dir="ltr">
          {/* Problems Panel on left */}
          <Panel defaultSize={problemsPanelSize} minSize={15} maxSize={50} order={1} className="h-full bg-bg">
            <BottomPanelContainer />
          </Panel>
          
          <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />
          
          {/* Editor + Terminal area */}
          <Panel order={2} className="h-full flex flex-col">
            {renderEditorArea()}
          </Panel>
          
          {/* Sidebar */}
          {sidebarOpen && (
            <>
              <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />
              <Panel defaultSize={20} minSize={15} maxSize={40} order={3} className="h-full">
                <Sidebar />
              </Panel>
            </>
          )}
        </PanelGroup>
      </div>
    )
  }

  // Original layout without problems panel
  return (
    <div className="flex-1 flex overflow-hidden">
      {sidebarOpen ? (
        <PanelGroup 
          direction="horizontal" 
          autoSaveId="azravibe-explorer-horizontal-v2"
          dir="ltr"
        >
          {/* Right side in RTL: Sidebar Explorer */}
          <Panel 
            defaultSize={20} 
            minSize={15} 
            maxSize={40} 
            order={1}
            className="h-full"
          >
            <Sidebar />
          </Panel>

          {/* Resizing Handle between Sidebar and Editor Workspace */}
          <PanelResizeHandle className="w-1.5 hover:bg-accent/40 bg-editor border-l border-r border-neutral-900 cursor-col-resize transition-colors duration-150" />

          {/* Left side in RTL: Editor */}
          <Panel order={2} className="h-full flex flex-col">
            {renderEditorArea()}
          </Panel>
        </PanelGroup>
      ) : (
        /* Sidebar is completely hidden! Just render editor directly */
        <div className="flex-1 h-full">
          {renderEditorArea()}
        </div>
      )}
    </div>
  )
}
