# ── Stage 1: Builder ─────────────────────────────────────────────────────────
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libssl-dev \
    libpq-dev \
    libpqxx-dev \
    libasio-dev \
    libboost-all-dev \
    libfmt-dev \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем Crow (header-only)
RUN git clone --depth 1 --branch v1.2.0 https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cd /tmp/crow && \
    cmake -B build -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    cmake --install build && \
    rm -rf /tmp/crow

# nlohmann/json (header-only)
RUN git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git /tmp/json && \
    cd /tmp/json && \
    cmake -B build -DJSON_BuildTests=OFF && \
    cmake --install build && \
    rm -rf /tmp/json

WORKDIR /app
COPY . .

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -GNinja \
    && cmake --build build --parallel $(nproc)

# ── Stage 2: Runtime (minimal image) ─────────────────────────────────────────
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libssl-dev \
    libpq-dev \
    libpqxx-dev \
    libfmt-dev \
    libspdlog-dev \
    libcrypt1 \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Непривилегированный пользователь
RUN useradd -r -s /bin/false crm

WORKDIR /app
COPY --from=builder /app/build/crm_backend .

USER crm
EXPOSE 8000

HEALTHCHECK --interval=10s --timeout=3s --retries=3 \
    CMD curl -f http://localhost:8000/api/health || exit 1

CMD ["./crm_backend"]
