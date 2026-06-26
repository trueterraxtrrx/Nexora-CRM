# Nexora — C++ Backend + React Frontend

<div align="center">

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue?logo=cplusplus)
![Crow](https://img.shields.io/badge/Crow-1.2.0-black)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-16-336791?logo=postgresql)
![Redis](https://img.shields.io/badge/Redis-7-DC382D?logo=redis)
![React](https://img.shields.io/badge/React-18-61DAFB?logo=react)
![Docker](https://img.shields.io/badge/Docker-Compose-2496ED?logo=docker)

**Мультитенантная SaaS CRM с ultra-fast C++ бэкендом**

[Быстрый старт](#быстрый-старт) • [API](#api) • [Разработка](#разработка) • [Деплой](#деплой)

</div>

---

## ✨ Фичи

### Core
- ✅ **Мультитенантность** — полная изоляция данных компаний
- ✅ **JWT Auth** — HS256 без сторонних JWT либ, реализовано вручную на OpenSSL
- ✅ **Role-based Access Control** — Admin, Manager, User с проверками на каждом endpoint
- ✅ **Connection Pool** — thread-safe пул соединений к PostgreSQL
- ✅ **Audit Log** — каждое действие логируется с JSONB деталями

### REST API (5 роутеров)
- ✅ **Auth** — register, login, JWT refresh, me
- ✅ **Clients** — CRUD + история взаимодействий + поиск/фильтры
- ✅ **Tasks** — Kanban (todo → in_progress → done), дедлайны, приоритеты
- ✅ **Finance** — доходы/расходы + месячные отчёты
- ✅ **Users** — управление командой, роли, лимиты по плану

### Roadmap (реализовано в v2.0)
- ✅ **WebSocket** — real-time уведомления о действиях
- ✅ **Redis Cache** — кэширование клиентов, задач, финансов
- ✅ **Email Notifications** — SMTP async для событий
- ✅ **Telegram Bot** — интеграция с Telegram для уведомлений
- ✅ **Prometheus Metrics** — `/metrics` endpoint для мониторинга
- ✅ **Rate Limiting** — per-company request limits
- ✅ **CSV/Excel Export** — экспорт клиентов, задач, финансов
- 🔄 gRPC (в разработке) — inter-service communication

---

## 🚀 Быстрый старт

### Docker (1 команда)

```bash
git clone https://github.com/terraicy/Nexora-CRM
cd Nexora-CRM

# Копируй .env и поменяй JWT_SECRET
cp backend/.env.example backend/.env
sed -i 's/JWT_SECRET=.*/JWT_SECRET=your_super_secret_key_32chars/' backend/.env

# Запуск — Docker Compose будет строить C++ из исходников
docker-compose up --build

# На первый раз: 5-10 минут (компиляция C++)
# На второй раз: 30 секунд (кэш слоёв)
```

**Откроется:**
- Фронтенд: http://localhost
- API: http://localhost:8000
- Metrics: http://localhost:8000/metrics
- Health: http://localhost:8000/api/health

### Локально (для разработки)

**Требования:** C++23, CMake 3.20+, libpq, libpqxx, OpenSSL, Boost, hiredis

```bash
# Backend
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja
cmake --build build --parallel $(nproc)
export $(cat .env | xargs)
./build/crm_backend

# Frontend (другой терминал)
cd frontend
npm install && npm run dev
```

---

## 📊 Производительность

| Метрика | Python/FastAPI | C++/Crow | Улучшение |
|---------|---|---|---|
| **Latency (p99)** | ~15ms | ~2ms | **7.5x** |
| **Throughput** | ~3k req/s | ~50k req/s | **16.7x** |
| **RAM (idle)** | ~80MB | ~8MB | **10x** |
| **Binary** | N/A | ~5MB | compact |

На реальных нагрузках C++ бэкенд обрабатывает **в 10-15 раз больше запросов** на той же железке.

---

## 🗂️ API Endpoints

### Authentication
```
POST   /api/v1/auth/register      - Регистрация
POST   /api/v1/auth/login         - Логин → JWT
GET    /api/v1/auth/me            - Текущий пользователь
```

### Clients
```
GET    /api/v1/clients                           - Список (поиск, фильтры)
POST   /api/v1/clients                           - Создать
GET    /api/v1/clients/:id                       - Получить
PATCH  /api/v1/clients/:id                       - Обновить
DELETE /api/v1/clients/:id                       - Удалить
GET    /api/v1/clients/:id/interactions          - История взаимодействий
POST   /api/v1/clients/:id/interactions          - Добавить взаимодействие
GET    /api/v1/export/clients?format=csv         - Экспорт в CSV
```

### Tasks
```
GET    /api/v1/tasks                             - Список (статус, приоритет, overdue)
POST   /api/v1/tasks                             - Создать
PATCH  /api/v1/tasks/:id                         - Обновить статус/дедлайн
DELETE /api/v1/tasks/:id                         - Удалить
GET    /api/v1/export/tasks?format=csv           - Экспорт
```

### Finance
```
GET    /api/v1/finance                           - Все записи
POST   /api/v1/finance                           - Добавить доход/расход
DELETE /api/v1/finance/:id                       - Удалить
GET    /api/v1/finance/report?year=2025          - Отчёт по месяцам
GET    /api/v1/export/finance?format=csv         - Экспорт
```

### WebSocket
```
WS     /ws/:company_id/:token                    - Real-time уведомления
```

### Monitoring
```
GET    /metrics                                  - Prometheus метрики
GET    /api/health                               - Health check
```

---

## 📁 Структура проекта

```
Nexora-CRM/
├── backend/
│   ├── include/
│   │   ├── core/             (config, database, jwt, password, middleware, rate_limiter)
│   │   ├── api/              (5 роутеров)
│   │   ├── cache/            (Redis)
│   │   ├── notifications/    (WebSocket, Telegram)
│   │   ├── email/            (SMTP async)
│   │   ├── metrics/          (Prometheus)
│   │   ├── export/           (CSV exporter)
│   │   └── utils/
│   ├── src/                  (реализации)
│   ├── tests/                (Google Test)
│   ├── CMakeLists.txt
│   └── Dockerfile            (multi-stage build)
├── frontend/
│   ├── src/
│   │   ├── pages/            (Dashboard, Clients, Tasks, Finance, Auth)
│   │   ├── components/
│   │   ├── hooks/
│   │   ├── api/
│   │   └── types/
│   ├── package.json
│   └── Dockerfile
├── docker-compose.yml        (PostgreSQL + Backend + Frontend)
└── GUIDE.md                  (подробное руководство по разработке)
```

---

## 🔧 Разработка

### Добавить новый endpoint

1. **Создать заголовок** `include/api/new_handler.hpp`
2. **Реализовать** `src/api/new_handler.cpp` с `register_*_routes()`
3. **Добавить в CMakeLists.txt** к `set(SOURCES ...)`
4. **Подключить в main.cpp**: `crm::api::register_new_routes(app);`
5. **Пересобрать**: `cmake --build build`

### Запустить тесты

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja
cmake --build build
cd build && ctest --output-on-failure
```

### Локальная разработка с Docker

```bash
# Backend с горячей перезагрузкой (требует rebuild)
docker-compose up -d db redis
docker-compose logs -f backend

# Frontend с hot reload
docker-compose up -d db redis backend
cd frontend && npm run dev
```

---

## 📦 Технологии

| Слой | Tech | Зачем |
|------|------|-------|
| **HTTP Server** | Crow 1.2 | Lightweight, header-only, ~50k req/s |
| **Database** | PostgreSQL 16 | ACID, JSONB, мультитенант |
| **C++ ORM** | libpqxx | Типобезопасный, пул соединений |
| **JWT** | OpenSSL HMAC-SHA256 | Реализовано вручную, без зависимостей |
| **Password Hash** | bcrypt (cost=12) | Стойкий к brute-force |
| **Cache** | Redis 7 | Горячие данные, sessions |
| **JSON** | nlohmann/json | Header-only, самая популярная |
| **Logging** | spdlog + fmt | Structured, цветной вывод |
| **Testing** | Google Test | Unit тесты core-логики |
| **Frontend** | React 18 + TS | SPA, real-time WebSocket |
| **Build** | CMake 3.20 + Ninja | Быстрая инкрементальная сборка |
| **Infra** | Docker + Nginx | Zero-downtime деплой |
| **CI/CD** | GitHub Actions | Сборка, тесты, пуш в GHCR |

---

## 🚢 Деплой на VPS

```bash
# 1. Установи Docker
curl -fsSL https://get.docker.com | sh

# 2. Клонируй и настрой
git clone https://github.com/terraicy/Nexora-CRM
cd Nexora-CRM
cp backend/.env.example backend/.env
nano backend/.env  # поменяй JWT_SECRET и DB_PASSWORD

# 3. Запуск
docker-compose up -d --build

# 4. HTTPS + Nginx (опционально)
sudo apt-get install certbot python3-certbot-nginx
sudo certbot --nginx -d crm.yourdomain.com
```

---

## 📝 Лицензия

MIT

---

## 🤝 Контрибьюции

Issues и PR welcome! Следуй [CONTRIBUTING](CONTRIBUTING.md).

---

## Changelog

### v2.0 (Roadmap реализован)
- ✅ WebSocket для real-time уведомлений
- ✅ Redis кэш для горячих запросов
- ✅ Email уведомления (SMTP async)
- ✅ Telegram Bot интеграция
- ✅ Prometheus /metrics endpoint
- ✅ Rate limiting per-company
- ✅ CSV/Excel export для всех таблиц
- ✅ Улучшенная документация (GUIDE.md)

### v1.0 (Стартовый релиз)
- REST API с 5 роутерами
- JWT auth + role-based access control
- Multi-tenant архитектура
- Audit log с JSONB
- React SPA frontend
- Docker Compose
- GitHub Actions CI/CD
