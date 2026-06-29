# Nexora CRM V2.3 Deployment Notes

This public version is prepared for local/demo hosting, not production operation.

## Build

```bash
cd frontend
pnpm install
pnpm run build
```

## Backend

```bash
cd backend
cmake -S . -B build
cmake --build build
```

## Docker

```bash
cp .env.example .env
docker compose up --build
```

## Demo Hosting

- Set `DEMO_MODE=true` and `VITE_DEMO_MODE=true`.
- Set `ALLOWED_ORIGINS` to the hosted frontend origin.
- Replace `JWT_SECRET` for any private deployment.
- Do not publish private CRM records or databases.
<!-- Project version: Nexora CRM V2.3 -->
