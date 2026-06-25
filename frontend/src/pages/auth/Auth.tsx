import { useState } from 'react'
import { Link } from 'react-router-dom'
import { useForm } from 'react-hook-form'
import { zodResolver } from '@hookform/resolvers/zod'
import { z } from 'zod'
import { Building2 } from 'lucide-react'
import { useAuth } from '../../hooks/useAuth'
import { Button, Input } from '../../components/ui'

// ─────────────────────────────────────────────
// LOGIN
// ─────────────────────────────────────────────

const loginSchema = z.object({
  email:    z.string().email('Неверный email'),
  password: z.string().min(1, 'Введите пароль'),
})
type LoginForm = z.infer<typeof loginSchema>

export function LoginPage() {
  const { login } = useAuth()
  const [serverError, setServerError] = useState('')

  const { register, handleSubmit, formState: { errors, isSubmitting } } = useForm<LoginForm>({
    resolver: zodResolver(loginSchema),
  })

  const onSubmit = async (data: LoginForm) => {
    try {
      setServerError('')
      await login(data)
    } catch (e: any) {
      setServerError(e?.response?.data?.detail ?? 'Ошибка входа')
    }
  }

  return (
    <AuthLayout title="Войти в аккаунт">
      <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
        <Input
          label="Email"
          type="email"
          autoComplete="email"
          error={errors.email?.message}
          {...register('email')}
        />
        <Input
          label="Пароль"
          type="password"
          autoComplete="current-password"
          error={errors.password?.message}
          {...register('password')}
        />

        {serverError && (
          <p className="text-sm text-red-400 bg-red-950/40 border border-red-900/50 rounded-lg px-3 py-2">
            {serverError}
          </p>
        )}

        <Button type="submit" className="w-full mt-2" isLoading={isSubmitting}>
          Войти
        </Button>

        <p className="text-center text-sm text-slate-500">
          Нет аккаунта?{' '}
          <Link to="/register" className="text-indigo-400 hover:text-indigo-300 transition-colors">
            Зарегистрироваться
          </Link>
        </p>
      </form>
    </AuthLayout>
  )
}

// ─────────────────────────────────────────────
// REGISTER
// ─────────────────────────────────────────────

const registerSchema = z.object({
  full_name:    z.string().min(2, 'Минимум 2 символа'),
  company_name: z.string().min(2, 'Минимум 2 символа'),
  email:        z.string().email('Неверный email'),
  password:     z.string().min(8, 'Минимум 8 символов'),
})
type RegisterForm = z.infer<typeof registerSchema>

export function RegisterPage() {
  const { register: registerUser } = useAuth()
  const [serverError, setServerError] = useState('')

  const { register, handleSubmit, formState: { errors, isSubmitting } } = useForm<RegisterForm>({
    resolver: zodResolver(registerSchema),
  })

  const onSubmit = async (data: RegisterForm) => {
    try {
      setServerError('')
      await registerUser(data)
    } catch (e: any) {
      setServerError(e?.response?.data?.detail ?? 'Ошибка регистрации')
    }
  }

  return (
    <AuthLayout title="Создать аккаунт">
      <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
        <Input
          label="Ваше имя"
          error={errors.full_name?.message}
          {...register('full_name')}
        />
        <Input
          label="Название компании"
          hint="Будет использовано для создания вашего рабочего пространства"
          error={errors.company_name?.message}
          {...register('company_name')}
        />
        <Input
          label="Email"
          type="email"
          error={errors.email?.message}
          {...register('email')}
        />
        <Input
          label="Пароль"
          type="password"
          hint="Минимум 8 символов"
          error={errors.password?.message}
          {...register('password')}
        />

        {serverError && (
          <p className="text-sm text-red-400 bg-red-950/40 border border-red-900/50 rounded-lg px-3 py-2">
            {serverError}
          </p>
        )}

        <Button type="submit" className="w-full mt-2" isLoading={isSubmitting}>
          Создать аккаунт
        </Button>

        <p className="text-center text-sm text-slate-500">
          Уже есть аккаунт?{' '}
          <Link to="/login" className="text-indigo-400 hover:text-indigo-300 transition-colors">
            Войти
          </Link>
        </p>
      </form>
    </AuthLayout>
  )
}

// ─────────────────────────────────────────────
// Общий лейаут для auth страниц
// ─────────────────────────────────────────────

function AuthLayout({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <div className="min-h-screen bg-[#0A0E1A] flex items-center justify-center p-4">
      <div className="w-full max-w-sm">
        {/* Логотип */}
        <div className="flex items-center justify-center gap-2 mb-8">
          <div className="w-9 h-9 rounded-xl bg-indigo-600 flex items-center justify-center">
            <Building2 size={18} className="text-white" />
          </div>
          <span className="text-white font-semibold text-lg tracking-tight">CRM SaaS</span>
        </div>

        {/* Карточка */}
        <div className="bg-[#161B22] border border-slate-800 rounded-2xl p-6 shadow-xl">
          <h1 className="text-slate-100 font-semibold text-lg mb-5">{title}</h1>
          {children}
        </div>
      </div>
    </div>
  )
}
