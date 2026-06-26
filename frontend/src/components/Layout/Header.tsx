import { Bell, Search } from 'lucide-react'
import { useLocation } from 'react-router-dom'
import { useAuth } from '../../hooks/useAuth'

const PAGE_TITLES: Record<string, string> = {
  '/dashboard': 'Dashboard',
  '/clients': 'Clients',
  '/tasks': 'Tasks',
  '/finance': 'Finance',
  '/settings': 'Settings',
}

export function Header() {
  const { user } = useAuth()
  const location = useLocation()
  const title = PAGE_TITLES[location.pathname] ?? 'Nexora CRM'

  return (
    <header className="h-14 border-b border-line bg-surface flex items-center px-6 gap-4 sticky top-0 z-10">
      <h1 className="text-white font-semibold text-lg flex-shrink-0">{title}</h1>

      <div className="flex-1 max-w-md">
        <div className="relative">
          <Search size={15} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
          <input
            type="text"
            placeholder="Search clients, tasks..."
            className="w-full bg-bg border border-line rounded-lg pl-9 pr-4 py-1.5 text-sm text-ink placeholder:text-slate-500 focus:outline-none focus:ring-1 focus:ring-signal focus:border-signal transition"
          />
        </div>
      </div>

      <div className="flex items-center gap-2 ml-auto">
        <button
          className="relative w-8 h-8 rounded-lg flex items-center justify-center text-slate-400 hover:text-white hover:bg-panel transition-colors"
          title="Notifications"
        >
          <Bell size={16} />
          <span className="absolute top-1.5 right-1.5 w-1.5 h-1.5 rounded-full bg-signal" />
        </button>

        <div className="flex items-center gap-2 px-2">
          <div className="w-7 h-7 rounded-full bg-signal flex items-center justify-center">
            <span className="text-white text-xs font-semibold">
              {user?.full_name?.charAt(0)?.toUpperCase() ?? 'U'}
            </span>
          </div>
          <span className="text-sm text-slate-400 hidden md:block">
            {user?.full_name ?? user?.email}
          </span>
        </div>
      </div>
    </header>
  )
}
