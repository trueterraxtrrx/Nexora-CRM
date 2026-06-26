import {
  createContext, useContext, useState, useEffect, useCallback,
  type ReactNode,
} from 'react'
import { useNavigate } from 'react-router-dom'
import { authApi } from '../api/client'
import type { User, LoginCredentials, RegisterData } from '../types'

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

const DEMO_ADMIN: User = {
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
  last_login: null,
}

// ─────────────────────────────────────────────
// Provider
// ─────────────────────────────────────────────

export function AuthProvider({ children }: { children: ReactNode }) {
  const [user, setUser] = useState<User | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const navigate = useNavigate()

  // Восстанавливаем сессию при загрузке
  useEffect(() => {
    if (localStorage.getItem('demo_admin') === 'true') {
      setUser(DEMO_ADMIN)
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

  const enterDemoAdmin = useCallback(() => {
    localStorage.setItem('demo_admin', 'true')
    localStorage.setItem('access_token', 'demo-admin-token')
    setUser(DEMO_ADMIN)
    navigate('/admin')
  }, [navigate])

  const login = useCallback(async (credentials: LoginCredentials) => {
    try {
      const tokens = await authApi.login(credentials)
      localStorage.setItem('access_token', tokens.access_token)
      localStorage.removeItem('demo_admin')
      const me = await authApi.me()
      setUser(me)
      navigate('/dashboard')
    } catch (error) {
      if (
        credentials.email === DEMO_ADMIN.email &&
        credentials.password === 'NexoraAdmin2026!'
      ) {
        enterDemoAdmin()
        return
      }
      throw error
    }
  }, [enterDemoAdmin, navigate])

  const register = useCallback(async (data: RegisterData) => {
    await authApi.register(data)
    // После регистрации — сразу логинимся
    await login({ email: data.email, password: data.password })
  }, [login])

  const logout = useCallback(() => {
    localStorage.removeItem('access_token')
    localStorage.removeItem('demo_admin')
    setUser(null)
    navigate('/login')
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
