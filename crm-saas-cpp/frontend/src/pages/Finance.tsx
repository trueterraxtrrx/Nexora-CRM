import { useState } from 'react'
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import { useForm } from 'react-hook-form'
import { zodResolver } from '@hookform/resolvers/zod'
import { z } from 'zod'
import { Plus, TrendingUp, TrendingDown, DollarSign, Trash2 } from 'lucide-react'
import {
  BarChart, Bar, XAxis, YAxis, CartesianGrid,
  Tooltip, ResponsiveContainer, Legend,
} from 'recharts'
import { financeApi } from '../api/client'
import { Button, Card, Badge, Modal, Input, StatCard } from '../components/ui'
import { format } from 'date-fns'
import clsx from 'clsx'

const MONTHS = ['Янв','Фев','Мар','Апр','Май','Июн','Июл','Авг','Сен','Окт','Ноя','Дек']

const schema = z.object({
  type:        z.enum(['income','expense']),
  amount:      z.number({ invalid_type_error: 'Введите сумму' }).positive('Должно быть > 0'),
  description: z.string().optional(),
  category:    z.string().optional(),
  date:        z.string().optional(),
})
type FormData = z.infer<typeof schema>

const TYPE_LABELS = { income: 'Доход', expense: 'Расход' }

export default function Finance() {
  const [isModalOpen, setModal]   = useState(false)
  const [typeFilter, setFilter]   = useState<'income'|'expense'|''>('')
  const year = new Date().getFullYear()
  const qc = useQueryClient()

  const { data: records = [], isLoading } = useQuery({
    queryKey: ['finance', typeFilter],
    queryFn: () => financeApi.list({ type: typeFilter || undefined }),
  })

  const { data: report } = useQuery({
    queryKey: ['finance-report', year],
    queryFn: () => financeApi.report({ year }),
  })

  const create = useMutation({
    mutationFn: financeApi.create,
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['finance'] })
      qc.invalidateQueries({ queryKey: ['finance-report'] })
    },
  })

  const remove = useMutation({
    mutationFn: financeApi.delete,
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['finance'] })
      qc.invalidateQueries({ queryKey: ['finance-report'] })
    },
  })

  const { register, handleSubmit, reset, formState: { errors } } = useForm<FormData>({
    resolver: zodResolver(schema),
    defaultValues: { type: 'income' },
  })

  const onSubmit = async (data: FormData) => {
    await create.mutateAsync(data)
    reset()
    setModal(false)
  }

  const chartData = report?.by_month.map(m => ({
    name:    MONTHS[m.month - 1],
    Доходы:  m.income,
    Расходы: m.expense,
    Прибыль: m.income - m.expense,
  })) ?? []

  return (
    <div className="p-6 space-y-5">
      <div className="flex items-center justify-between">
        <h2 className="text-slate-100 font-semibold text-xl">Финансы</h2>
        <Button leftIcon={<Plus size={14} />} onClick={() => setModal(true)}>
          Добавить запись
        </Button>
      </div>

      {/* Стат-карточки */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <StatCard
          label="Доходы за год"
          value={report ? `${report.total_income.toLocaleString('ru')} ₽` : '—'}
          icon={<TrendingUp size={16} />}
          deltaPositive
        />
        <StatCard
          label="Расходы за год"
          value={report ? `${report.total_expense.toLocaleString('ru')} ₽` : '—'}
          icon={<TrendingDown size={16} />}
        />
        <StatCard
          label="Прибыль"
          value={report ? `${report.profit.toLocaleString('ru')} ₽` : '—'}
          deltaPositive={report ? report.profit >= 0 : undefined}
          icon={<DollarSign size={16} />}
        />
      </div>

      {/* График */}
      {chartData.length > 0 && (
        <Card padding="md">
          <h3 className="text-slate-300 font-medium text-sm mb-4">По месяцам, {year}</h3>
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={chartData} margin={{ top: 0, right: 0, left: -20, bottom: 0 }}>
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

      {/* Фильтр + Таблица */}
      <Card padding="none">
        <div className="flex items-center gap-2 px-5 py-3 border-b border-slate-800">
          {(['', 'income', 'expense'] as const).map(f => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={clsx(
                'px-3 py-1.5 rounded-lg text-xs font-medium transition-colors',
                typeFilter === f
                  ? 'bg-indigo-600 text-white'
                  : 'bg-slate-800 text-slate-400 hover:text-slate-200'
              )}
            >
              {f === '' ? 'Все' : TYPE_LABELS[f]}
            </button>
          ))}
        </div>

        {isLoading ? (
          <div className="flex justify-center py-12">
            <div className="w-6 h-6 border-2 border-indigo-600 border-t-transparent rounded-full animate-spin" />
          </div>
        ) : (
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-slate-800">
                  {['Тип', 'Сумма', 'Описание', 'Категория', 'Дата', ''].map(h => (
                    <th key={h} className="text-left px-5 py-3 text-xs font-medium text-slate-500 uppercase tracking-wide">
                      {h}
                    </th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {records.map(r => (
                  <tr key={r.id} className="border-b border-slate-800/50 hover:bg-slate-800/20 transition-colors">
                    <td className="px-5 py-3">
                      <Badge variant={r.type === 'income' ? 'success' : 'danger'}>
                        {TYPE_LABELS[r.type]}
                      </Badge>
                    </td>
                    <td className="px-5 py-3">
                      <span className={clsx(
                        'text-sm font-semibold',
                        r.type === 'income' ? 'text-emerald-400' : 'text-red-400'
                      )}>
                        {r.type === 'income' ? '+' : '−'}{Number(r.amount).toLocaleString('ru')} {r.currency}
                      </span>
                    </td>
                    <td className="px-5 py-3 text-sm text-slate-400">{r.description ?? '—'}</td>
                    <td className="px-5 py-3 text-sm text-slate-500">{r.category ?? '—'}</td>
                    <td className="px-5 py-3 text-xs text-slate-500">
                      {format(new Date(r.date), 'dd.MM.yyyy')}
                    </td>
                    <td className="px-5 py-3">
                      <button
                        onClick={() => remove.mutate(r.id)}
                        className="text-slate-600 hover:text-red-400 transition-colors"
                      >
                        <Trash2 size={14} />
                      </button>
                    </td>
                  </tr>
                ))}
                {records.length === 0 && (
                  <tr>
                    <td colSpan={6} className="text-center py-10 text-slate-600 text-sm">
                      Нет записей
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        )}
      </Card>

      {/* Модал */}
      <Modal isOpen={isModalOpen} onClose={() => { setModal(false); reset() }} title="Новая запись">
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
          <div className="flex flex-col gap-1">
            <label className="text-sm text-slate-300 font-medium">Тип</label>
            <div className="grid grid-cols-2 gap-2">
              {(['income', 'expense'] as const).map(t => (
                <label key={t} className="cursor-pointer">
                  <input type="radio" value={t} className="sr-only" {...register('type')} />
                  <div className={clsx(
                    'text-center py-2 rounded-lg border text-sm font-medium transition-colors',
                    'border-slate-700 text-slate-400 hover:border-slate-600'
                  )}>
                    {TYPE_LABELS[t]}
                  </div>
                </label>
              ))}
            </div>
          </div>
          <Input
            label="Сумма *"
            type="number"
            step="0.01"
            error={errors.amount?.message}
            {...register('amount', { valueAsNumber: true })}
          />
          <Input label="Описание" {...register('description')} />
          <div className="grid grid-cols-2 gap-3">
            <Input label="Категория" {...register('category')} />
            <Input label="Дата" type="date" {...register('date')} />
          </div>
          <div className="flex gap-2 pt-2">
            <Button type="button" variant="secondary" className="flex-1"
              onClick={() => { setModal(false); reset() }}>
              Отмена
            </Button>
            <Button type="submit" className="flex-1" isLoading={create.isPending}>
              Добавить
            </Button>
          </div>
        </form>
      </Modal>
    </div>
  )
}
