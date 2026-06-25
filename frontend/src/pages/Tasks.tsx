import { useState } from 'react'
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import { useForm } from 'react-hook-form'
import { zodResolver } from '@hookform/resolvers/zod'
import { z } from 'zod'
import { Plus, Clock, AlertTriangle } from 'lucide-react'
import { tasksApi } from '../api/client'
import { Button, Badge, Modal, Input } from '../components/ui'
import type { Task, TaskStatus } from '../types'
import { format } from 'date-fns'
import clsx from 'clsx'

// ─────────────────────────────────────────────
// Конфигурация статусов (колонки канбана)
// ─────────────────────────────────────────────

const COLUMNS: { status: TaskStatus; label: string; color: string }[] = [
  { status: 'todo',        label: 'К выполнению', color: 'border-slate-700'  },
  { status: 'in_progress', label: 'В работе',     color: 'border-indigo-700' },
  { status: 'done',        label: 'Готово',        color: 'border-emerald-800'},
  { status: 'cancelled',   label: 'Отменено',      color: 'border-red-900'   },
]

const PRIORITY_BADGE: Record<string, any> = {
  urgent: 'danger',
  high:   'warning',
  medium: 'info',
  low:    'default',
}

const PRIORITY_LABELS: Record<string, string> = {
  urgent: 'Срочно',
  high:   'Высокий',
  medium: 'Средний',
  low:    'Низкий',
}

// ─────────────────────────────────────────────
// Форма
// ─────────────────────────────────────────────

const schema = z.object({
  title:       z.string().min(3, 'Минимум 3 символа'),
  description: z.string().optional(),
  priority:    z.enum(['low','medium','high','urgent']).default('medium'),
  deadline:    z.string().optional(),
})
type FormData = z.infer<typeof schema>

// ─────────────────────────────────────────────
// Task Card
// ─────────────────────────────────────────────

function TaskCard({ task, onStatusChange }: {
  task: Task
  onStatusChange: (id: number, status: TaskStatus) => void
}) {
  const isOverdue = task.deadline && new Date(task.deadline) < new Date()
    && task.status !== 'done' && task.status !== 'cancelled'

  return (
    <div className={clsx(
      'bg-[#0D1117] border border-slate-800 rounded-lg p-3.5 space-y-2.5',
      'hover:border-slate-700 transition-colors cursor-pointer group'
    )}>
      <div className="flex items-start justify-between gap-2">
        <p className="text-sm text-slate-200 font-medium leading-snug">{task.title}</p>
        <Badge variant={PRIORITY_BADGE[task.priority]}>{PRIORITY_LABELS[task.priority]}</Badge>
      </div>

      {task.description && (
        <p className="text-xs text-slate-500 line-clamp-2">{task.description}</p>
      )}

      <div className="flex items-center justify-between">
        {task.deadline ? (
          <div className={clsx(
            'flex items-center gap-1 text-xs',
            isOverdue ? 'text-red-400' : 'text-slate-500'
          )}>
            {isOverdue ? <AlertTriangle size={10} /> : <Clock size={10} />}
            {format(new Date(task.deadline), 'dd.MM.yyyy')}
          </div>
        ) : <span />}

        {/* Быстрая смена статуса */}
        <select
          value={task.status}
          onChange={e => onStatusChange(task.id, e.target.value as TaskStatus)}
          onClick={e => e.stopPropagation()}
          className="text-xs bg-slate-800 border border-slate-700 text-slate-400 rounded px-1.5 py-0.5 focus:outline-none"
        >
          <option value="todo">К выполнению</option>
          <option value="in_progress">В работе</option>
          <option value="done">Готово</option>
          <option value="cancelled">Отменено</option>
        </select>
      </div>
    </div>
  )
}

// ─────────────────────────────────────────────
// Страница
// ─────────────────────────────────────────────

