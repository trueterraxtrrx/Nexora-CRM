# Гайд по развёртыванию и разработке CRM SaaS (C++ Backend)

## Содержание

1. [Как работает проект](#как-работает-проект)
2. [Установка зависимостей](#установка-зависимостей)
3. [Запуск через Docker (5 минут)](#запуск-через-docker)
4. [Запуск локально](#запуск-локально)
5. [Переменные окружения](#переменные-окружения)
6. [Структура кода](#структура-кода)
7. [Как добавить новый endpoint](#как-добавить-новый-endpoint)
8. [Тесты](#тесты)
9. [Деплой на VPS](#деплой-на-vps)
10. [Частые ошибки](#частые-ошибки)

---

## Как работает проект

```
Запрос → Nginx → Crow HTTP Server → Middleware (JWT + CORS)
       → Handler → ConnectionPool → PostgreSQL
       → JSON response → клиент
```

**Crow** — это C++ HTTP фреймворк (header-only). Принимает запросы, парсит URL/body,
передаёт в твой обработчик. Аналог Flask/Express, только в 10 раз быстрее.

**Connection Pool** — вместо того чтобы открывать соединение с БД на каждый запрос
(дорого — ~5ms overhead), держим пул из 10 открытых соединений и раздаём потокам.

**JWT** — реализован вручную через OpenSSL HMAC-SHA256. Никаких сторонних JWT либ.
Токен живёт 24 часа, содержит `user_id`, `company_id`, `role`.

**Мультитенантность** — в каждом запросе из токена берём `company_id` и добавляем
его в каждый SQL запрос (`WHERE company_id = $1`). Никакой магии — просто WHERE.

---

## Установка зависимостей

### Ubuntu 24.04 / Debian 12

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \      # gcc, g++, make
  cmake \                # система сборки
  ninja-build \          # быстрый backend для cmake
  git \
  pkg-config \
  libssl-dev \           # OpenSSL (JWT + bcrypt)
  libpq-dev \            # PostgreSQL C клиент (заголовки)
  libpqxx-dev \          # libpqxx — C++ обёртка над libpq
  libboost-all-dev \     # Boost (нужен Crow)
  libfmt-dev \           # fmt — форматирование строк
  libspdlog-dev \        # spdlog — логирование
  libbcrypt-dev          # bcrypt для хэширования паролей
```

### Crow (header-only, устанавливаем вручную)

```bash
git clone --depth 1 --branch v1.2.0 https://github.com/CrowCpp/Crow.git /tmp/crow
cd /tmp/crow
cmake -B build \
  -DCROW_BUILD_EXAMPLES=OFF \
  -DCROW_BUILD_TESTS=OFF
sudo cmake --install build
# Заголовки появятся в /usr/local/include/crow/
```

### nlohmann/json (header-only)

```bash
git clone --depth 1 --branch v3.11.3 \
  https://github.com/nlohmann/json.git /tmp/json
cd /tmp/json
cmake -B build -DJSON_BuildTests=OFF
sudo cmake --install build
# Заголовки в /usr/local/include/nlohmann/
```

### macOS

```bash
brew install cmake ninja openssl libpq libpqxx fmt spdlog boost

# Crow и nlohmann — аналогично через git+cmake (см. выше)

# Если cmake не видит openssl:
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
```

### PostgreSQL (локально)

```bash
# Ubuntu
sudo apt-get install postgresql-16

# Создать БД и пользователя
sudo -u postgres psql << 'SQL'
CREATE USER crm_user WITH PASSWORD 'crm_pass';
CREATE DATABASE crm_db OWNER crm_user;
GRANT ALL PRIVILEGES ON DATABASE crm_db TO crm_user;
SQL
```

---

## Запуск через Docker

Самый простой способ — Docker Compose запустит PostgreSQL, backend и frontend.

```bash
# 1. Клонируй репозиторий
git clone https://github.com/yourname/crm-saas-cpp
cd crm-saas-cpp

# 2. Создай .env (ОБЯЗАТЕЛЬНО поменяй JWT_SECRET!)
cp backend/.env.example backend/.env
nano backend/.env
# Измени: JWT_SECRET=my_super_secret_key_minimum_32_chars

# 3. Запуск
docker-compose up --build

# Первый запуск — долго (компиляция C++ в Docker, ~5-10 минут)
# Последующие — быстро (кэш слоёв)
```

После запуска:
- **Фронтенд**: http://localhost
- **Backend API**: http://localhost:8000
- **Health check**: http://localhost:8000/api/health

```bash
# Запуск в фоне
docker-compose up -d

# Логи backend
docker-compose logs -f backend

# Остановка
docker-compose down

# Остановка с удалением данных БД
docker-compose down -v
```

---

## Запуск локально

### Backend

```bash
cd backend

# 1. Создай .env
cp .env.example .env
# Заполни DB_* если не дефолтные значения, обязательно JWT_SECRET

# 2. Загрузи переменные
export $(grep -v '^#' .env | xargs)

# 3. Сборка (Debug для разработки)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja
cmake --build build --parallel $(nproc)

# 4. Запуск
./build/crm_backend
# → [2025-01-01 12:00:00] [info] Starting CRM Backend on port 8000
# → [2025-01-01 12:00:00] [info] Running DB migrations...
# → [2025-01-01 12:00:00] [info] Migrations completed successfully
```

### Frontend

```bash
cd frontend

# 1. Зависимости
npm install

# 2. .env
echo "VITE_API_URL=http://localhost:8000" > .env

# 3. Dev-сервер
npm run dev
# → http://localhost:5173
```

---

## Переменные окружения

| Переменная         | Обязательная | По умолчанию       | Описание                          |
|--------------------|--------------|--------------------|-----------------------------------|
| `JWT_SECRET`       | **ДА**       | —                  | Секрет для JWT. Мин. 32 символа   |
| `DB_HOST`          | нет          | `localhost`        | Хост PostgreSQL                   |
| `DB_PORT`          | нет          | `5432`             | Порт PostgreSQL                   |
| `DB_NAME`          | нет          | `crm_db`           | Имя базы данных                   |
| `DB_USER`          | нет          | `crm_user`         | Пользователь БД                   |
| `DB_PASSWORD`      | нет          | `crm_pass`         | Пароль БД                         |
| `DB_POOL_SIZE`     | нет          | `10`               | Размер пула соединений            |
| `PORT`             | нет          | `8000`             | Порт HTTP сервера                 |
| `THREADS`          | нет          | кол-во ядер CPU    | Потоков в thread pool             |
| `DEBUG`            | нет          | `false`            | Подробное логирование             |
| `JWT_EXPIRY_HOURS` | нет          | `24`               | Время жизни токена (часы)         |
| `ALLOWED_ORIGINS`  | нет          | `localhost:*`      | CORS origins через запятую        |

---

## Структура кода

```
backend/
├── CMakeLists.txt          ← система сборки
├── include/                ← заголовочные файлы (.hpp)
│   ├── core/
│   │   ├── config.hpp      ← Config::load() из env
│   │   ├── database.hpp    ← ConnectionPool + ScopedConnection
│   │   ├── jwt.hpp         ← JwtService: create_token/verify_token
│   │   ├── password.hpp    ← hash_password/verify_password
│   │   └── middleware.hpp  ← AuthMiddleware, CorsMiddleware, require_role()
│   ├── api/
│   │   └── *_handler.hpp   ← объявления register_*_routes()
│   └── utils/
│       ├── response.hpp    ← json_ok(), json_error(), parse_body()
│       ├── audit.hpp       ← write_audit()
│       └── slugify.hpp     ← slugify()
├── src/                    ← реализации (.cpp)
│   ├── main.cpp            ← точка входа, инициализация, маршруты
│   ├── core/               ← реализации core-модулей
│   ├── api/                ← HTTP обработчики
│   └── utils/              ← утилиты
└── tests/                  ← Google Test unit тесты
```

### Принцип работы handler'а

Каждый handler — это функция `register_*_routes(AppType& app)` которая
через макрос `CROW_ROUTE` регистрирует лямбды на URL паттерны:

```cpp
CROW_ROUTE(app, "/api/v1/clients").methods(crow::HTTPMethod::Get)
([](const crow::request& req, crow::response& res, AppType::context& ctx) {
    // 1. Проверяем авторизацию
    if (!require_auth(req, res, ctx)) return;
    auto& claims = *ctx.get<AuthMiddleware>().claims;

    // 2. Транзакция к БД
    res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
        auto rows = txn.exec_params(
            "SELECT * FROM clients WHERE company_id=$1",
            claims.company_id  // ← мультитенантность
        );
        // 3. Собираем JSON ответ
        json result = json::array();
        for (const auto& row : rows) result.push_back(...);
        return json_ok(result);
    });
});
```

---

## Как добавить новый endpoint

Пример: добавляем `GET /api/v1/stats` — статистика дашборда.

### 1. Создай заголовочный файл

```cpp
// include/api/stats_handler.hpp
#pragma once
#include "core/middleware.hpp"
namespace crm::api {
    void register_stats_routes(crm::core::AppType& app);
}
```

### 2. Создай реализацию

```cpp
// src/api/stats_handler.cpp
#include "api/stats_handler.hpp"
#include "core/database.hpp"
#include "core/middleware.hpp"
#include "utils/response.hpp"
#include <nlohmann/json.hpp>

namespace crm::api {
using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

void register_stats_routes(AppType& app) {
    CROW_ROUTE(app, "/api/v1/stats").methods(crow::HTTPMethod::Get)
    ([](const crow::request& req, crow::response& res, AppType::context& ctx) {
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.get<AuthMiddleware>().claims;

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto clients = txn.exec_params1(
                    "SELECT COUNT(*) FROM clients WHERE company_id=$1 AND is_active=TRUE",
                    claims.company_id
                );
                auto tasks = txn.exec_params1(
                    "SELECT COUNT(*) FROM tasks WHERE company_id=$1 AND status NOT IN ('done','cancelled')",
                    claims.company_id
                );
                return json_ok({
                    {"total_clients", clients[0].as<int>()},
                    {"active_tasks",  tasks[0].as<int>()},
                });
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });
}
} // namespace crm::api
```

### 3. Добавь в CMakeLists.txt

```cmake
set(SOURCES
    ...
    src/api/stats_handler.cpp   # ← добавь сюда
)
```

### 4. Подключи в main.cpp

```cpp
#include "api/stats_handler.hpp"
// ...
crm::api::register_stats_routes(app);  // ← добавь после других register_*
```

### 5. Пересобери

```bash
cmake --build build --parallel $(nproc)
```

---

## Тесты

Тесты написаны с Google Test и покрывают core-логику (без зависимостей от БД):

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja
cmake --build build

# Запуск всех тестов
cd build && ctest --output-on-failure

# Или напрямую
./tests/crm_tests
```

**Что тестируется:**
- `test_jwt.cpp` — создание токена, верификация, tamper detection, expired
- `test_password.cpp` — bcrypt hash/verify, разные соли, пустой пароль
- `test_slugify.cpp` — конвертация строк в slug, спецсимволы, длина

**Добавить тест:**
```cpp
// tests/test_new_feature.cpp
#include <gtest/gtest.h>
#include "utils/my_util.hpp"

TEST(MyUtilTest, BasicCase) {
    EXPECT_EQ(my_function("input"), "expected_output");
}
```
Добавь файл в `tests/CMakeLists.txt` → `target_sources(crm_tests PRIVATE test_new_feature.cpp)`.

---

## Деплой на VPS

### Минимальные требования
- Ubuntu 24.04 VPS, 1GB RAM (для сборки рекомендуется 2GB)
- Docker + Docker Compose установлены

```bash
# 1. Установи Docker
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
newgrp docker

# 2. Клонируй проект
git clone https://github.com/yourname/crm-saas-cpp
cd crm-saas-cpp

# 3. Настрой переменные
cp backend/.env.example backend/.env
nano backend/.env
# ОБЯЗАТЕЛЬНО: JWT_SECRET=<случайная строка 32+ символов>
# ОБЯЗАТЕЛЬНО: DB_PASSWORD=<сложный пароль>
# Также обнови DB_PASSWORD в docker-compose.yml

# 4. Запуск
docker-compose up -d --build

# 5. Проверка
curl http://localhost:8000/api/health
# → {"status":"ok","version":"1.0.0"}
```

### Nginx как reverse proxy (если нужен HTTPS)

```nginx
# /etc/nginx/sites-available/crm
server {
    server_name crm.yourdomain.com;

    location / {
        proxy_pass http://localhost:80;
    }

    location /api/ {
        proxy_pass http://localhost:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

```bash
sudo certbot --nginx -d crm.yourdomain.com
```

### Автоматическое обновление

```bash
# pull + rebuild + restart (zero-downtime с --no-deps)
git pull
docker-compose up -d --build --no-deps backend
docker-compose up -d --build --no-deps frontend
```

---

## Частые ошибки

### `libpqxx not found` при cmake
```bash
sudo apt-get install libpqxx-dev
# или укажи путь вручную:
cmake -B build -DCMAKE_PREFIX_PATH=/usr/local
```

### `CROW not found` при cmake
```bash
# Убедись что установка Crow прошла успешно
ls /usr/local/include/crow/  # должны быть файлы
# Если нет — повтори установку с sudo
```

### `connection refused` к PostgreSQL
```bash
# Проверь что postgres запущен
sudo systemctl status postgresql
# Проверь что DB_HOST=localhost (не db) при локальном запуске
# db — это hostname внутри docker-compose сети
```

### JWT_SECRET не задан
```bash
# При старте увидишь:
# [error] Required env var 'JWT_SECRET' is not set
# Реши:
export JWT_SECRET="your_secret_key_minimum_32_chars_here"
# или добавь в .env и загрузи через: export $(cat .env | xargs)
```

### Сборка падает с OOM в Docker
```bash
# Docker по умолчанию даёт мало памяти компилятору C++
# Увеличь лимит Docker Desktop до 4GB
# Или собирай с меньшим параллелизмом:
cmake --build build --parallel 1
```

### `pqxx::unexpected_rows` в логах
```bash
# exec_params1() ожидает ровно 1 строку — если 0 или 2+, бросает исключение
# Используй exec_params() если количество строк неизвестно
auto rows = txn.exec_params("SELECT ...");
if (rows.empty()) return json_error(404, "Not found");
```
