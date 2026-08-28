import { EditorView } from '@codemirror/view'
import * as azravibeSyntax from '../../assets/azravibe.json'

export function buildTheme(
  isRtl: boolean,
  theme: 'azravibe' | 'vscode' | 'light',
  fontSize: number,
  fontFamily: string
) {
  const isLight = theme === 'light'
  const background = isLight ? '#ffffff' : theme === 'vscode' ? '#1E1E1E' : '#1A1A1A'
  const foreground = isLight ? '#1f2937' : '#CCCCCC'
  const gutterBg = isLight ? '#F3F3F3' : theme === 'vscode' ? '#252526' : '#111111'
  const gutterFg = isLight ? '#64748b' : '#8b949e'
  const activeLine = isLight ? '#EEF2F7' : '#262a2b'
  const selection = isLight ? 'rgba(0, 122, 204, 0.22)' : 'rgba(0, 245, 255, 0.22)'
  const accent = theme === 'azravibe' ? '#00F5FF' : '#007ACC'
  const dotColor = isLight ? '#999999' : '#505050'

  return EditorView.theme({
    '&': {
      height: '100%',
      width: '100%',
      backgroundColor: background,
      color: foreground,
      fontSize: `${fontSize}px`
    },
    '.cm-scroller': {
      fontFamily,
      lineHeight: `${Math.round(fontSize * 1.5)}px`,
      flexDirection: 'row !important',
      direction: 'ltr'
    },
    '.cm-content': {
      order: isRtl ? '1 !important' : '2 !important',
      minHeight: '100%',
      caretColor: accent,
      padding: '0 12px',
      textAlign: isRtl ? 'right' : 'left',
      direction: isRtl ? 'rtl' : 'ltr'
    },
    '.cm-line': {
      direction: isRtl ? 'rtl' : 'ltr',
      textAlign: isRtl ? 'right' : 'left',
      position: 'relative'
    },
    '.cm-gutters': {
      order: isRtl ? '2 !important' : '1 !important',
      backgroundColor: gutterBg,
      color: gutterFg,
      borderRight: isRtl ? '0' : `1px solid ${isLight ? '#d1d5db' : '#2a2a2a'}`,
      borderLeft: isRtl ? `1px solid ${isLight ? '#d1d5db' : '#2a2a2a'}` : '0',
      minWidth: '44px'
    },
    '.cm-lineNumbers .cm-gutterElement': {
      minWidth: '36px',
      padding: '0 9px',
      textAlign: isRtl ? 'left' : 'right'
    },
    '.cm-foldGutter .cm-gutterElement, .cm-lintGutter .cm-gutterElement': {
      padding: '0 4px'
    },
    '.cm-activeLine': {
      backgroundColor: activeLine
    },
    '.cm-activeLineGutter': {
      backgroundColor: isLight ? '#E5E7EB' : '#202425',
      color: accent
    },
    '.cm-selectionBackground, &.cm-focused .cm-selectionBackground': {
      backgroundColor: `${selection} !important`
    },
    '.cm-cursor': {
      borderLeftColor: accent,
      borderLeftWidth: '2px'
    },
    '&.cm-focused': {
      outline: 'none'
    },
    '.cm-searchMatch': {
      backgroundColor: isLight ? '#fde68a' : '#574b1f'
    },
    '.cm-tooltip, .cm-panel': {
      backgroundColor: isLight ? '#ffffff' : '#202020',
      border: `1px solid ${isLight ? '#d1d5db' : '#333'}`,
      color: foreground
    },
    '.cm-tooltip-autocomplete ul li[aria-selected]': {
      backgroundColor: isLight ? '#e0f2fe' : '#06343a',
      color: foreground
    },
    '.cm-line.cm-indent-visual-line::before': {
      content: '""',
      position: 'absolute',
      top: '0',
      bottom: '0',
      [isRtl ? 'right' : 'left']: '0',
      width: 'calc(var(--azr-indent-columns) * var(--azr-indent-char-width))',
      pointerEvents: 'none',
      backgroundImage: `radial-gradient(circle, ${dotColor} 0 1.35px, transparent 1.55px)`,
      backgroundSize: 'var(--azr-indent-char-width) 100%',
      backgroundPosition: `${isRtl ? 'right' : 'left'} center`,
      backgroundRepeat: 'repeat-x',
      opacity: isLight ? '0.72' : '0.66',
      zIndex: '0'
    },
    '.cm-line.cm-indent-visual-line > *': {
      position: 'relative',
      zIndex: '1'
    },
    '.cm-token-keyword': {
      color: azravibeSyntax.colors.keyword,
      fontWeight: '700'
    },
    '.cm-token-builtin': {
      color: azravibeSyntax.colors.builtin
    },
    '.cm-token-string': {
      color: azravibeSyntax.colors.string
    },
    '.cm-token-number': {
      color: azravibeSyntax.colors.number
    },
    '.cm-token-comment': {
      color: azravibeSyntax.colors.comment,
      fontStyle: 'italic'
    },
    '.cm-token-operator': {
      color: azravibeSyntax.colors.operator
    },
    '.cm-token-function': {
      color: azravibeSyntax.colors.function,
      fontWeight: '600'
    },
    '.cm-token-variable': {
      color: azravibeSyntax.colors.variable
    },
    '.cm-token-type': {
      color: azravibeSyntax.colors.type
    },
    '.cm-token-property': {
      color: azravibeSyntax.colors.property
    },
    '.cm-token-parenthesis': {
      color: azravibeSyntax.colors.parenthesis
    }
  })
}
