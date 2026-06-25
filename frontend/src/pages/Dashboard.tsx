import { useQuery } from '@tanstack/react-query'
import {
  AreaChart, Area, XAxis, YAxis, CartesianGrid,
  Tooltip, ResponsiveContainer, BarChart, Bar, Legend,
} from 'recharts'
import { Users, CheckSquare, TrendingUp, TrendingDown, Clock, AlertCircle } from 'lucide-react'
import { Card, StatCard, Badge } from '../components/ui'
import { financeApi, tasksApi, clientsApi } from '../api/client'
import { format } from 'date-fns'
import { ru } from 'date-fns/locale'

const MONTHS = ['Янв','Фев','Мар','Апр','Май','Июн','Июл','Авг','Сен','Окт','Ноя','Дек']

const PRIORITY_COLORS: Record<string, string> = {
  urgent: 'danger',
  high:   'warning',
  medium: 'info',
  low:    'default',
} as const

const STATUS_LABELS: Record<string, string> = {
  todo:        'К выполнению',
  in_progress: 'В работе',
  done:        'Готово',
  cancelled:   'Отменено',
}

export default function Dashboard() {
  const year = new Date().getFullYear()

  const { data: report } = useQuery({
    queryKey: ['finance-report', year],
    queryFn: () => financeApi.report({ year }),
  })

  const { data: tasks } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => tasksApi.list({ limit: 10 }),
  })

  const { data: clients } = useQuery({
    queryKey: ['clients'],
    queryFn: () => clientsApi.list({ limit: 5, is_active: true }),
  })

  const chartData = report?.by_month.map((m) => ({
    name: MONTHS[m.month - 1],
    Доходы:  m.income,
    Расходы: m.expense,
    Прибыль: m.income - m.expense,
  })) ?? []

  const activeTasks  = tasks?.filter(t => t.status !== 'done' && t.status !== 'cancelled') ?? []
  const overdueTasks = activeTasks.filter(
    t => t.deadline && new Date(t.deadline) < new Date()
  )

  return (
    <div className="p-6 space-y-6">
      {/* Заголовок */}
      <div>
        <h2 className="text-slate-100 text-xl font-semibold">Обзор</h2>
        <p className="text-slate-500 text-sm mt-0.5">
          {format(new Date(), 'd MMMM yyyy', { locale: ru })}
        </p>
      </div>

      {/* Стат-карточки */}
      <div className="grid grid-cols-2 xl:grid-cols-4 gap-4">
        <StatCard
          label="Клиентов"
          value={clients?.length ?? '—'}
          icon={<Users size={16} />}
        />
        <StatCard
          label="Активных задач"
          value={activeTasks.length}
          icon={<CheckSquare size={16} />}
        />
        <StatCard
          label="Доходы (месяц)"
          value={report ? `${report.total_income.toLocaleString('ru')} ₽` : '—'}
          icon={<TrendingUp size={16} />}
          deltaPositive
        />
        <StatCard
          label="Расходы (месяц)"
          value={report ? `${report.total_expense.toLocaleString('ru')} ₽` : '—'}
          icon={<TrendingDown size={16} />}
        />
      </div>

      {/* Просроченные */}
      {overdueTasks.length > 0 && (
        <div className="flex items-center gap-3 px-4 py-3 bg-red-950/40 border border-red-900/50 rounded-xl">
          <AlertCircle size={16} className="text-red-400 flex-shrink-0" />
          <span className="text-sm text-red-300">
            <span className="font-semibold">{overdueTasks.length}</span> задач просрочено — требуют внимания
          </span>
        </div>
      )}

      <div className="grid grid-cols-1 xl:grid-cols-3 gap-6">
        {/* График доходов/расходов */}
        <Card className="xl:col-span-2" padding="md">
          <h3 className="text-slate-300 font-medium text-sm mb-4">Финансы за {year}</h3>
          <ResponsiveContainer width="100%" height={220}>
            <AreaChart data={chartData} margin={{ top: 0, right: 0, left: -20, bottom: 0 }}>
              <defs>
                <linearGradient id="income" x1="0" y1="0" x2="0" y2="1">
                  <stop offset="0%" stopColor="#6366f1" stopOpacity={0.3} />
                  <stop offset="100%" stopColor="#6366f1" stopOpacity={0} />
                </linearGradient>
                <linearGradient id="expense" x1="0" y1="0" x2="0" y2="1">
                  <stop offset="0%" stopColor="#f43f5e" stopOpacity={0.3} />
                  <stop offset="100%" stopColor="#f43f5e" stopOpacity={0} />
                </linearGradient>
              </defs>
              <CartesianGrid strokeDasharray="3 3" stroke="#1e293b" />
              <XAxis dataKey="name" tick={{ fill: '#64748b', fontSize: 11 }} axisLine={false} tickLine={false} />
              <YAxis tick={{ fill: '#64748b', fontSize: 11 }} axisLine={false} tickLine={false} />
              <Tooltip
                contentStyle={{ background: '#1e293b', border: '1px solid #334155', borderRadius: 8, fontSize: 12 }}
                labelStyle={{ color: '#94a3b8' }}
              />
              <Area type="monotone" dataKey="Доходы"  stroke="#6366f1" fill="url(#income)"  strokeWidth={2} />
              <Area type="monotone" dataKey="Расходы" stroke="#f43f5e" fill="url(#expense)" strokeWidth={2} />
            </AreaChart>
          </ResponsiveContainer>
        </Card>

        {/* Последние задачи */}
        <Card padding="md">
          <div className="flex items-center justify-between mb-4">
            <h3 className="text-slate-300 font-medium text-sm">Последние задачи</h3>
            <Clock size={14} className="text-slate-600" />
          </div>
          <div className="space-y-3">
            {(tasks ?? []).slice(0, 6).map(task => (
              <div key={task.id} className="flex items-start gap-2.5">
                <Badge variant={PRIORITY_COLORS[task.priority] as any} className="mt-0.5 flex-shrink-0">
                  {task.priority}
                </Badge>
                <div className="flex-1 min-w-0">
                  <p className="text-sm text-slate-300 truncate">{task.title}</p>
                  <p className="text-xs text-slate-600">{STATUS_LABELS[task.status]}</p>
                </div>
              </div>
            ))}
            {!tasks?.length && (
              <p className="text-sm text-slate-600 text-center py-4">Нет задач</p>
            )}
          </div>
        </Card>
      </div>

      {/* Прибыль по категориям */}
      {report?.by_category && Object.keys(report.by_category).length > 0 && (
        <Card padding="md">
          <h3 className="text-slate-300 font-medium text-sm mb-4">По категориям</h3>
          <ResponsiveContainer width="100%" height={180}>
            <BarChart
              data={Object.entries(report.by_category).map(([cat, vals]) => ({
                name: cat,
                Доходы: vals.income,
                Расходы: vals.expense,
              }))}
              margin={{ top: 0, right: 0, left: -20, bottom: 0 }}
            >
              <CartesianGrid strokeDasharray="3 3" stroke="#1e293b" />
              <XAxis dataKey="name" tick={{ fill: '#64748b', fontSize: 11 }} axisLine={false} tickLine={false} />
              <YAxis tick={{ fill: '#64748b', fontSize: 11 }} axisLine={false} tickLine={false} />
              <Tooltip
                contentStyle={{ background: '#1e293b', border: '1px solid #334155', borderRadius: 8, fontSize: 12 }}
              />
              <Legend wrapperStyle={{ fontSize: 12, color: '#94a3b8' }} />
              <Bar dataKey="Доходы"  fill="#6366f1" radius={[4,4,0,0]} />
              <Bar dataKey="Расходы" fill="#f43f5e" radius={[4,4,0,0]} />
            </BarChart>
          </ResponsiveContainer>
        </Card>
      )}
    </div>
  )
}
