import React from 'react'
import { useStore } from '../store/useStore'

interface BrandMarkProps {
  size?: 'sm' | 'md' | 'lg' | 'xl'
  className?: string
}

const sizes = {
  sm: 'w-5 h-5',
  md: 'w-8 h-8',
  lg: 'w-12 h-12',
  xl: 'w-20 h-20'
}

export default function BrandMark({ size = 'md', className = '' }: BrandMarkProps) {
  const theme = useStore((state) => state.theme)
  const src = theme === 'light'
    ? 'icons/use-in-light-theme.png'
    : 'icons/use-in-dark-theme.png'

  return (
    <img
      src={src}
      alt="Azravibe IDE"
      draggable={false}
      className={`${sizes[size]} object-contain select-none ${className}`}
    />
  )
}
