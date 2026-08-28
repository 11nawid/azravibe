import { EditorSelection, EditorState } from '@codemirror/state'
import { CompletionContext } from '@codemirror/autocomplete'
import { Diagnostic, linter } from '@codemirror/lint'
import {
  HighlightStyle,
  StreamLanguage
} from '@codemirror/language'
import {
  Decoration,
  DecorationSet,
  EditorView,
  ViewPlugin,
  ViewUpdate
} from '@codemirror/view'
import { tags } from '@lezer/highlight'
import * as azravibeSyntax from '../../assets/azravibe.json'

export type MiniTokenKind =
  | 'keyword'
  | 'builtin'
  | 'string'
  | 'number'
  | 'comment'
  | 'operator'
  | 'function'
  | 'type'
  | 'property'
  | 'variable'
  | 'parenthesis'
  | 'plain'

type AzravibeToken = { from: number; to: number; text: string; kind: MiniTokenKind }

const identifierPattern = /[\u0600-\u06FF\u200Ca-zA-Z_$][\u0600-\u06FF\u200Ca-zA-Z0-9_$]*/y
const keywordSet = new Set(azravibeSyntax.keywords as string[])
const builtinSet = new Set(azravibeSyntax.builtins as string[])
const typeKeywords = new Set(['نوع', 'عدد_صحیح', 'عدد_اعشاری', 'متن', 'بولی', 'بایت'])
const functionDeclarators = new Set(['کار', 'تابع', 'function'])
const callableKeywords = new Set(['بنویس', 'بگیر', 'بیار'])
const controlKeywords = new Set(['اگه', 'وگرنه', 'تا', 'وقتی', 'برای', 'هر', 'توی', 'سعی', 'کن', 'خطا', 'بالاخره'])
const multiWordKeywords = (azravibeSyntax.keywords as string[])
  .filter((keyword) => keyword.includes(' '))
  .sort((a, b) => b.length - a.length)

export const minimapColors: Record<MiniTokenKind, string> = {
  keyword: azravibeSyntax.colors.keyword,
  builtin: azravibeSyntax.colors.builtin,
  string: azravibeSyntax.colors.string,
  number: azravibeSyntax.colors.number,
  comment: azravibeSyntax.colors.comment,
  operator: azravibeSyntax.colors.operator,
  function: azravibeSyntax.colors.function,
  type: azravibeSyntax.colors.type,
  property: azravibeSyntax.colors.property,
  parenthesis: azravibeSyntax.colors.parenthesis,
  variable: azravibeSyntax.colors.variable,
  plain: '#6f7d8f'
}

export const azravibeHighlightStyle = HighlightStyle.define([
  { tag: tags.keyword, class: 'cm-token-keyword' },
  { tag: tags.variableName, class: 'cm-token-variable' },
  { tag: tags.definition(tags.variableName), class: 'cm-token-function' },
  { tag: tags.standard(tags.variableName), class: 'cm-token-builtin' },
  { tag: tags.number, class: 'cm-token-number' },
  { tag: tags.string, class: 'cm-token-string' },
  { tag: tags.comment, class: 'cm-token-comment' },
  { tag: tags.operator, class: 'cm-token-operator' },
  { tag: tags.function(tags.variableName), class: 'cm-token-function' },
  { tag: tags.atom, class: 'cm-token-builtin' },
  { tag: tags.typeName, class: 'cm-token-type' },
  { tag: tags.propertyName, class: 'cm-token-property' },
  { tag: tags.paren, class: 'cm-token-parenthesis' },
  { tag: tags.bracket, class: 'cm-token-parenthesis' },
  { tag: tags.squareBracket, class: 'cm-token-parenthesis' },
  { tag: tags.brace, class: 'cm-token-parenthesis' }
])

