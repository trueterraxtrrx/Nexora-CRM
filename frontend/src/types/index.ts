// ─────────────────────────────────────────────
// ENUMS
// ─────────────────────────────────────────────

export type UserRole = 'admin' | 'manager' | 'user'
export type SubscriptionPlan = 'free' | 'pro' | 'enterprise'
export type TaskStatus = 'todo' | 'in_progress' | 'done' | 'cancelled'
export type TaskPriority = 'low' | 'medium' | 'high' | 'urgent'
export type FinanceType = 'income' | 'expense'
export type InteractionType = 'call' | 'email' | 'meeting' | 'note'

// ─────────────────────────────────────────────
// AUTH
// ─────────────────────────────────────────────

export interface AuthTokens {
  access_token: string
  token_type: string
}

export interface LoginCredentials {
  email: string
  password: string
}

export interface RegisterData {
  email: string
  password: string
  full_name: string
  company_name: string
}

// ─────────────────────────────────────────────
// USER
// ─────────────────────────────────────────────

export interface User {
  id: number
  company_id: number
  email: string
  full_name: string | null
  role: UserRole
  is_active: boolean
  avatar_url: string | null
  notify_email: boolean
  telegram_chat_id: string | null
  created_at: string
  last_login: string | null
}

export interface UserCreate {
  email: string
  password: string
  full_name: string
  role: UserRole
}

export interface UserUpdate {
  full_name?: string
  role?: UserRole
  notify_email?: boolean
  telegram_chat_id?: string
}

// ─────────────────────────────────────────────
// COMPANY
// ─────────────────────────────────────────────

export interface Company {
  id: number
  name: string
  slug: string
  plan: SubscriptionPlan
  max_users: number
  max_clients: number
  created_at: string
}

// ─────────────────────────────────────────────
// CLIENT
// ─────────────────────────────────────────────

export interface Client {
  id: number
  company_id: number
  name: string
  email: string | null
  phone: string | null
  company_name: string | null
  notes: string | null
  tags: string | null  // JSON-строка: '["vip","partner"]'
  is_active: boolean
  created_at: string
  updated_at: string
}

export interface ClientCreate {
  name: string
  email?: string
  phone?: string
  company_name?: string
  notes?: string
  tags?: string
}

export interface ClientUpdate extends Partial<ClientCreate> {
  is_active?: boolean
}

export interface ClientInteraction {
  id: number
  client_id: number
  user_id: number
  type: InteractionType
  content: string
  created_at: string
}

// ─────────────────────────────────────────────
// TASK
// ─────────────────────────────────────────────

export interface Task {
  id: number
  company_id: number
  title: string
  description: string | null
  status: TaskStatus
  priority: TaskPriority
  deadline: string | null
  client_id: number | null
  assignee_id: number | null
  created_at: string
  updated_at: string
  completed_at: string | null
}

export interface TaskCreate {
  title: string
  description?: string
  status?: TaskStatus
  priority?: TaskPriority
  deadline?: string
  client_id?: number
  assignee_id?: number
}

export type TaskUpdate = Partial<TaskCreate>

// ─────────────────────────────────────────────
// FINANCE
// ─────────────────────────────────────────────

export interface Finance {
  id: number
  company_id: number
  type: FinanceType
  amount: number
  currency: string
  description: string | null
  category: string | null
  date: string
  client_id: number | null
  created_at: string
}

export interface FinanceCreate {
  type: FinanceType
  amount: number
  currency?: string
  description?: string
  category?: string
  date?: string
  client_id?: number
}

export interface FinanceReport {
  total_income: number
  total_expense: number
  profit: number
  by_category: Record<string, { income: number; expense: number }>
  by_month: Array<{ month: number; income: number; expense: number }>
}

// ─────────────────────────────────────────────
// AUDIT LOG
// ─────────────────────────────────────────────

export interface AuditLog {
  id: number
  user_id: number | null
  action: string
  entity: string | null
  entity_id: number | null
  details: string | null
  created_at: string
}

// ─────────────────────────────────────────────
// DASHBOARD
// ─────────────────────────────────────────────

export interface DashboardStats {
  total_clients: number
  active_tasks: number
  completed_tasks: number
  monthly_income: number
  monthly_expense: number
  recent_activities: AuditLog[]
}

// ─────────────────────────────────────────────
// API
// ─────────────────────────────────────────────

export interface ApiError {
  detail: string
}

export interface PaginatedParams {
  skip?: number
  limit?: number
}
