import * as React from 'react'
import { useEffect, useMemo, useRef, useState } from 'react'
import { Extension, EditorState } from '@codemirror/state'
import {
  EditorView,
  ViewUpdate,
  highlightActiveLine,
  highlightActiveLineGutter,
  keymap,
  lineNumbers
} from '@codemirror/view'
import {
  defaultKeymap,
  copyLineDown,
  copyLineUp,
  cursorMatchingBracket,
  deleteCharBackward,
  deleteLine,
  history,
  historyKeymap,
  insertBlankLine,
  indentWithTab,
  moveLineDown,
  moveLineUp,
  redo,
  selectLine,
  undo
} from '@codemirror/commands'
import { highlightSelectionMatches, openSearchPanel, replaceAll, searchKeymap, selectNextOccurrence } from '@codemirror/search'
import {
  bracketMatching,
  foldGutter,
  foldKeymap,
  indentOnInput,
  syntaxHighlighting,
  indentUnit
} from '@codemirror/language'
import { autocompletion, closeBrackets, closeBracketsKeymap, completionKeymap } from '@codemirror/autocomplete'
import { lintGutter } from '@codemirror/lint'
import { javascript } from '@codemirror/lang-javascript'
import { useStore } from '../store/useStore'
import ProblemsPanel from './ProblemsPanel'
import SettingsPanel from './SettingsPanel'
import SearchPanel from './SearchPanel'
import OutlinePanel from './OutlinePanel'
import AboutPanel from './AboutPanel'
import AzravibeMinimap from './code-editor/AzravibeMinimap'
import {
  azravibeCompletions,
  azravibeHighlightStyle,
  azravibeLanguage,
  azravibeSemanticHighlight,
  collectBasicAzravibeDiagnostics,
  getCursorLineCol,
  indentGuides,
  insertLineAbove,
  makeAzravibeLinter,
  toggleSingleLineComment
} from './code-editor/azravibe-language'
import { buildTheme } from './code-editor/theme'

