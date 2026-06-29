import { useState } from 'react'
import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query'
import { useForm } from 'react-hook-form'
import { zodResolver } from '@hookform/resolvers/zod'
import { z } from 'zod'
import { ShieldCheck, UserPlus, Trash2, Mail, Crown } from 'lucide-react'
import { usersApi } from '../api/client'
import { Badge, Button, Card, EmptyState, Input } from '../components/ui'
import { useAuth } from '../hooks/useAuth'
import type { User, UserRole } from '../types'
import { format } from 'date-fns'

const schema = z.object({
  email: z.string().email('Введите корректный email'),
  password: z.string().min(8, 'Минимум 8 символов'),
  full_name: z.string().min(2, 'Минимум 2 символа'),
  role: z.enum(['admin', 'manager', 'user']).default('user'),
})

type FormData = z.infer<typeof schema>

const DEMO_USERS: User[] = [
  {
    id: 1,
    company_id: 1,
    email: 'admin@nexora.local',
    full_name: 'Nexora Admin',
    role: 'admin',
    is_active: true,
    avatar_url: null,
    notify_email: true,
    telegram_chat_id: null,
    created_at: new Date().toISOString(),
    last_login: new Date().toISOString(),
  },
  {
    id: 2,
    company_id: 1,
    email: 'manager@nexora.local',
    full_name: 'Nexora Manager',
    role: 'manager',
    is_active: true,
    avatar_url: null,
    notify_email: true,
    telegram_chat_id: null,
    created_at: new Date().toISOString(),
    last_login: null,
  },
]

const ROLE_BADGE: Record<UserRole, 'purple' | 'info' | 'default'> = {
  admin: 'purple',
  manager: 'info',
  user: 'default',
}

const ROLE_LABEL: Record<UserRole, string> = {
  admin: 'Админ',
  manager: 'Менеджер',
  user: 'Пользователь',
}

