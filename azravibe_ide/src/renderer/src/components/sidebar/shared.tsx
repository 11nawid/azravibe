import { File } from 'lucide-react'

export const pathParent = (targetPath: string) => targetPath.substring(0, targetPath.lastIndexOf('\\'))

export const pathContains = (parent: string, child: string) => (
  child === parent || child.startsWith(`${parent}\\`)
)

export const validatePathName = (name: string, isFa: boolean): string | null => {
  if (!name.trim()) return null

  const invalidChars = /[\\/:*?"<>|]/
  if (invalidChars.test(name)) {
    return isFa
      ? 'نام وارد شده حاوی کاراکترهای غیرمجاز است. لطفاً از کاراکترهای معتبر استفاده کنید.'
      : `The name '${name}' is not valid as a file or folder name. Please choose a different name.`
  }

  const reservedNames = /^(con|prn|aux|nul|com[1-9]|lpt[1-9])(\..*)?$/i
  if (reservedNames.test(name.trim())) {
    return isFa
      ? 'این نام در ویندوز رزرو شده است. لطفاً نام دیگری انتخاب کنید.'
      : 'This name is reserved by Windows. Please choose a different name.'
  }

  if (/[.\s]$/.test(name)) {
    return isFa
      ? 'نام فایل یا پوشه نباید با نقطه یا فاصله تمام شود.'
      : 'File and folder names cannot end with a dot or space.'
  }

  return null
}

export const getFileIcon = (fileName: string) => {
  const ext = fileName.split('.').pop()?.toLowerCase()
  switch (ext) {
    case 'azr':
      return (
        <span className="w-4 h-4 rounded bg-gradient-to-tr from-cyan-600 to-teal-400 text-black flex items-center justify-center text-[9px] font-black shadow-sm shrink-0">
          آ
        </span>
      )
    case 'c':
      return (
        <span className="w-4 h-4 rounded bg-[#007acc] text-white flex items-center justify-center text-[9px] font-extrabold shadow-sm shrink-0">
          C
        </span>
      )
    case 'h':
      return (
        <span className="w-4 h-4 rounded bg-[#a07cff] text-white flex items-center justify-center text-[9px] font-extrabold shadow-sm shrink-0">
          H
        </span>
      )
    case 'js':
    case 'jsx':
      return (
        <span className="w-4 h-4 rounded bg-[#f7df1e] text-black flex items-center justify-center text-[9px] font-black shadow-sm shrink-0">
          JS
        </span>
      )
    case 'ts':
    case 'tsx':
      return (
        <span className="w-4 h-4 rounded bg-[#3178c6] text-white flex items-center justify-center text-[9px] font-extrabold shadow-sm shrink-0">
          TS
        </span>
      )
    case 'json':
      return (
        <span className="w-4 h-4 rounded bg-[#cb9200] text-white flex items-center justify-center text-[9px] font-bold shadow-sm shrink-0">
          {}
        </span>
      )
    case 'md':
      return (
        <span className="w-4 h-4 rounded bg-[#009688] text-white flex items-center justify-center text-[8px] font-black shadow-sm shrink-0">
          MD
        </span>
      )
    default:
      return <File size={14} className="text-neutral-400 shrink-0" />
  }
}
