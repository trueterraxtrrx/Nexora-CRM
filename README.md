# CRM SaaS — C++ Backend

<div align="center">

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue?logo=cplusplus)
![Crow](https://img.shields.io/badge/Crow-1.2.0-black)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-16-336791?logo=postgresql)
![React](https://img.shields.io/badge/React-18-61DAFB?logo=react)
![Docker](https://img.shields.io/badge/Docker-Compose-2496ED?logo=docker)
![CI](https://github.com/yourname/crm-saas-cpp/actions/workflows/ci.yml/badge.svg)

**Мультитенантная CRM система с C++ бэкендом и React фронтендом**

</div>

---

## Архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                        Client Browser                        │
└──────────────────────────┬──────────────────────────────────┘
                           │ HTTP
┌──────────────────────────▼──────────────────────────────────┐
│                     Nginx (port 80)                          │
│          SPA static files + /api/* reverse proxy             │
└────────────┬─────────────────────────────┬──────────────────┘
             │                             │
┌────────────▼──────────┐      ┌───────────▼────────────────┐
│   React 18 + TS        │      │   C++ Backend (Crow)        │
│   Tailwind CSS         │      │   port 8000                 │
│   React Query          │      │   HS256 JWT Auth            │
│   Recharts             │      │   Connection Pool           │
└───────────────────────┘      │   Thread Pool               │
                               └───────────┬────────────────┘
                                           │ libpqxx
                               ┌───────────▼────────────────┐
                               │     PostgreSQL 16            │
                               │     Multi-tenant schema      │
                               │     Full ACID transactions   │
                               └────────────────────────────┘
```

## Стек

| Слой         | Технология                            | Зачем                                        |
|--------------|---------------------------------------|----------------------------------------------|
| HTTP Server  | [Crow 1.2](https://crowcpp.org)       | Lightweight, zero-dep, header-only C++ Sinatra|
| Database     | libpqxx 7 + PostgreSQL 16             | type-safe C++ клиент, пул соединений         |
| Auth         | JWT HS256 (OpenSSL HMAC-SHA256)       | Stateless, без сторонних JWT либ             |
| Passwords    | bcrypt (libbcrypt, cost=12)           | Стойкий к brute-force                        |
| JSON         | nlohmann/json (header-only)           | Самая популярная JSON либа для C++           |
| Logging      | spdlog + fmt                          | Structured logging, цветной вывод            |
| Build        | CMake 3.20 + Ninja                    | Быстрая инкрементальная сборка               |
| Frontend     | React 18 + TypeScript + Tailwind      | Полноценный SPA                              |
| Infra        | Docker + Docker Compose + Nginx       | Одна команда — весь стек запущен             |
| CI/CD        | GitHub Actions                        | Сборка, тесты, пуш в GHCR                   |

## Возможности

- **Мультитенантность** — каждая компания полностью изолирована на уровне `company_id`
- **Планы подписки** — Free / Pro / Enterprise с лимитами на пользователей и клиентов
- **Роли** — Admin, Manager, User с проверкой прав на уровне каждого endpoint
- **Журнал аудита** — каждое действие логируется в `audit_logs` (JSONB детали)
- **CRUD** — Клиенты, Задачи (канбан), Финансы с отчётами
- **История взаимодействий** — звонки, встречи, письма, заметки
- **Connection Pool** — thread-safe пул соединений к БД
- **Миграции** — встроенные SQL миграции при старте
- **Health check** — `/api/health` для мониторинга

## Быстрый старт

### Docker (рекомендуется)

```bash
git clone https://github.com/yourname/crm-saas-cpp
cd crm-saas-cpp

# Создай .env из примера
cp backend/.env.example backend/.env
# Обязательно поменяй JWT_SECRET в .env!

docker-compose up --build
```

- Фронтенд: **http://localhost**
- API: **http://localhost:8000**
- Health: **http://localhost:8000/api/health**

### Локальная сборка C++ бэкенда

**Ubuntu/Debian:**
```bash
sudo apt-get install -y \
  build-essential cmake ninja-build git \
  libssl-dev libpq-dev libpqxx-dev libboost-all-dev \
  libfmt-dev libspdlog-dev libbcrypt-dev

# Crow (header-only)
git clone --depth 1 --branch v1.2.0 https://github.com/CrowCpp/Crow.git /tmp/crow
cd /tmp/crow && cmake -B build -DCROW_BUILD_EXAMPLES=OFF && sudo cmake --install build

# nlohmann/json (header-only)
git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git /tmp/json
cd /tmp/json && cmake -B build -DJSON_BuildTests=OFF && sudo cmake --install build

# Сборка проекта
cd crm-saas-cpp/backend
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja
cmake --build build --parallel $(nproc)

# Запуск
cp .env.example .env  # заполни DB_* и JWT_SECRET
export $(cat .env | xargs)
./build/crm_backend
```

**macOS:**
```bash
brew install cmake ninja openssl libpq libpqxx fmt spdlog boost
# Crow и nlohmann/json — аналогично через git + cmake install
```

### Тесты

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja
cmake --build build
cd build && ctest --output-on-failure
```

## API

### Auth
| Метод  | URL                      | Описание                          |
|--------|--------------------------|-----------------------------------|
| POST   | `/api/v1/auth/register`  | Регистрация + создание компании   |
| POST   | `/api/v1/auth/login`     | Логин → JWT токен                 |
| GET    | `/api/v1/auth/me`        | Текущий пользователь              |

### Clients
| Метод  | URL                                      | Роли         |
|--------|------------------------------------------|--------------|
| GET    | `/api/v1/clients?search=&is_active=`     | all          |
| POST   | `/api/v1/clients`                        | all          |
| PATCH  | `/api/v1/clients/:id`                    | all          |
| DELETE | `/api/v1/clients/:id`                    | admin/manager|
| GET    | `/api/v1/clients/:id/interactions`       | all          |
| POST   | `/api/v1/clients/:id/interactions`       | all          |

### Tasks
| Метод  | URL                                      | Описание                |
|--------|------------------------------------------|-------------------------|
| GET    | `/api/v1/tasks?status=&priority=&overdue=` | Список с фильтрами    |
| POST   | `/api/v1/tasks`                          | Создать задачу          |
| PATCH  | `/api/v1/tasks/:id`                      | Обновить/сменить статус |
| DELETE | `/api/v1/tasks/:id`                      | Удалить задачу          |

### Finance
| Метод  | URL                                      | Описание          |
|--------|------------------------------------------|-------------------|
| GET    | `/api/v1/finance?type=income`            | Список записей    |
| POST   | `/api/v1/finance`                        | Добавить запись   |
| DELETE | `/api/v1/finance/:id`                    | Удалить запись    |
| GET    | `/api/v1/finance/report?year=2025`       | Финансовый отчёт  |

### Users (admin only)
| Метод  | URL                    | Описание               |
|--------|------------------------|------------------------|
| GET    | `/api/v1/users`        | Список пользователей   |
| POST   | `/api/v1/users`        | Пригласить (admin)     |
| PATCH  | `/api/v1/users/:id`    | Обновить профиль       |
| DELETE | `/api/v1/users/:id`    | Деактивировать (admin) |

## Структура C++ бэкенда

```
backend/
├── CMakeLists.txt          # Сборка всего проекта
├── include/
│   ├── core/
│   │   ├── config.hpp      # Typed конфиг из env vars
│   │   ├── database.hpp    # Thread-safe connection pool
│   │   ├── jwt.hpp         # HS256 JWT без сторонних либ
│   │   ├── password.hpp    # bcrypt hash/verify
│   │   └── middleware.hpp  # Auth + CORS middleware для Crow
│   ├── api/
│   │   ├── auth_handler.hpp
│   │   ├── clients_handler.hpp
│   │   ├── tasks_handler.hpp
│   │   ├── finance_handler.hpp
│   │   └── users_handler.hpp
│   └── utils/
│       ├── response.hpp    # JSON response helpers
│       ├── audit.hpp       # Audit log writer
│       └── slugify.hpp     # Company slug generator
├── src/                    # Реализации (.cpp)
└── tests/
    ├── test_jwt.cpp        # Unit тесты JWT
    ├── test_password.cpp   # Unit тесты bcrypt
    └── test_slugify.cpp    # Unit тесты slugify
```

## Почему C++ для SaaS?

| Метрика        | Python/FastAPI | C++/Crow    |
|----------------|---------------|-------------|
| Latency (p99)  | ~15ms         | ~2ms        |
| Throughput     | ~3k req/s     | ~50k req/s  |
| RAM (idle)     | ~80MB         | ~8MB        |
| Binary size    | N/A           | ~5MB        |
| CPU per req    | высокий       | минимальный |

На реальных нагрузках C++ бэкенд обрабатывает в 10-15x больше запросов на той же железке.

## Roadmap

- [ ] WebSocket для real-time уведомлений
- [ ] Email уведомления (SMTP async)
- [ ] Telegram Bot уведомления
- [ ] Redis кэш для горячих запросов
- [ ] Prometheus метрики (`/metrics`)
- [ ] gRPC сервис для inter-service
- [ ] Rate limiting (per company)
- [ ] Export в CSV/Excel

## Лицензия

MIT