export default function Tasks() {
  const [isModalOpen, setModal] = useState(false)
  const qc = useQueryClient()

  const { data: tasks = [], isLoading } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => tasksApi.list({ limit: 200 }),
  })

  const createTask = useMutation({
    mutationFn: tasksApi.create,
    onSuccess: () => qc.invalidateQueries({ queryKey: ['tasks'] }),
  })

  const updateTask = useMutation({
    mutationFn: ({ id, ...data }: { id: number; status: TaskStatus }) =>
      tasksApi.update(id, data),
    onSuccess: () => qc.invalidateQueries({ queryKey: ['tasks'] }),
  })

  const { register, handleSubmit, reset, formState: { errors } } = useForm<FormData>({
    resolver: zodResolver(schema),
    defaultValues: { priority: 'medium' },
  })

  const onSubmit = async (data: FormData) => {
    await createTask.mutateAsync({
      ...data,
      deadline: data.deadline || undefined,
    })
    reset()
    setModal(false)
  }

  const handleStatusChange = (id: number, status: TaskStatus) => {
    updateTask.mutate({ id, status })
  }

  return (
    <div className="p-6 space-y-5">
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-slate-100 font-semibold text-xl">Задачи</h2>
          <p className="text-slate-500 text-sm mt-0.5">{tasks.length} задач всего</p>
        </div>
        <Button leftIcon={<Plus size={14} />} onClick={() => setModal(true)}>
          Новая задача
        </Button>
      </div>

      {isLoading ? (
        <div className="flex justify-center py-16">
          <div className="w-6 h-6 border-2 border-indigo-600 border-t-transparent rounded-full animate-spin" />
        </div>
      ) : (
        /* Канбан */
        <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-4 gap-4">
          {COLUMNS.map(col => {
            const colTasks = tasks.filter(t => t.status === col.status)
            return (
              <div key={col.status} className="space-y-3">
                {/* Заголовок колонки */}
                <div className={clsx(
                  'flex items-center justify-between px-3 py-2 rounded-lg border-l-2 bg-slate-900/50',
                  col.color
                )}>
                  <span className="text-sm font-medium text-slate-300">{col.label}</span>
                  <span className="text-xs text-slate-500 bg-slate-800 px-1.5 py-0.5 rounded">
                    {colTasks.length}
                  </span>
                </div>

                {/* Карточки */}
                <div className="space-y-2.5">
                  {colTasks.map(task => (
                    <TaskCard
                      key={task.id}
                      task={task}
                      onStatusChange={handleStatusChange}
                    />
                  ))}
                  {colTasks.length === 0 && (
                    <div className="text-center py-6 text-slate-600 text-xs border border-dashed border-slate-800 rounded-lg">
                      Нет задач
                    </div>
                  )}
                </div>
              </div>
            )
          })}
        </div>
      )}

      {/* Модал создания */}
      <Modal isOpen={isModalOpen} onClose={() => { setModal(false); reset() }} title="Новая задача">
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
          <Input label="Название *" error={errors.title?.message} {...register('title')} />
          <div className="flex flex-col gap-1">
            <label className="text-sm text-slate-300 font-medium">Описание</label>
            <textarea
              className="w-full bg-slate-900 border border-slate-700 rounded-lg px-3 py-2 text-sm text-slate-200 focus:outline-none focus:ring-1 focus:ring-indigo-500 resize-none h-20"
              {...register('description')}
            />
          </div>
          <div className="grid grid-cols-2 gap-3">
            <div className="flex flex-col gap-1">
              <label className="text-sm text-slate-300 font-medium">Приоритет</label>
              <select
                className="bg-slate-900 border border-slate-700 rounded-lg px-3 py-2 text-sm text-slate-200 focus:outline-none focus:ring-1 focus:ring-indigo-500"
                {...register('priority')}
              >
                <option value="low">Низкий</option>
                <option value="medium">Средний</option>
                <option value="high">Высокий</option>
                <option value="urgent">Срочно</option>
              </select>
            </div>
            <Input label="Дедлайн" type="datetime-local" {...register('deadline')} />
          </div>
          <div className="flex gap-2 pt-2">
            <Button type="button" variant="secondary" className="flex-1"
              onClick={() => { setModal(false); reset() }}>
              Отмена
            </Button>
            <Button type="submit" className="flex-1" isLoading={createTask.isPending}>
              Создать
            </Button>
          </div>
        </form>
      </Modal>
    </div>
  )
}
