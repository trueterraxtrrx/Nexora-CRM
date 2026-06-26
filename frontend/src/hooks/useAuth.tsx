import {
  createContext, useContext, useState, useEffect, useCallback,
  type ReactNode,
} from 'react'
import { useNavigate } from 'react-router-dom'
import { authApi } from '../api/client'
import type { User, LoginCredentials, RegisterData } from '../types'

const DEMO_MODE = import.meta.env.VITE_DEMO_MODE === 'true'
const demoUser: User = {
  id: 1,
  company_id: 1,
  email: 'demo@krynex.local',
  full_name: 'KRYNEX Demo',
  role: 'admin',
  is_active: true,
  avatar_url: null,
  notify_email: true,
  telegram_chat_id: null,
  created_at: new Date(0).toISOString(),
  last_login: null,
}

// ─────────────────────────────────────────────
// Context
// ─────────────────────────────────────────────

interface AuthContextValue {
  user: User | null
  isLoading: boolean
  isAuthenticated: boolean
  login: (credentials: LoginCredentials) => Promise<void>
  register: (data: RegisterData) => Promise<void>
  enterDemoAdmin: () => void
  logout: () => void
}

const AuthContext = createContext<AuthContextValue | null>(null)

// ─────────────────────────────────────────────
// Provider
// ─────────────────────────────────────────────

export function AuthProvider({ children }: { children: ReactNode }) {
  const [user, setUser] = useState<User | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const navigate = useNavigate()

  // Восстанавливаем сессию при загрузке
  useEffect(() => {
    if (DEMO_MODE) {
      localStorage.setItem('access_token', 'demo-token')
      setUser(demoUser)
      setIsLoading(false)
      return
    }
    const token = localStorage.getItem('access_token')
    if (!token) {
      setIsLoading(false)
      return
    }

    authApi.me()
      .then(setUser)
      .catch(() => localStorage.removeItem('access_token'))
      .finally(() => setIsLoading(false))
  }, [])

  const login = useCallback(async (credentials: LoginCredentials) => {
    if (DEMO_MODE) {
      localStorage.setItem('access_token', 'demo-token')
      setUser({ ...demoUser, email: credentials.email || demoUser.email })
      navigate('/dashboard')
      return
    }
    const tokens = await authApi.login(credentials)
    localStorage.setItem('access_token', tokens.access_token)
    const me = await authApi.me()
    setUser(me)
    navigate('/dashboard')
  }, [navigate])

  const register = useCallback(async (data: RegisterData) => {
    await authApi.register(data)
    // После регистрации — сразу логинимся
    await login({ email: data.email, password: data.password })
  }, [login])

  const enterDemoAdmin = useCallback(() => {
    localStorage.setItem('access_token', 'demo-token')
    setUser(demoUser)
    navigate('/dashboard')
  }, [navigate])

  const logout = useCallback(() => {
    localStorage.removeItem('access_token')
    setUser(DEMO_MODE ? demoUser : null)
    navigate(DEMO_MODE ? '/dashboard' : '/login')
  }, [navigate])

  return (
    <AuthContext.Provider
      value={{
        user,
        isLoading,
        isAuthenticated: !!user,
        login,
        register,
        enterDemoAdmin,
        logout,
      }}
    >
      {children}
    </AuthContext.Provider>
  )
}

// ─────────────────────────────────────────────
// Hook
// ─────────────────────────────────────────────

export function useAuth(): AuthContextValue {
  const ctx = useContext(AuthContext)
  if (!ctx) throw new Error('useAuth must be used inside AuthProvider')
  return ctx
}
