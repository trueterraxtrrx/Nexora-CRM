import axios, { AxiosError } from 'axios'
import type {
  AuthTokens, LoginCredentials, RegisterData,
  User, UserCreate, UserUpdate,
  Client, ClientCreate, ClientUpdate, ClientInteraction,
  Task, TaskCreate, TaskUpdate,
  Finance, FinanceCreate, FinanceReport,
} from '../types'

// ─────────────────────────────────────────────
// Базовый клиент
// ─────────────────────────────────────────────

export const api = axios.create({
  baseURL: import.meta.env.VITE_API_URL || 'http://localhost:8000',
  headers: { 'Content-Type': 'application/json' },
})

// Автоматически добавляем токен в каждый запрос
api.interceptors.request.use((config) => {
  const token = localStorage.getItem('access_token')
  if (token) {
    config.headers.Authorization = `Bearer ${token}`
  }
  return config
})

// Обрабатываем 401 — разлогиниваем
api.interceptors.response.use(
  (response) => response,
  (error: AxiosError) => {
    if (error.response?.status === 401) {
      localStorage.removeItem('access_token')
      window.location.href = '/login'
    }
    return Promise.reject(error)
  }
)

// ─────────────────────────────────────────────
// AUTH
// ─────────────────────────────────────────────

export const authApi = {
  login: async (credentials: LoginCredentials): Promise<AuthTokens> => {
    const { data } = await api.post<AuthTokens>('/api/v1/auth/login', credentials)
    return data
  },

  register: async (payload: RegisterData): Promise<User> => {
    const { data } = await api.post<User>('/api/v1/auth/register', payload)
    return data
  },

  me: async (): Promise<User> => {
    const { data } = await api.get<User>('/api/v1/auth/me')
    return data
  },
}

// ─────────────────────────────────────────────
// USERS
// ─────────────────────────────────────────────

export const usersApi = {
  list: async (): Promise<User[]> => {
    const { data } = await api.get<User[]>('/api/v1/users')
    return data
  },

  create: async (payload: UserCreate): Promise<User> => {
    const { data } = await api.post<User>('/api/v1/users', payload)
    return data
  },

  update: async (id: number, payload: UserUpdate): Promise<User> => {
    const { data } = await api.patch<User>(`/api/v1/users/${id}`, payload)
    return data
  },

  deactivate: async (id: number): Promise<void> => {
    await api.delete(`/api/v1/users/${id}`)
  },
}

// ─────────────────────────────────────────────
// CLIENTS
// ─────────────────────────────────────────────

export const clientsApi = {
  list: async (params?: {
    search?: string
    is_active?: boolean
    tag?: string
    skip?: number
    limit?: number
  }): Promise<Client[]> => {
    const { data } = await api.get<Client[]>('/api/v1/clients', { params })
    return data
  },

  get: async (id: number): Promise<Client> => {
    const { data } = await api.get<Client>(`/api/v1/clients/${id}`)
    return data
  },

  create: async (payload: ClientCreate): Promise<Client> => {
    const { data } = await api.post<Client>('/api/v1/clients', payload)
    return data
  },

  update: async (id: number, payload: ClientUpdate): Promise<Client> => {
    const { data } = await api.patch<Client>(`/api/v1/clients/${id}`, payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    await api.delete(`/api/v1/clients/${id}`)
  },

  interactions: async (clientId: number): Promise<ClientInteraction[]> => {
    const { data } = await api.get<ClientInteraction[]>(
      `/api/v1/clients/${clientId}/interactions`
    )
    return data
  },

  addInteraction: async (
    clientId: number,
    payload: { type: string; content: string }
  ): Promise<ClientInteraction> => {
    const { data } = await api.post<ClientInteraction>(
      `/api/v1/clients/${clientId}/interactions`,
      payload
    )
    return data
  },
}

// ─────────────────────────────────────────────
// TASKS
// ─────────────────────────────────────────────

export const tasksApi = {
  list: async (params?: {
    status?: string
    priority?: string
    assignee_id?: number
    client_id?: number
    overdue?: boolean
    skip?: number
    limit?: number
  }): Promise<Task[]> => {
    const { data } = await api.get<Task[]>('/api/v1/tasks', { params })
    return data
  },

  get: async (id: number): Promise<Task> => {
    const { data } = await api.get<Task>(`/api/v1/tasks/${id}`)
    return data
  },

  create: async (payload: TaskCreate): Promise<Task> => {
    const { data } = await api.post<Task>('/api/v1/tasks', payload)
    return data
  },

  update: async (id: number, payload: TaskUpdate): Promise<Task> => {
    const { data } = await api.patch<Task>(`/api/v1/tasks/${id}`, payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    await api.delete(`/api/v1/tasks/${id}`)
  },
}

// ─────────────────────────────────────────────
// FINANCE
// ─────────────────────────────────────────────

export const financeApi = {
  list: async (params?: {
    type?: string
    category?: string
    date_from?: string
    date_to?: string
    client_id?: number
  }): Promise<Finance[]> => {
    const { data } = await api.get<Finance[]>('/api/v1/finance', { params })
    return data
  },

  create: async (payload: FinanceCreate): Promise<Finance> => {
    const { data } = await api.post<Finance>('/api/v1/finance', payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    await api.delete(`/api/v1/finance/${id}`)
  },

  report: async (params: { year: number; month?: number }): Promise<FinanceReport> => {
    const { data } = await api.get<FinanceReport>('/api/v1/finance/report', { params })
    return data
  },
}
