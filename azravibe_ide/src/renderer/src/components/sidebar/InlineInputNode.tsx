import { useEffect, useRef, useState } from 'react'
import { AlertTriangle, File, Folder } from 'lucide-react'
import { validatePathName } from './shared'

interface InlineInputProps {
  isDirectory: boolean
  depth?: number
  onSubmit: (name: string) => void
  onCancel: () => void
  isFa: boolean
}

export default function InlineInputNode({
  isDirectory,
  depth = 0,
  onSubmit,
  onCancel,
  isFa
}: InlineInputProps) {
  const [val, setVal] = useState('')
  const [errorMsg, setErrorMsg] = useState<string | null>(null)
  const ref = useRef<HTMLInputElement>(null)

  useEffect(() => {
    ref.current?.focus()
  }, [])

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const inputVal = e.target.value
    setVal(inputVal)
    setErrorMsg(validatePathName(inputVal, isFa))
  }

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      const nextError = validatePathName(val, isFa)
      setErrorMsg(nextError)
      if (val.trim() && !nextError) {
        onSubmit(val.trim())
      } else {
        ref.current?.focus()
      }
    } else if (e.key === 'Escape') {
      onCancel()
    }
  }

  return (
    <div
      className="explorer-inline-create flex flex-col w-full"
      style={isFa ? { paddingRight: `${depth * 12 + 22}px` } : { paddingLeft: `${depth * 12 + 22}px` }}
      onClick={(e) => e.stopPropagation()}
    >
      <div className={`flex h-[22px] items-center ${isFa ? 'flex-row-reverse text-right' : 'flex-row text-left'}`}>
        <span className={isFa ? 'ml-1.5 text-[#dcb67a]' : 'mr-1.5 text-[#dcb67a]'}>
          {isDirectory ? <Folder size={14} className="text-[#dcb67a]" /> : <File size={14} />}
        </span>
        <input
          ref={ref}
          type="text"
          value={val}
          onChange={handleChange}
          onBlur={() => {
            if (!val.trim() && !errorMsg) {
              onCancel()
            }
          }}
          onKeyDown={handleKeyDown}
          className={`h-[20px] min-w-0 flex-1 border bg-[#172033] px-1.5 text-[12px] text-[#d4d4d4] outline-none ${
            errorMsg ? 'border-red-500/80 focus:border-red-500' : 'border-[#3b6ea8] focus:border-[#5b9bd5]'
          } ${isFa ? 'text-right' : 'text-left'}`}
          placeholder={isDirectory ? (isFa ? 'نام پوشه...' : 'Folder Name...') : (isFa ? 'نام فایل...' : 'File Name...')}
        />
      </div>

      {errorMsg && (
        <div
          className={`mt-2 p-2 bg-[#2d1212] border border-red-900/60 rounded text-[10px] text-red-300 leading-normal flex items-start justify-start ${
            isFa ? 'flex-row-reverse text-right mr-1.5 ml-0' : 'flex-row text-left ml-1.5 mr-0'
          } max-w-[240px]`}
        >
          <AlertTriangle size={12} className={`text-red-400 shrink-0 mt-0.5 ${isFa ? 'ml-1.5' : 'mr-1.5'}`} />
          <span className="flex-1 font-medium">{errorMsg}</span>
        </div>
      )}
    </div>
  )
}
