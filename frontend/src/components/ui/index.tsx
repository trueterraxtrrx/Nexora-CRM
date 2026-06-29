import { type ReactNode, type ButtonHTMLAttributes, type InputHTMLAttributes } from 'react'
import { Loader2, X } from 'lucide-react'
import clsx from 'clsx'

// ─────────────────────────────────────────────
// BUTTON
// ─────────────────────────────────────────────

interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'primary' | 'secondary' | 'danger' | 'ghost'
  size?: 'sm' | 'md' | 'lg'
  isLoading?: boolean
  leftIcon?: ReactNode
}

export function Button({
  variant = 'primary',
  size = 'md',
  isLoading,
  leftIcon,
  children,
  className,
  disabled,
  ...props
}: ButtonProps) {
  const base = 'inline-flex items-center justify-center gap-2 font-medium rounded-lg transition-colors focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-offset-[#0D1117] disabled:opacity-50 disabled:cursor-not-allowed'

  const variants = {
    primary:   'bg-indigo-600 text-white hover:bg-indigo-500 focus:ring-indigo-500',
    secondary: 'bg-slate-800 text-slate-200 hover:bg-slate-700 border border-slate-700 focus:ring-slate-500',
    danger:    'bg-red-600 text-white hover:bg-red-500 focus:ring-red-500',
    ghost:     'text-slate-400 hover:text-slate-200 hover:bg-slate-800 focus:ring-slate-500',
  }

  const sizes = {
    sm: 'px-3 py-1.5 text-xs',
    md: 'px-4 py-2 text-sm',
    lg: 'px-5 py-2.5 text-base',
  }

  return (
    <button
      className={clsx(base, variants[variant], sizes[size], className)}
      disabled={disabled || isLoading}
      {...props}
    >
      {isLoading ? <Loader2 size={14} className="animate-spin" /> : leftIcon}
      {children}
    </button>
  )
}

// ─────────────────────────────────────────────
// CARD
// ─────────────────────────────────────────────

interface CardProps {
  children: ReactNode
  className?: string
  padding?: 'none' | 'sm' | 'md' | 'lg'
}

export function Card({ children, className, padding = 'md' }: CardProps) {
  const paddings = { none: '', sm: 'p-4', md: 'p-5', lg: 'p-6' }
  return (
    <div
      className={clsx(
        'bg-[#161B22] border border-slate-800 rounded-xl',
        paddings[padding],
        className
      )}
    >
      {children}
    </div>
  )
}

// ─────────────────────────────────────────────
// BADGE
// ─────────────────────────────────────────────

type BadgeVariant = 'default' | 'success' | 'warning' | 'danger' | 'info' | 'purple'

interface BadgeProps {
  children: ReactNode
  variant?: BadgeVariant
  className?: string
}

export function Badge({ children, variant = 'default', className }: BadgeProps) {
  const variants: Record<BadgeVariant, string> = {
    default: 'bg-slate-800 text-slate-400',
    success: 'bg-emerald-950 text-emerald-400',
    warning: 'bg-amber-950 text-amber-400',
    danger:  'bg-red-950 text-red-400',
    info:    'bg-sky-950 text-sky-400',
    purple:  'bg-purple-950 text-purple-400',
  }

  return (
    <span
      className={clsx(
        'inline-flex items-center px-2 py-0.5 rounded-md text-xs font-medium',
        variants[variant],
        className
      )}
    >
      {children}
    </span>
  )
}

// ─────────────────────────────────────────────
// INPUT
// ─────────────────────────────────────────────

interface InputProps extends InputHTMLAttributes<HTMLInputElement> {
  label?: string
  error?: string
  hint?: string
}

export function Input({ label, error, hint, className, id, ...props }: InputProps) {
  const inputId = id ?? label?.toLowerCase().replace(/\s+/g, '-')

  return (
    <div className="flex flex-col gap-1">
      {label && (
        <label htmlFor={inputId} className="text-sm text-slate-300 font-medium">
          {label}
        </label>
      )}
      <input
        id={inputId}
        className={clsx(
          'w-full bg-slate-900 border rounded-lg px-3 py-2 text-sm text-slate-200 placeholder:text-slate-500',
          'focus:outline-none focus:ring-1 focus:ring-indigo-500 transition',
          error
            ? 'border-red-500 focus:ring-red-500'
            : 'border-slate-700 hover:border-slate-600',
          className
        )}
        {...props}
      />
      {hint && !error && <p className="text-xs text-slate-500">{hint}</p>}
      {error && <p className="text-xs text-red-400">{error}</p>}
    </div>
  )
}

// ─────────────────────────────────────────────
// MODAL
// ─────────────────────────────────────────────

interface ModalProps {
  isOpen: boolean
  onClose: () => void
  title: string
  children: ReactNode
  size?: 'sm' | 'md' | 'lg'
}

export function Modal({ isOpen, onClose, title, children, size = 'md' }: ModalProps) {
  if (!isOpen) return null

  const sizes = {
    sm: 'max-w-sm',
    md: 'max-w-lg',
    lg: 'max-w-2xl',
  }

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-sm"
      onClick={(e) => { if (e.target === e.currentTarget) onClose() }}
    >
      <div className={clsx('w-full bg-[#161B22] border border-slate-700 rounded-xl shadow-xl', sizes[size])}>
        {/* Заголовок */}
        <div className="flex items-center justify-between px-5 py-4 border-b border-slate-800">
          <h2 className="text-slate-100 font-semibold">{title}</h2>
          <button
            onClick={onClose}
            className="text-slate-500 hover:text-slate-200 transition-colors"
          >
            <X size={18} />
          </button>
        </div>

        {/* Контент */}
        <div className="p-5">{children}</div>
      </div>
    </div>
  )
}

// ─────────────────────────────────────────────
// EMPTY STATE
// ─────────────────────────────────────────────

interface EmptyStateProps {
  icon?: ReactNode
  title: string
  description?: string
  action?: ReactNode
}

export function EmptyState({ icon, title, description, action }: EmptyStateProps) {
  return (
    <div className="flex flex-col items-center justify-center py-16 px-4 text-center">
      {icon && (
        <div className="w-12 h-12 rounded-full bg-slate-800 flex items-center justify-center mb-4 text-slate-500">
          {icon}
        </div>
      )}
      <h3 className="text-slate-300 font-medium mb-1">{title}</h3>
      {description && <p className="text-slate-500 text-sm mb-4">{description}</p>}
      {action}
    </div>
  )
}

// ─────────────────────────────────────────────
// STAT CARD
// ─────────────────────────────────────────────

interface StatCardProps {
  label: string
  value: string | number
  delta?: string
  deltaPositive?: boolean
  icon?: ReactNode
}

export function StatCard({ label, value, delta, deltaPositive, icon }: StatCardProps) {
  return (
    <Card>
      <div className="flex items-start justify-between">
        <div>
          <p className="text-xs text-slate-500 font-medium uppercase tracking-wide">{label}</p>
          <p className="text-2xl font-semibold text-slate-100 mt-1">{value}</p>
          {delta && (
            <p className={clsx('text-xs mt-1', deltaPositive ? 'text-emerald-400' : 'text-red-400')}>
              {deltaPositive ? '↑' : '↓'} {delta}
            </p>
          )}
        </div>
        {icon && (
          <div className="w-9 h-9 rounded-lg bg-indigo-600/20 flex items-center justify-center text-indigo-400">
            {icon}
          </div>
        )}
      </div>
    </Card>
  )
}
// Project version: Nexora CRM V2.3