export default function Admin() {
  const { user } = useAuth()
  const qc = useQueryClient()
  const isAdmin = user?.role === 'admin'
  const demoMode = localStorage.getItem('demo_admin') === 'true'
  const [demoUsers, setDemoUsers] = useState<User[]>(DEMO_USERS)

  const { data: apiUsers = [], isLoading } = useQuery({
    queryKey: ['users'],
    queryFn: usersApi.list,
    enabled: isAdmin && !demoMode,
  })
  const users = demoMode ? demoUsers : apiUsers

  const createUser = useMutation({
    mutationFn: usersApi.create,
    onSuccess: () => qc.invalidateQueries({ queryKey: ['users'] }),
  })

  const updateUser = useMutation({
    mutationFn: ({ id, role }: { id: number; role: UserRole }) =>
      usersApi.update(id, { role }),
    onSuccess: () => qc.invalidateQueries({ queryKey: ['users'] }),
  })

  const deactivateUser = useMutation({
    mutationFn: usersApi.deactivate,
    onSuccess: () => qc.invalidateQueries({ queryKey: ['users'] }),
  })

  const { register, handleSubmit, reset, formState: { errors } } = useForm<FormData>({
    resolver: zodResolver(schema),
    defaultValues: { role: 'user' },
  })

  const onSubmit = async (data: FormData) => {
    if (demoMode) {
      setDemoUsers(current => [
        ...current,
        {
          id: Math.max(...current.map(member => member.id)) + 1,
          company_id: 1,
          email: data.email,
          full_name: data.full_name,
          role: data.role,
          is_active: true,
          avatar_url: null,
          notify_email: true,
          telegram_chat_id: null,
          created_at: new Date().toISOString(),
          last_login: null,
        },
      ])
      reset({ role: 'user', email: '', password: '', full_name: '' })
      return
    }

    await createUser.mutateAsync(data)
    reset({ role: 'user', email: '', password: '', full_name: '' })
  }

  const handleRoleChange = (id: number, role: UserRole) => {
    if (demoMode) {
      setDemoUsers(current => current.map(member =>
        member.id === id ? { ...member, role } : member
      ))
      return
    }

    updateUser.mutate({ id, role })
  }

  const handleDeactivate = (id: number) => {
    if (demoMode) {
      setDemoUsers(current => current.map(member =>
        member.id === id ? { ...member, is_active: false } : member
      ))
      return
    }

    deactivateUser.mutate(id)
  }

  if (!isAdmin) {
    return (
      <div className="p-6">
        <EmptyState
          icon={<ShieldCheck size={22} />}
          title="Нужны права администратора"
          description="Эта страница доступна только пользователям с ролью admin."
        />
      </div>
    )
  }

  return (
    <div className="p-6 space-y-5">
      <div className="flex items-center justify-between gap-3 flex-wrap">
        <div>
          <h2 className="text-slate-100 font-semibold text-xl">Админка</h2>
          <p className="text-slate-500 text-sm mt-0.5">Пользователи, роли и доступ команды</p>
        </div>
        <Badge variant="purple" className="gap-1">
          <Crown size={12} />
          {user?.email}
        </Badge>
      </div>

      <div className="grid grid-cols-1 xl:grid-cols-[360px_1fr] gap-5">
        <Card padding="md">
          <div className="flex items-center gap-2 mb-4">
            <UserPlus size={16} className="text-indigo-400" />
            <h3 className="text-slate-200 font-medium">Создать пользователя</h3>
          </div>
          <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
            <Input label="Email" type="email" error={errors.email?.message} {...register('email')} />
            <Input label="Пароль" type="password" error={errors.password?.message} {...register('password')} />
            <Input label="Имя" error={errors.full_name?.message} {...register('full_name')} />
            <div className="flex flex-col gap-1">
              <label className="text-sm text-slate-300 font-medium">Роль</label>
              <select
                className="bg-slate-900 border border-slate-700 rounded-lg px-3 py-2 text-sm text-slate-200 focus:outline-none focus:ring-1 focus:ring-indigo-500"
                {...register('role')}
              >
                <option value="user">Пользователь</option>
                <option value="manager">Менеджер</option>
                <option value="admin">Админ</option>
              </select>
            </div>
            <Button type="submit" className="w-full" isLoading={createUser.isPending}>
              Добавить
            </Button>
          </form>
        </Card>

        <Card padding="none">
          {isLoading ? (
            <div className="flex justify-center py-12">
              <div className="w-6 h-6 border-2 border-indigo-600 border-t-transparent rounded-full animate-spin" />
            </div>
          ) : users.length === 0 ? (
            <EmptyState
              icon={<UserPlus size={20} />}
              title="Пользователи не найдены"
              description="Создайте первого пользователя команды."
            />
          ) : (
            <div className="overflow-x-auto">
              <table className="w-full">
                <thead>
                  <tr className="border-b border-slate-800">
                    {['Пользователь', 'Роль', 'Статус', 'Создан', ''].map(h => (
                      <th key={h} className="text-left px-5 py-3 text-xs font-medium text-slate-500 uppercase tracking-wide">
                        {h}
                      </th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  {users.map((member: User) => (
                    <tr key={member.id} className="border-b border-slate-800/50 hover:bg-slate-800/20 transition-colors">
                      <td className="px-5 py-3.5">
                        <div className="flex items-center gap-3">
                          <div className="w-8 h-8 rounded-full bg-indigo-600/20 flex items-center justify-center">
                            <span className="text-indigo-300 text-xs font-semibold">
                              {(member.full_name ?? member.email).charAt(0).toUpperCase()}
                            </span>
                          </div>
                          <div className="min-w-0">
                            <p className="text-sm font-medium text-slate-200 truncate">
                              {member.full_name ?? member.email}
                            </p>
                            <p className="text-xs text-slate-500 flex items-center gap-1">
                              <Mail size={11} />
                              {member.email}
                            </p>
                          </div>
                        </div>
                      </td>
                      <td className="px-5 py-3.5">
                        <select
                          value={member.role}
                          disabled={member.id === user?.id || updateUser.isPending}
                          onChange={e => handleRoleChange(member.id, e.target.value as UserRole)}
                          className="bg-slate-900 border border-slate-700 rounded-lg px-3 py-1.5 text-xs text-slate-300 focus:outline-none disabled:opacity-60"
                        >
                          <option value="user">Пользователь</option>
                          <option value="manager">Менеджер</option>
                          <option value="admin">Админ</option>
                        </select>
                        <Badge variant={ROLE_BADGE[member.role]} className="ml-2">
                          {ROLE_LABEL[member.role]}
                        </Badge>
                      </td>
                      <td className="px-5 py-3.5">
                        <Badge variant={member.is_active ? 'success' : 'default'}>
                          {member.is_active ? 'Активен' : 'Отключен'}
                        </Badge>
                      </td>
                      <td className="px-5 py-3.5 text-xs text-slate-500">
                        {format(new Date(member.created_at), 'dd.MM.yyyy')}
                      </td>
                      <td className="px-5 py-3.5">
                        <button
                          disabled={member.id === user?.id || deactivateUser.isPending || !member.is_active}
                          onClick={() => handleDeactivate(member.id)}
                          className="text-slate-600 hover:text-red-400 transition-colors disabled:opacity-40 disabled:hover:text-slate-600"
                          title="Отключить пользователя"
                        >
                          <Trash2 size={14} />
                        </button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </Card>
      </div>
    </div>
  )
}
// Project version: Nexora CRM V2.3
