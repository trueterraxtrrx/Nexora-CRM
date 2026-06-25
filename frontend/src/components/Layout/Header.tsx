import { Bell, Search } from 'lucide-react'
import { useAuth } from '../../hooks/useAuth'
import { useLocation } from 'react-router-dom'

const PAGE_TITLES: Record<string, string> = {
  '/dashboard': 'Дашборд',
  '/clients':   'Клиенты',
  '/tasks':     'Задачи',
  '/finance':   'Финансы',
  '/settings':  'Настройки',
}

export function Header() {
  const { user } = useAuth()
  const location = useLocation()
  const title = PAGE_TITLES[location.pathname] ?? 'CRM'

  return (
    <header className="h-14 border-b border-slate-800 bg-[#0D1117] flex items-center px-6 gap-4 sticky top-0 z-10">
      {/* Заголовок страницы */}
      <h1 className="text-slate-100 font-semibold text-lg flex-shrink-0">{title}</h1>

      {/* Поиск */}
      <div className="flex-1 max-w-md">
        <div className="relative">
          <Search size={15} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500" />
          <input
            type="text"
            placeholder="Поиск клиентов, задач..."
            className="w-full bg-slate-800 border border-slate-700 rounded-lg pl-9 pr-4 py-1.5 text-sm text-slate-300 placeholder:text-slate-500 focus:outline-none focus:ring-1 focus:ring-indigo-500 focus:border-indigo-500 transition"
          />
        </div>
      </div>

      <div className="flex items-center gap-2 ml-auto">
        {/* Уведомления */}
        <button className="relative w-8 h-8 rounded-lg flex items-center justify-center text-slate-400 hover:text-slate-200 hover:bg-slate-800 transition-colors">
          <Bell size={16} />
          <span className="absolute top-1.5 right-1.5 w-1.5 h-1.5 rounded-full bg-indigo-500" />
        </button>

        {/* Мини-профиль */}
        <div className="flex items-center gap-2 px-2">
          <div className="w-7 h-7 rounded-full bg-indigo-600 flex items-center justify-center">
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
