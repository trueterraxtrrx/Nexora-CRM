# Changelog

All notable changes to this project will be documented in this file.

## [2.2.0] - 2026-06-26

### Fixed
- Restored missing backend public headers for API handlers, middleware, core services, cache, notifications, metrics, and export modules.
- Added missing backend source files to CMake and fixed bcrypt/libcrypt linkage.
- Fixed PostgreSQL enum migrations and nullable client field inserts.
- Fixed frontend Vite/TypeScript build issues, auth login payload format, and API paths.
- Fixed Docker build/runtime issues for frontend nginx config and backend healthcheck dependencies.

## [2.0.0] - 2025-01-26

### 🚀 Roadmap Implemented

#### Real-time Features
- ✅ **WebSocket Support** - Real-time notifications for all company users
  - `include/notifications/websocket.hpp`
  - `src/notifications/websocket.cpp`
  - Thread-safe connection manager with broadcast functionality

#### Caching & Performance
- ✅ **Redis Integration** - Cache hot data (clients, tasks, finance)
  - `include/cache/redis_cache.hpp`
  - TTL-based expiration
  - Automatic cache invalidation on UPDATE/DELETE
  - JSON support

#### Notifications
- ✅ **Email Notifications (SMTP)** - Async email sending
  - `include/email/smtp_client.hpp`
  - Non-blocking thread pool
  - Support for HTML templates
  
- ✅ **Telegram Bot** - Message notifications
  - `include/notifications/telegram.hpp`
  - Async sending
  - User chat_id integration

#### Monitoring & Analytics
- ✅ **Prometheus Metrics** - `/metrics` endpoint
  - `include/metrics/prometheus.hpp`
  - Request counters by endpoint
  - Database query tracking
  - Active connections gauge
  - Prometheus format export

- ✅ **Rate Limiting** - Per-company request limits
  - `include/core/rate_limiter.hpp`
  - 1000 req/min default (configurable)
  - Sliding window algorithm
  - HTTP 429 on limit exceeded

#### Data Export
- ✅ **CSV/Excel Export** - Download data in standard formats
  - `include/export/csv_exporter.hpp`
  - Clients, Tasks, Finance tables
  - Proper CSV escaping
  - API endpoints: `/api/v1/export/{clients,tasks,finance}?format=csv`

### 📝 Documentation
- Updated README.md with v2.0 features
- Extended GUIDE.md with usage examples for all new features
- Added .env.production for production deployments
- Troubleshooting section for v2.0 components

### 🔧 Infrastructure
- Updated docker-compose.yml with Redis service
- Added health checks for all services
- Proper networking between containers
- Environment variables for all new features

### 📦 Build System
- Updated CMakeLists.txt with new source files
- Optional hiredis dependency for Redis
- Support for building with/without Redis

---

## [1.0.0] - 2025-01-20

### Initial Release

#### Core Features
- ✅ Multi-tenant architecture
- ✅ JWT authentication (HS256)
- ✅ Role-based access control
- ✅ Connection pooling
- ✅ Audit logging

#### API Endpoints
- ✅ Authentication (register, login, me)
- ✅ Clients (CRUD + interactions + search)
- ✅ Tasks (Kanban, priorities, deadlines)
- ✅ Finance (income/expense tracking)
- ✅ Users (team management)

#### Frontend
- ✅ React 18 + TypeScript
- ✅ Tailwind CSS styling
- ✅ Real-time React Query
- ✅ Recharts visualizations

#### Infrastructure
- ✅ Docker containerization
- ✅ Docker Compose orchestration
- ✅ PostgreSQL database
- ✅ GitHub Actions CI/CD

---

## Roadmap (Future)

- [ ] gRPC service for inter-service communication
- [ ] GraphQL API alongside REST
- [ ] Mobile app (React Native)
- [ ] Advanced reporting (PDF generation)
- [ ] Webhooks for external integrations
- [ ] Two-factor authentication
- [ ] S3 file storage integration
- [ ] Database read replicas for horizontal scaling