export default function CodeEditor() {
  const {
    activePath,
    tabs,
    updateFileContent,
    saveFile,
    addTerminal,
    setCursorPos,
    setDiagnostics,
    wordWrap,
    fontSize,
    fontFamily,
    compilerPath,
    editorRtl,
    theme,
    tabSize,
    insertSpaces,
    minimap
  } = useStore()

  const hostRef = useRef<HTMLDivElement>(null)
  const viewRef = useRef<EditorView | null>(null)
  const lastPathRef = useRef<string | null>(null)
  const [editorView, setEditorView] = useState<EditorView | null>(null)
  const activeTab = tabs.find((tab) => tab.path === activePath)
  const isRtl = editorRtl

  const extensions = useMemo<Extension[]>(() => {
    if (!activeTab) return []
    const isAzravibe = activeTab.path.toLowerCase().endsWith('.azr')

    return [
      lineNumbers(),
      foldGutter(),
      lintGutter(),
      highlightActiveLineGutter(),
      history(),
      closeBrackets(),
      bracketMatching(),
      indentOnInput(),
      highlightActiveLine(),
      highlightSelectionMatches(),
      autocompletion({ override: isAzravibe ? [azravibeCompletions] : undefined }),
      keymap.of([
        {
          key: 'Mod-s',
          run: () => {
            if (activePath) void saveFile(activePath)
            return true
          }
        },
        {
          key: 'F5',
          run: () => {
            if (activePath) {
              const termId = useStore.getState().activeTerminalId || addTerminal()
              void saveFile(activePath).then(() => {
                window.api.writeTerminal(termId, `${compilerPath} "${activePath}"\r`)
              })
            }
            return true
          }
        },
        {
          key: 'Mod-/',
          run: toggleSingleLineComment,
          preventDefault: true
        },
        {
          key: 'Backspace',
          run: deleteCharBackward
        },
        indentWithTab,
        ...defaultKeymap,
        ...historyKeymap,
        ...foldKeymap,
        ...closeBracketsKeymap,
        ...completionKeymap,
        ...searchKeymap
      ]),
      EditorView.updateListener.of((update: ViewUpdate) => {
        if (update.docChanged) {
          const next = update.state.doc.toString()
          updateFileContent(activeTab.path, next)
          setDiagnostics(isAzravibe
            ? collectBasicAzravibeDiagnostics(next).map((item) => ({ path: activeTab.path, ...item }))
            : []
          )
        }

        if (update.docChanged || update.selectionSet) {
          const cursor = getCursorLineCol(update.state)
          setCursorPos(cursor.line, cursor.col)
        }
      }),
      EditorView.editorAttributes.of({
        dir: isRtl ? 'rtl' : 'ltr',
        spellcheck: 'false'
      }),
      EditorView.perLineTextDirection.of(true),
      EditorView.contentAttributes.of({
        dir: isRtl ? 'rtl' : 'ltr',
        style: `text-align: ${isRtl ? 'right' : 'left'};`
      }),
      wordWrap ? EditorView.lineWrapping : [],
      indentGuides(theme === 'light', isRtl, fontSize),
      isAzravibe ? makeAzravibeLinter(activeTab.path) : [],
      isAzravibe ? [azravibeLanguage, syntaxHighlighting(azravibeHighlightStyle), azravibeSemanticHighlight()] : javascript(),
      EditorState.tabSize.of(tabSize),
      indentUnit.of(insertSpaces ? " ".repeat(tabSize) : "\t"),
      buildTheme(isRtl, theme, fontSize, fontFamily)
    ]
  }, [activeTab?.path, activePath, addTerminal, compilerPath, editorRtl, fontFamily, fontSize, saveFile, setCursorPos, setDiagnostics, theme, updateFileContent, wordWrap, tabSize, insertSpaces])

  useEffect(() => {
    if (!hostRef.current || !activeTab) return

    const pathChanged = lastPathRef.current !== activeTab.path
    lastPathRef.current = activeTab.path

      viewRef.current?.destroy()

    const state = EditorState.create({
      doc: activeTab.content,
      extensions
    })
    const view = new EditorView({
      state,
      parent: hostRef.current
    })
    viewRef.current = view
    setEditorView(view)

    if (pathChanged) {
      view.focus()
    }

    const cursor = getCursorLineCol(view.state)
    setCursorPos(cursor.line, cursor.col)
    setDiagnostics(activeTab.path.toLowerCase().endsWith('.azr')
      ? collectBasicAzravibeDiagnostics(activeTab.content).map((item) => ({ path: activeTab.path, ...item }))
      : []
    )

    return () => {
      view.destroy()
      if (viewRef.current === view) viewRef.current = null
      setEditorView((current) => current === view ? null : current)
    }
  }, [activeTab?.path, extensions])

  useEffect(() => {
    const handleCommand = (event: Event) => {
      const view = viewRef.current
      if (!view) return
      const detail = (event as CustomEvent<'undo' | 'redo' | 'cut' | 'copy' | 'paste' | 'find' | 'replace' | 'replaceAll' | 'addSelectionToNextFindMatch' | 'selectLine' | 'deleteLine' | 'insertLineBelow' | 'insertLineAbove' | 'jumpToMatchingBracket' | 'moveLineUp' | 'moveLineDown' | 'copyLineUp' | 'copyLineDown' | 'toggleComment'>).detail
      view.focus()

      if (detail === 'undo') undo(view)
      if (detail === 'redo') redo(view)
      if (detail === 'cut') document.execCommand('cut')
      if (detail === 'copy') document.execCommand('copy')
      if (detail === 'paste') document.execCommand('paste')
      if (detail === 'find') openSearchPanel(view)
      if (detail === 'replace') {
        openSearchPanel(view)
        const panel = view.dom.querySelector('.cm-search') as HTMLElement | null
        panel?.querySelector<HTMLInputElement>('[name="replace"]')?.focus()
      }
      if (detail === 'replaceAll') replaceAll(view)
      if (detail === 'addSelectionToNextFindMatch') selectNextOccurrence(view)
      if (detail === 'selectLine') selectLine(view)
      if (detail === 'deleteLine') deleteLine(view)
      if (detail === 'insertLineBelow') insertBlankLine(view)
        if (detail === 'insertLineAbove') {
          insertLineAbove(view)
        }
      if (detail === 'jumpToMatchingBracket') cursorMatchingBracket(view)
      if (detail === 'moveLineUp') moveLineUp(view)
      if (detail === 'moveLineDown') moveLineDown(view)
      if (detail === 'copyLineUp') copyLineUp(view)
      if (detail === 'copyLineDown') copyLineDown(view)
      if (detail === 'toggleComment') toggleSingleLineComment(view)
    }

    window.addEventListener('azravibe-editor-command', handleCommand)
    return () => window.removeEventListener('azravibe-editor-command', handleCommand)
  }, [])

  if (!activeTab) return null

  // Check if this is a panel tab
  if (activeTab.isPanel) {
    switch (activeTab.panelType) {
      case 'problems':
        return <ProblemsPanel />
      case 'settings':
        return <SettingsPanel />
      case 'search':
        return <SearchPanel />
      case 'outline':
        return <OutlinePanel />
      case 'about':
        return <AboutPanel />
      default:
        return null
    }
  }

  return (
    <div className="flex h-full w-full overflow-hidden bg-editor text-txt">
      <div
        ref={hostRef}
        className="min-w-0 flex-1 overflow-hidden"
        dir={isRtl ? 'rtl' : 'ltr'}
      />
      {minimap && activeTab.path.toLowerCase().endsWith('.azr') && (
        <AzravibeMinimap
          content={activeTab.content}
          view={editorView}
          theme={theme}
          isRtl={isRtl}
        />
      )}
    </div>
  )
}
