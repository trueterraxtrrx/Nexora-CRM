import { NavLink } from 'react-router-dom'
import {
  LayoutDashboard, Users, CheckSquare, DollarSign,
  Settings, LogOut, Building2, ChevronRight,
} from 'lucide-react'
import { useAuth } from '../../hooks/useAuth'
import clsx from 'clsx'

const NAV_ITEMS = [
  { to: '/dashboard',  icon: LayoutDashboard, label: 'Дашборд'  },
  { to: '/clients',    icon: Users,            label: 'Клиенты'  },
  { to: '/tasks',      icon: CheckSquare,      label: 'Задачи'   },
  { to: '/finance',    icon: DollarSign,       label: 'Финансы'  },
  { to: '/settings',   icon: Settings,         label: 'Настройки'},
]

const PLAN_COLORS: Record<string, string> = {
  free:       'bg-slate-700 text-slate-300',
  pro:        'bg-indigo-900 text-indigo-300',
  enterprise: 'bg-amber-900 text-amber-300',
}

export function Sidebar() {
  const { user, logout } = useAuth()

  return (
    <aside className="w-60 flex-shrink-0 bg-[#0D1117] border-r border-slate-800 flex flex-col h-screen sticky top-0">
      {/* Логотип */}
      <div className="px-5 py-5 border-b border-slate-800">
        <div className="flex items-center gap-2">
          <div className="w-8 h-8 rounded-lg bg-indigo-600 flex items-center justify-center">
            <Building2 size={16} className="text-white" />
          </div>
          <span className="font-semibold text-white tracking-tight">CRM SaaS</span>
        </div>
      </div>

      {/* Навигация */}
      <nav className="flex-1 px-3 py-4 space-y-0.5 overflow-y-auto">
        {NAV_ITEMS.map(({ to, icon: Icon, label }) => (
          <NavLink
            key={to}
            to={to}
            className={({ isActive }) =>
              clsx(
                'flex items-center gap-3 px-3 py-2 rounded-lg text-sm font-medium transition-colors group',
                isActive
                  ? 'bg-indigo-600 text-white'
                  : 'text-slate-400 hover:text-slate-100 hover:bg-slate-800'
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

      {/* Профиль пользователя */}
      <div className="border-t border-slate-800 p-3">
        <div className="flex items-center gap-3 px-2 py-2">
          {/* Аватар — инициалы */}
          <div className="w-8 h-8 rounded-full bg-indigo-600 flex items-center justify-center flex-shrink-0">
            <span className="text-white text-xs font-semibold">
              {user?.full_name?.charAt(0)?.toUpperCase() ?? user?.email?.charAt(0)?.toUpperCase()}
            </span>
          </div>
          <div className="flex-1 min-w-0">
            <p className="text-sm text-slate-200 font-medium truncate">
              {user?.full_name ?? user?.email}
            </p>
            <span className={clsx(
              'text-[10px] font-medium px-1.5 py-0.5 rounded uppercase tracking-wide',
              PLAN_COLORS['free'] // TODO: взять из company
            )}>
              {user?.role}
            </span>
          </div>
        </div>

        <button
          onClick={logout}
          className="mt-1 w-full flex items-center gap-3 px-3 py-2 rounded-lg text-sm text-slate-400 hover:text-red-400 hover:bg-slate-800 transition-colors"
        >
          <LogOut size={16} />
          Выйти
        </button>
      </div>
    </aside>
  )
}
