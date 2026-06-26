import axios, { AxiosError } from 'axios'
import type {
  AuthTokens, LoginCredentials, RegisterData,
  User, UserCreate, UserUpdate,
  Client, ClientCreate, ClientUpdate, ClientInteraction,
  Task, TaskCreate, TaskUpdate,
  Finance, FinanceCreate, FinanceReport,
  DashboardStats, AuditLog,
} from '../types'

// ─────────────────────────────────────────────
// Базовый клиент
// ─────────────────────────────────────────────

export const api = axios.create({
  baseURL: import.meta.env.VITE_API_URL || 'http://localhost:8000',
  headers: { 'Content-Type': 'application/json' },
})
const DEMO_MODE = import.meta.env.VITE_DEMO_MODE === 'true'
const now = new Date().toISOString()
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
  created_at: now,
  last_login: now,
}
let demoClients: Client[] = [
  { id: 1, company_id: 1, name: 'Acme Security', email: 'ops@acme.example', phone: '+1 555 0101', company_name: 'Acme Corp', notes: 'Enterprise security pilot', tags: '["enterprise","security"]', is_active: true, created_at: now, updated_at: now },
  { id: 2, company_id: 1, name: 'Northwind Finance', email: 'it@northwind.example', phone: '+1 555 0102', company_name: 'Northwind', notes: 'CRM demo account', tags: '["finance"]', is_active: true, created_at: now, updated_at: now },
]
let demoTasks: Task[] = [
  { id: 1, company_id: 1, title: 'Prepare Q3 renewal brief', description: 'Demo portfolio task', status: 'in_progress', priority: 'high', deadline: now, client_id: 1, assignee_id: 1, created_at: now, updated_at: now, completed_at: null },
  { id: 2, company_id: 1, title: 'Review onboarding checklist', description: 'Demo portfolio task', status: 'todo', priority: 'medium', deadline: now, client_id: 2, assignee_id: 1, created_at: now, updated_at: now, completed_at: null },
]
let demoFinance: Finance[] = [
  { id: 1, company_id: 1, type: 'income', amount: 12400, currency: 'USD', description: 'Demo subscription revenue', category: 'Subscriptions', date: now, client_id: 1, created_at: now },
  { id: 2, company_id: 1, type: 'expense', amount: 3200, currency: 'USD', description: 'Demo infrastructure spend', category: 'Infrastructure', date: now, client_id: null, created_at: now },
]
const demoInteractions: ClientInteraction[] = [
  { id: 1, client_id: 1, user_id: 1, type: 'meeting', content: 'Security roadmap review completed.', created_at: now },
  { id: 2, client_id: 1, user_id: 1, type: 'note', content: 'Follow up with hosted demo access.', created_at: now },
]

function demoReport(): FinanceReport {
  return {
    total_income: 12400,
    total_expense: 3200,
    profit: 9200,
    by_category: {
      Subscriptions: { income: 12400, expense: 0 },
      Infrastructure: { income: 0, expense: 3200 },
    },
    by_month: Array.from({ length: 12 }, (_, i) => ({
      month: i + 1,
      income: i < 6 ? 8000 + i * 900 : 12400,
      expense: i < 6 ? 2200 + i * 180 : 3200,
    })),
  }
}

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
      if (!DEMO_MODE) window.location.href = '/login'
    }
    return Promise.reject(error)
  }
)

// ─────────────────────────────────────────────
// AUTH
// ─────────────────────────────────────────────

export const authApi = {
  login: async (credentials: LoginCredentials): Promise<AuthTokens> => {
    if (DEMO_MODE) return { access_token: 'demo-token', token_type: 'bearer' }
    // FastAPI OAuth2 ожидает form-data, не JSON
    const form = new FormData()
    form.append('username', credentials.email)
    form.append('password', credentials.password)
    const { data } = await api.post<AuthTokens>('/api/v1/auth/login', form, {
      headers: { 'Content-Type': 'multipart/form-data' },
    })
    return data
  },

  register: async (payload: RegisterData): Promise<User> => {
    if (DEMO_MODE) return { ...demoUser, email: payload.email, full_name: payload.full_name }
    const { data } = await api.post<User>('/api/v1/auth/register', payload)
    return data
  },

  me: async (): Promise<User> => {
    if (DEMO_MODE) return demoUser
    const { data } = await api.get<User>('/api/v1/auth/me')
    return data
  },
}

// ─────────────────────────────────────────────
// USERS
// ─────────────────────────────────────────────

