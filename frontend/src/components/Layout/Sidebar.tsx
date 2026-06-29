import { NavLink } from 'react-router-dom'
import {
  Building2,
  CheckSquare,
  ChevronRight,
  DollarSign,
  LayoutDashboard,
  LogOut,
  Settings,
  Users,
} from 'lucide-react'
import clsx from 'clsx'
import { useAuth } from '../../hooks/useAuth'

const NAV_ITEMS = [
  { to: '/dashboard', icon: LayoutDashboard, label: 'Dashboard' },
  { to: '/clients', icon: Users, label: 'Clients' },
  { to: '/tasks', icon: CheckSquare, label: 'Tasks' },
  { to: '/finance', icon: DollarSign, label: 'Finance' },
  { to: '/settings', icon: Settings, label: 'Settings' },
]

const ROLE_COLORS: Record<string, string> = {
  admin: 'bg-teal-950 text-signal',
  manager: 'bg-slate-800 text-slate-300',
  user: 'bg-amber-950 text-warn',
}

export function Sidebar() {
  const { user, logout } = useAuth()

  return (
    <aside className="w-60 flex-shrink-0 bg-surface border-r border-line flex flex-col h-screen sticky top-0">
      <div className="px-5 py-5 border-b border-line">
        <div className="flex items-center gap-2">
          <div className="w-8 h-8 rounded bg-panel flex items-center justify-center">
            <Building2 size={16} className="text-signal" />
          </div>
          <span className="font-semibold text-white tracking-tight">Nexora CRM</span>
        </div>
      </div>

      <nav className="flex-1 px-3 py-4 space-y-0.5 overflow-y-auto">
        {NAV_ITEMS.map(({ to, icon: Icon, label }) => (
          <NavLink
            key={to}
            to={to}
            className={({ isActive }) =>
              clsx(
                'flex items-center gap-3 px-3 py-2 rounded-lg text-sm font-medium transition-colors group',
                isActive
                  ? 'bg-panel text-white'
                  : 'text-slate-300 hover:text-white hover:bg-panel/70'
              )
            }
          >
            {({ isActive }) => (
              <>
                <Icon size={17} />
                <span className="flex-1">{label}</span>
                {isActive && <ChevronRight size={14} className="opacity-60" />}
              </>
            )}
          </NavLink>
        ))}
      </nav>

      <div className="border-t border-line p-3">
        <div className="flex items-center gap-3 px-2 py-2">
          <div className="w-8 h-8 rounded-full bg-signal flex items-center justify-center flex-shrink-0">
            <span className="text-white text-xs font-semibold">
              {user?.full_name?.charAt(0)?.toUpperCase() ?? user?.email?.charAt(0)?.toUpperCase()}
            </span>
          </div>
          <div className="flex-1 min-w-0">
            <p className="text-sm text-white font-medium truncate">
              {user?.full_name ?? user?.email}
            </p>
            <span className={clsx(
              'text-[10px] font-medium px-1.5 py-0.5 rounded uppercase tracking-wide',
              ROLE_COLORS[user?.role ?? 'user']
            )}>
              {user?.role}
            </span>
          </div>
        </div>

        <button
          onClick={logout}
          className="mt-1 w-full flex items-center gap-3 px-3 py-2 rounded-lg text-sm text-slate-300 hover:text-danger hover:bg-panel transition-colors"
        >
          <LogOut size={16} />
          Reset demo session
        </button>
      </div>
    </aside>
  )
}
// Project version: Nexora CRM V2.3
