import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import { clientsApi } from '../api/client'
import type { ClientCreate, ClientUpdate } from '../types'

const QUERY_KEY = 'clients'

export function useClients(params?: {
  search?: string
  is_active?: boolean
  tag?: string
}) {
  return useQuery({
    queryKey: [QUERY_KEY, params],
    queryFn: () => clientsApi.list(params),
    staleTime: 30_000, // 30 секунд кэша
  })
}

export function useClient(id: number) {
  return useQuery({
    queryKey: [QUERY_KEY, id],
    queryFn: () => clientsApi.get(id),
    enabled: !!id,
  })
}

export function useCreateClient() {
  const qc = useQueryClient()
  return useMutation({
    mutationFn: (data: ClientCreate) => clientsApi.create(data),
    onSuccess: () => qc.invalidateQueries({ queryKey: [QUERY_KEY] }),
  })
}

export function useUpdateClient() {
  const qc = useQueryClient()
  return useMutation({
    mutationFn: ({ id, data }: { id: number; data: ClientUpdate }) =>
      clientsApi.update(id, data),
    onSuccess: () => qc.invalidateQueries({ queryKey: [QUERY_KEY] }),
  })
}

export function useDeleteClient() {
  const qc = useQueryClient()
  return useMutation({
    mutationFn: (id: number) => clientsApi.delete(id),
    onSuccess: () => qc.invalidateQueries({ queryKey: [QUERY_KEY] }),
  })
}

export function useClientInteractions(clientId: number) {
  return useQuery({
    queryKey: [QUERY_KEY, clientId, 'interactions'],
    queryFn: () => clientsApi.interactions(clientId),
    enabled: !!clientId,
  })
}

export function useAddInteraction(clientId: number) {
  const qc = useQueryClient()
  return useMutation({
    mutationFn: (data: { type: string; content: string }) =>
      clientsApi.addInteraction(clientId, data),
    onSuccess: () =>
      qc.invalidateQueries({ queryKey: [QUERY_KEY, clientId, 'interactions'] }),
  })
}
