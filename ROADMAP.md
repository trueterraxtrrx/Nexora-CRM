# Nexora CRM V1.0 Roadmap

Business CRM dashboard for clients, tasks and finance workflows in the KRYNEX Labs ecosystem.

## Already implemented

- C++ Crow backend with PostgreSQL persistence, JWT auth and modular route handlers.
- React/Vite dashboard for clients, tasks, finance and demo-mode preview.
- C++ CSV export and import parser with quote escaping and malformed-row validation.
- C++ required-header validation for safe client, task and finance import workflows.
- C++ finance import amount summarizer for fast CSV preview totals.
- Windows/MSVC build fixes, OpenSSL PBKDF2 password hashing and JWT base64url padding fix.
- Production guardrails plus CTest coverage for password, JWT and utility behavior.

## Will be implemented

- Richer public demo CRM seed data and read-only hosted demo controls.
- Pipeline and deal board views for sales workflow review.
- Import/export endpoint wiring, finance preview totals and lightweight audit/settings screens.
- End-to-end smoke tests for auth, clients, tasks and finance.

<!-- Project version: Nexora CRM V1.0 -->



