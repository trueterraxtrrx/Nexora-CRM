import { useState } from 'react'
import { useForm } from 'react-hook-form'
import { zodResolver } from '@hookform/resolvers/zod'
import { z } from 'zod'
import { Plus, Search, UserPlus, Phone, Mail, Building } from 'lucide-react'
import {
  useClients, useCreateClient, useDeleteClient
} from '../hooks/useClients'
import {
  Button, Card, Badge, Modal, Input, EmptyState
} from '../components/ui'
import { format } from 'date-fns'

// ─────────────────────────────────────────────
// Валидация формы
// ─────────────────────────────────────────────

const schema = z.object({
  name:         z.string().min(2, 'Минимум 2 символа'),
  email:        z.string().email('Неверный email').optional().or(z.literal('')),
  phone:        z.string().optional(),
  company_name: z.string().optional(),
  notes:        z.string().optional(),
})

type FormData = z.infer<typeof schema>

// ─────────────────────────────────────────────
// Компонент
// ─────────────────────────────────────────────

export default function Clients() {
  const [search, setSearch]       = useState('')
  const [isModalOpen, setModal]   = useState(false)
  const [activeFilter, setFilter] = useState<boolean | undefined>(undefined)

  const { data: clients, isLoading } = useClients({
    search: search || undefined,
    is_active: activeFilter,
  })
  const createClient = useCreateClient()
  const deleteClient = useDeleteClient()

  const {
    register, handleSubmit, reset,
    formState: { errors },
  } = useForm<FormData>({ resolver: zodResolver(schema) })

  const onSubmit = async (data: FormData) => {
    await createClient.mutateAsync({
      ...data,
      email: data.email || undefined,
    })
    reset()
    setModal(false)
  }

  return (
    <div className="p-6 space-y-5">
      {/* Тулбар */}
      <div className="flex items-center gap-3 flex-wrap">
        <div className="relative flex-1 min-w-[200px] max-w-sm">
          <Search size={15} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500" />
          <input
            value={search}
            onChange={e => setSearch(e.target.value)}
            placeholder="Поиск по имени, email, телефону..."
            className="w-full bg-slate-900 border border-slate-700 rounded-lg pl-9 pr-4 py-2 text-sm text-slate-300 placeholder:text-slate-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
          />
        </div>

        {/* Фильтры */}
        <div className="flex gap-1.5">
          {[
            { label: 'Все',      value: undefined },
            { label: 'Активные', value: true      },
            { label: 'Архив',    value: false     },
          ].map(({ label, value }) => (
            <button
              key={label}
              onClick={() => setFilter(value)}
              className={`px-3 py-1.5 rounded-lg text-xs font-medium transition-colors ${
                activeFilter === value
                  ? 'bg-indigo-600 text-white'
                  : 'bg-slate-800 text-slate-400 hover:text-slate-200'
              }`}
            >
              {label}
            </button>
          ))}
        </div>

        <Button leftIcon={<Plus size={14} />} onClick={() => setModal(true)} className="ml-auto">
          Добавить клиента
        </Button>
      </div>

      {/* Таблица */}
      <Card padding="none">
        {isLoading ? (
          <div className="flex justify-center py-12">
            <div className="w-6 h-6 border-2 border-indigo-600 border-t-transparent rounded-full animate-spin" />
          </div>
        ) : !clients?.length ? (
          <EmptyState
            icon={<UserPlus size={20} />}
            title="Нет клиентов"
            description={search ? 'Ничего не найдено по вашему запросу' : 'Добавьте первого клиента, чтобы начать работу'}
            action={
              !search && (
                <Button leftIcon={<Plus size={14} />} onClick={() => setModal(true)}>
                  Добавить клиента
                </Button>
              )
            }
          />
        ) : (
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-slate-800">
                  {['Имя', 'Контакты', 'Компания', 'Добавлен', 'Статус', ''].map(h => (
                    <th key={h} className="text-left px-5 py-3 text-xs font-medium text-slate-500 uppercase tracking-wide">
                      {h}
                    </th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {clients.map(client => (
                  <tr
                    key={client.id}
                    className="border-b border-slate-800/50 hover:bg-slate-800/30 transition-colors"
                  >
                    <td className="px-5 py-3.5">
                      <div className="flex items-center gap-3">
                        <div className="w-8 h-8 rounded-full bg-indigo-600/20 flex items-center justify-center flex-shrink-0">
                          <span className="text-indigo-400 text-xs font-semibold">
                            {client.name.charAt(0).toUpperCase()}
                          </span>
                        </div>
                        <span className="text-sm font-medium text-slate-200">{client.name}</span>
                      </div>
                    </td>
                    <td className="px-5 py-3.5">
                      <div className="space-y-0.5">
                        {client.email && (
                          <div className="flex items-center gap-1.5 text-xs text-slate-400">
                            <Mail size={11} className="text-slate-600" />
                            {client.email}
                          </div>
                        )}
                        {client.phone && (
                          <div className="flex items-center gap-1.5 text-xs text-slate-400">
                            <Phone size={11} className="text-slate-600" />
                            {client.phone}
                          </div>
                        )}
                      </div>
                    </td>
                    <td className="px-5 py-3.5">
                      {client.company_name ? (
                        <div className="flex items-center gap-1.5 text-xs text-slate-400">
                          <Building size={11} className="text-slate-600" />
                          {client.company_name}
                        </div>
                      ) : (
                        <span className="text-slate-600 text-xs">—</span>
                      )}
                    </td>
                    <td className="px-5 py-3.5 text-xs text-slate-500">
                      {format(new Date(client.created_at), 'dd.MM.yyyy')}
                    </td>
                    <td className="px-5 py-3.5">
                      <Badge variant={client.is_active ? 'success' : 'default'}>
                        {client.is_active ? 'Активен' : 'Архив'}
                      </Badge>
                    </td>
                    <td className="px-5 py-3.5">
                      <div className="flex gap-1 justify-end">
                        <Button
                          variant="ghost"
                          size="sm"
                          onClick={() => deleteClient.mutate(client.id)}
                        >
                          Удалить
                        </Button>
                      </div>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </Card>

      {/* Модал создания */}
      <Modal isOpen={isModalOpen} onClose={() => { setModal(false); reset() }} title="Новый клиент">
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
          <Input label="Имя *" error={errors.name?.message} {...register('name')} />
          <Input label="Email" type="email" error={errors.email?.message} {...register('email')} />
          <Input label="Телефон" {...register('phone')} />
          <Input label="Компания" {...register('company_name')} />
          <div className="flex flex-col gap-1">
            <label className="text-sm text-slate-300 font-medium">Заметки</label>
            <textarea
              className="w-full bg-slate-900 border border-slate-700 rounded-lg px-3 py-2 text-sm text-slate-200 placeholder:text-slate-500 focus:outline-none focus:ring-1 focus:ring-indigo-500 resize-none h-20"
              {...register('notes')}
            />
          </div>
          <div className="flex gap-2 pt-2">
            <Button
              type="button"
              variant="secondary"
              className="flex-1"
              onClick={() => { setModal(false); reset() }}
            >
              Отмена
            </Button>
            <Button
              type="submit"
              className="flex-1"
              isLoading={createClient.isPending}
            >
              Создать
            </Button>
          </div>
        </form>
      </Modal>
    </div>
  )
}