export const azravibeLanguage = StreamLanguage.define({
  token(stream) {
    if (stream.eatSpace()) return null

    for (const keyword of multiWordKeywords) {
      if (stream.match(keyword)) return 'keyword'
    }

    if (stream.match(/#.*/)) return 'comment'
    if (stream.match(/"([^"\\]|\\.)*"/)) return 'string'
    if (stream.match(/"([^"\\]|\\.)*$/)) return 'invalid'
    if (stream.match(/\d+(?:\.\d+)?/)) return 'number'
    if (stream.match(/[+\-*/%=!<>:،]+/)) return 'operator'

    const word = stream.match(/[\u0600-\u06FF\u200Ca-zA-Z_$][\u0600-\u06FF\u200Ca-zA-Z0-9_$]*/)
    if (word && typeof word === 'string') {
      const value = word
      const rest = stream.string.slice(stream.pos)
      if (typeKeywords.has(value)) return 'type'
      if ((callableKeywords.has(value) || builtinSet.has(value)) && /^\s*\(/.test(rest)) return 'def'
      if (keywordSet.has(value)) return 'keyword'
      if (builtinSet.has(value)) return 'builtin'
      if (/^\s*\(/.test(rest)) return 'def'
      if (/^\s*:/.test(rest)) return 'property'
      return 'variableName'
    }

    stream.next()
    return null
  }
})

function classifyAzravibeToken(line: string, start: number): { text: string; kind: MiniTokenKind; next: number } {
  const rest = line.slice(start)
  if (rest.startsWith('#')) return { text: rest, kind: 'comment', next: line.length }
  if (/^\s+$/.test(rest)) return { text: rest, kind: 'plain', next: line.length }

  const multi = multiWordKeywords.find((keyword) => rest.startsWith(keyword))
  if (multi) return { text: multi, kind: 'keyword', next: start + multi.length }

  const stringMatch = /^"([^"\\]|\\.)*"?/.exec(rest)
  if (stringMatch && rest.startsWith('"')) {
    return { text: stringMatch[0], kind: 'string', next: start + stringMatch[0].length }
  }

  const numberMatch = /^\d+(?:\.\d+)?/.exec(rest)
  if (numberMatch) return { text: numberMatch[0], kind: 'number', next: start + numberMatch[0].length }

  const parenthesisMatch = /^[()[\]{}]+/.exec(rest)
  if (parenthesisMatch) return { text: parenthesisMatch[0], kind: 'parenthesis', next: start + parenthesisMatch[0].length }

  const operatorMatch = /^[+\-*/%=!<>:،]+/.exec(rest)
  if (operatorMatch) return { text: operatorMatch[0], kind: 'operator', next: start + operatorMatch[0].length }

  identifierPattern.lastIndex = start
  const word = identifierPattern.exec(line)
  if (word && word.index === start) {
    const text = word[0]
    const after = line.slice(identifierPattern.lastIndex)
    const before = line.slice(0, start).trimEnd().split(/\s+/).pop() || ''
    let kind: MiniTokenKind = 'variable'
    if (typeKeywords.has(text)) kind = 'type'
    else if ((callableKeywords.has(text) || builtinSet.has(text)) && /^\s*\(/.test(after)) kind = 'function'
    else if (keywordSet.has(text) || controlKeywords.has(text)) kind = 'keyword'
    else if (builtinSet.has(text)) kind = 'builtin'
    else if (functionDeclarators.has(before) || /^\s*\(/.test(after)) kind = 'function'
    else if (/^\s*:/.test(after)) kind = 'property'
    return { text, kind, next: identifierPattern.lastIndex }
  }

  return { text: rest[0], kind: 'plain', next: start + 1 }
}

function tokenizeAzravibeLine(line: string, base = 0): AzravibeToken[] {
  const tokens: AzravibeToken[] = []
  let index = 0

  while (index < line.length) {
    const token = classifyAzravibeToken(line, index)
    if (token.text.trim()) {
      tokens.push({
        from: base + index,
        to: base + token.next,
        text: token.text,
        kind: token.kind
      })
    }
    index = Math.max(token.next, index + 1)
  }

  return tokens
}

export function tokenizeMiniLine(line: string) {
  const tokens: Array<{ text: string; width: number; kind: MiniTokenKind }> = []
  for (const token of tokenizeAzravibeLine(line)) {
    if (tokens.length >= 120) break
    tokens.push({ text: token.text, width: Math.min(42, Math.max(2, token.text.length * 2)), kind: token.kind })
  }
  return tokens
}

export function azravibeSemanticHighlight() {
  return ViewPlugin.fromClass(class {
    decorations: DecorationSet

    constructor(view: EditorView) {
      this.decorations = this.buildDecorations(view)
    }

    update(update: ViewUpdate) {
      if (update.docChanged || update.viewportChanged) {
        this.decorations = this.buildDecorations(update.view)
      }
    }

    buildDecorations(view: EditorView): DecorationSet {
      const decorations: any[] = []

      for (const range of view.visibleRanges) {
        const startLine = view.state.doc.lineAt(range.from).number
        const endLine = view.state.doc.lineAt(range.to).number

        for (let lineNumber = startLine; lineNumber <= endLine; lineNumber += 1) {
          const line = view.state.doc.line(lineNumber)
          for (const token of tokenizeAzravibeLine(line.text, line.from)) {
            if (token.kind === 'plain') continue
            decorations.push(
              Decoration.mark({ class: `cm-token-${token.kind}` }).range(token.from, token.to)
            )
          }
        }
      }

      return Decoration.set(decorations, true)
    }
  }, {
    decorations: (plugin) => plugin.decorations
  })
}

export function getCursorLineCol(state: EditorState) {
  const head = state.selection.main.head
  const line = state.doc.lineAt(head)
  return {
    line: line.number,
    col: head - line.from + 1
  }
}

export function toggleSingleLineComment(view: EditorView) {
  const { state, dispatch } = view
  const changes = []

  for (const range of state.selection.ranges) {
    const fromLine = state.doc.lineAt(range.from)
    const toLine = state.doc.lineAt(range.to)

    for (let lineNum = fromLine.number; lineNum <= toLine.number; lineNum += 1) {
      const line = state.doc.line(lineNum)
      const lineText = line.text
      const hasComment = lineText.startsWith('#')

      if (hasComment) {
        changes.push({
          from: line.from,
          to: line.to,
          insert: lineText.replace(/^#\s?/, '')
        })
      } else {
        changes.push({
          from: line.from,
          to: line.to,
          insert: `#${lineText}`
        })
      }
    }
  }

  if (changes.length > 0) {
    dispatch({ changes })
    return true
  }

  return false
}

const completionWords = [
  ...(azravibeSyntax.keywords as string[]),
  ...(azravibeSyntax.builtins as string[])
].filter((value, index, array) => array.indexOf(value) === index)

export function azravibeCompletions(context: CompletionContext) {
  const word = context.matchBefore(/[\u0600-\u06FF\u200Ca-zA-Z_$][\u0600-\u06FF\u200Ca-zA-Z0-9_$]*/)
  if (!word || (word.from === word.to && !context.explicit)) return null

  return {
    from: word.from,
    options: completionWords.map((label) => ({
      label,
      type: (azravibeSyntax.keywords as string[]).includes(label) ? 'keyword' : 'function',
      apply: label
    }))
  }
}

export function collectBasicAzravibeDiagnostics(value: string): Array<{
  line: number
  col: number
  message: string
  severity: 'error' | 'warning'
}> {
  const diagnostics: Array<{ line: number; col: number; message: string; severity: 'error' | 'warning' }> = []
  const stack: string[] = []
  const matching: Record<string, string> = { ')': '(', ']': '[', '}': '{' }
  const opens = new Set(['(', '[', '{'])
  const closes = new Set([')', ']', '}'])

  value.split(/\r?\n/).forEach((rawLine, lineIndex) => {
    let inString = false
    for (let index = 0; index < rawLine.length; index += 1) {
      const char = rawLine[index]
      const escaped = index > 0 && rawLine[index - 1] === '\\'
      if (char === '"' && !escaped) {
        inString = !inString
        continue
      }
      if (inString) continue
      if (char === '#') break
      if (opens.has(char)) stack.push(`${char}:${lineIndex + 1}:${index + 1}`)
      if (closes.has(char)) {
        const open = stack.pop()
        if (!open || open.split(':')[0] !== matching[char]) {
          diagnostics.push({
            line: lineIndex + 1,
            col: index + 1,
            message: 'Mismatched closing bracket',
            severity: 'error'
          })
        }
      }
    }
    if (inString) {
      diagnostics.push({
        line: lineIndex + 1,
        col: rawLine.length,
        message: 'Unclosed string literal',
        severity: 'error'
      })
    }
  })

  for (const item of stack) {
    const [, line, col] = item.split(':')
    diagnostics.push({
      line: Number(line),
      col: Number(col),
      message: 'Unclosed bracket',
      severity: 'error'
    })
  }

  return diagnostics
}

export function indentGuides(isLight: boolean, isRtl: boolean, fontSize: number) {
  const guideColor = isLight ? 'rgba(148, 163, 184, 0.42)' : 'rgba(110, 124, 145, 0.32)'
  const activeGuideColor = isLight ? 'rgba(59, 130, 246, 0.62)' : 'rgba(0, 245, 255, 0.52)'

  return ViewPlugin.fromClass(class {
    decorations: DecorationSet

    constructor(view: EditorView) {
      this.decorations = this.buildDecorations(view)
    }

    update(update: ViewUpdate) {
      if (update.docChanged || update.selectionSet || update.viewportChanged || update.geometryChanged) {
        this.decorations = this.buildDecorations(update.view)
      }
    }

    buildDecorations(view: EditorView): DecorationSet {
      const decorations: any[] = []
      const tabSize = view.state.tabSize
      const { from, to } = view.viewport
      const startLine = view.state.doc.lineAt(from).number
      const endLine = view.state.doc.lineAt(to).number
      const cursorLine = view.state.doc.lineAt(view.state.selection.main.head).number
      const lineElement = view.dom.querySelector('.cm-line')
      if (!lineElement) return Decoration.none

      const testSpan = document.createElement('span')
      testSpan.style.visibility = 'hidden'
      testSpan.style.position = 'absolute'
      testSpan.style.whiteSpace = 'pre'
      testSpan.style.fontFamily = getComputedStyle(lineElement).fontFamily
      testSpan.style.fontSize = getComputedStyle(lineElement).fontSize
      testSpan.textContent = 'x'
      lineElement.appendChild(testSpan)
      const charWidth = testSpan.getBoundingClientRect().width
      lineElement.removeChild(testSpan)

      if (charWidth === 0) return Decoration.none

      for (let lineNum = startLine; lineNum <= endLine; lineNum += 1) {
        const line = view.state.doc.line(lineNum)
        const lineText = line.text
        if (lineText.trim().length === 0) continue

        let totalIndentColumns = 0
        for (let i = 0; i < lineText.length; i += 1) {
          if (lineText[i] === ' ' || lineText[i] === '\t') {
            totalIndentColumns += lineText[i] === '\t' ? tabSize : 1
          } else {
            break
          }
        }

        const numGuides = Math.floor(totalIndentColumns / tabSize)
        if (totalIndentColumns > 0) {
          const isActive = lineNum === cursorLine
          const layers: string[] = []
          const positions: string[] = []
          const guideWidths: string[] = []

          for (let level = 1; level <= numGuides; level += 1) {
            const color = isActive && level === numGuides ? activeGuideColor : guideColor
            const offset = level * tabSize * charWidth - charWidth / 2
            layers.push(`linear-gradient(to bottom, ${color}, ${color})`)
            positions.push(`${isRtl ? `calc(100% - ${offset}px)` : `${offset}px`} 0`)
            guideWidths.push('1px 100%')
          }

          decorations.push(
            Decoration.line({
              attributes: {
                class: 'cm-indent-visual-line',
                style: `
                  position: relative;
                  --azr-indent-columns: ${totalIndentColumns};
                  --azr-indent-char-width: ${charWidth}px;
                  background-image: ${layers.join(', ')};
                  background-position: ${positions.join(', ')};
                  background-size: ${guideWidths.join(', ')};
                  background-repeat: no-repeat;
                `
              }
            }).range(line.from, line.from)
          )
        }
      }

      return Decoration.set(decorations)
    }
  }, {
    decorations: (plugin) => plugin.decorations
  })
}

export function makeAzravibeLinter(path: string) {
  return linter((view) => {
    const basicDiagnostics = collectBasicAzravibeDiagnostics(view.state.doc.toString())
    return basicDiagnostics.map<Diagnostic>((item) => {
      const line = view.state.doc.line(item.line)
      const from = Math.min(line.to, line.from + Math.max(0, item.col - 1))
      return {
        from,
        to: Math.min(line.to, from + 1),
        severity: item.severity,
        message: item.message,
        source: path
      }
    })
  })
}

export function insertLineAbove(view: EditorView) {
  const head = view.state.selection.main.head
  const line = view.state.doc.lineAt(head)
  view.dispatch({
    changes: { from: line.from, insert: '\n' },
    selection: EditorSelection.cursor(line.from)
  })
}
