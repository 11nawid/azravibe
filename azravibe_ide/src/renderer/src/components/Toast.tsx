import React, { useEffect } from 'react'
import { CheckCircle } from 'lucide-react'

interface ToastProps {
  message: string
  isVisible: boolean
  onClose: () => void
}

export default function Toast({ message, isVisible, onClose }: ToastProps) {
  useEffect(() => {
    if (isVisible) {
      const timer = setTimeout(() => {
        onClose()
      }, 2000) // Auto-hide after 2 seconds
      
      return () => clearTimeout(timer)
    }
  }, [isVisible, onClose])

  if (!isVisible) return null

  return (
    <div className="fixed bottom-16 right-4 z-[99999] animate-in fade-in slide-in-from-bottom-2 duration-300">
      <div className="bg-editor border border-editor px-4 py-3 flex items-center gap-3 min-w-[200px]">
        <CheckCircle size={18} className="text-accent shrink-0" />
        <span className="text-xs text-txt font-medium">{message}</span>
      </div>
    </div>
  )
}