export const usersApi = {
  list: async (): Promise<User[]> => {
    if (DEMO_MODE) return [demoUser]
    const { data } = await api.get<User[]>('/api/v1/users/')
    return data
  },

  create: async (payload: UserCreate): Promise<User> => {
    if (DEMO_MODE) return { ...demoUser, id: Date.now(), ...payload }
    const { data } = await api.post<User>('/api/v1/users/', payload)
    return data
  },

  update: async (id: number, payload: UserUpdate): Promise<User> => {
    if (DEMO_MODE) return { ...demoUser, id, ...payload }
    const { data } = await api.patch<User>(`/api/v1/users/${id}`, payload)
    return data
  },

  deactivate: async (id: number): Promise<void> => {
    if (DEMO_MODE) return
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
    if (DEMO_MODE) return demoClients.filter((client) => params?.is_active === undefined || client.is_active === params.is_active)
    const { data } = await api.get<Client[]>('/api/v1/clients/', { params })
    return data
  },

  get: async (id: number): Promise<Client> => {
    if (DEMO_MODE) return demoClients.find((client) => client.id === id) ?? demoClients[0]
    const { data } = await api.get<Client>(`/api/v1/clients/${id}`)
    return data
  },

  create: async (payload: ClientCreate): Promise<Client> => {
    if (DEMO_MODE) {
      const created = { id: Date.now(), company_id: 1, is_active: true, created_at: now, updated_at: now, email: null, phone: null, company_name: null, notes: null, tags: null, ...payload } as Client
      demoClients = [created, ...demoClients]
      return created
    }
    const { data } = await api.post<Client>('/api/v1/clients/', payload)
    return data
  },

  update: async (id: number, payload: ClientUpdate): Promise<Client> => {
    if (DEMO_MODE) {
      demoClients = demoClients.map((client) => client.id === id ? { ...client, ...payload, updated_at: now } : client)
      return demoClients.find((client) => client.id === id) ?? demoClients[0]
    }
    const { data } = await api.patch<Client>(`/api/v1/clients/${id}`, payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    if (DEMO_MODE) {
      demoClients = demoClients.filter((client) => client.id !== id)
      return
    }
    await api.delete(`/api/v1/clients/${id}`)
  },

  interactions: async (clientId: number): Promise<ClientInteraction[]> => {
    if (DEMO_MODE) return demoInteractions.filter((item) => item.client_id === clientId)
    const { data } = await api.get<ClientInteraction[]>(
      `/api/v1/clients/${clientId}/interactions`
    )
    return data
  },

  addInteraction: async (
    clientId: number,
    payload: { type: string; content: string }
  ): Promise<ClientInteraction> => {
    if (DEMO_MODE) return { id: Date.now(), client_id: clientId, user_id: 1, created_at: now, ...payload } as ClientInteraction
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
    if (DEMO_MODE) return demoTasks
    const { data } = await api.get<Task[]>('/api/v1/tasks/', { params })
    return data
  },

  get: async (id: number): Promise<Task> => {
    if (DEMO_MODE) return demoTasks.find((task) => task.id === id) ?? demoTasks[0]
    const { data } = await api.get<Task>(`/api/v1/tasks/${id}`)
    return data
  },

  create: async (payload: TaskCreate): Promise<Task> => {
    if (DEMO_MODE) {
      const created = { id: Date.now(), company_id: 1, description: null, status: 'todo', priority: 'medium', deadline: null, client_id: null, assignee_id: 1, created_at: now, updated_at: now, completed_at: null, ...payload } as Task
      demoTasks = [created, ...demoTasks]
      return created
    }
    const { data } = await api.post<Task>('/api/v1/tasks/', payload)
    return data
  },

  update: async (id: number, payload: TaskUpdate): Promise<Task> => {
    if (DEMO_MODE) {
      demoTasks = demoTasks.map((task) => task.id === id ? { ...task, ...payload, updated_at: now } : task)
      return demoTasks.find((task) => task.id === id) ?? demoTasks[0]
    }
    const { data } = await api.patch<Task>(`/api/v1/tasks/${id}`, payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    if (DEMO_MODE) {
      demoTasks = demoTasks.filter((task) => task.id !== id)
      return
    }
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
    if (DEMO_MODE) return demoFinance.filter((row) => !params?.type || row.type === params.type)
    const { data } = await api.get<Finance[]>('/api/v1/finance/', { params })
    return data
  },

  create: async (payload: FinanceCreate): Promise<Finance> => {
    if (DEMO_MODE) {
      const created = { id: Date.now(), company_id: 1, currency: 'USD', description: null, category: null, date: now, client_id: null, created_at: now, ...payload } as Finance
      demoFinance = [created, ...demoFinance]
      return created
    }
    const { data } = await api.post<Finance>('/api/v1/finance/', payload)
    return data
  },

  delete: async (id: number): Promise<void> => {
    if (DEMO_MODE) {
      demoFinance = demoFinance.filter((row) => row.id !== id)
      return
    }
    await api.delete(`/api/v1/finance/${id}`)
  },

  report: async (params: { year: number; month?: number }): Promise<FinanceReport> => {
    if (DEMO_MODE) return demoReport()
    const { data } = await api.get<FinanceReport>('/api/v1/finance/report', { params })
    return data
  },
}
