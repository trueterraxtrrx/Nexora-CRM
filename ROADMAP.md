# Nexora 3.0 Roadmap

Nexora 3.0 should move the product from a working CRM core to a polished operating system for sales, tasks, finance, and team control.

## Product Goals

- Make daily work faster: fewer clicks, clearer priorities, stronger filters.
- Give managers a live picture of revenue, workload, overdue tasks, and team risk.
- Prepare the platform for integrations, automations, and AI-assisted workflows.
- Keep the C++ backend fast, observable, and easy to deploy.

## 3.0 Milestones

### 1. Command Center

- Unified dashboard with sales pipeline, task load, finance forecast, and alerts.
- Saved dashboard views for owner, manager, and operator roles.
- Quick actions for creating clients, tasks, finance records, and follow-ups.

### 2. Smart CRM

- Client timeline with calls, meetings, notes, tasks, finance links, and file events.
- Lead scoring based on activity, overdue tasks, and revenue potential.
- Tags, segments, and saved filters for clients.

### 3. Workflow Automation

- Rules such as "new VIP client creates onboarding task".
- SLA alerts for stale clients and overdue high-priority tasks.
- Email and Telegram templates for common events.

### 4. Finance 3.0

- Cashflow forecast by month and category.
- Budget limits with warnings.
- Export presets for accounting and manager reports.

### 5. Team And Access

- Activity audit viewer in the frontend.
- Granular permissions beyond admin/manager/user.
- User invitation flow and onboarding checklist.

### 6. Integrations

- Webhooks for clients, tasks, and finance events.
- Public API keys with scoped access.
- Calendar integration for task deadlines and meetings.

### 7. Reliability

- Expanded backend test coverage for all API handlers.
- End-to-end smoke tests for auth, clients, tasks, and finance.
- Production deployment checklist with backup and restore flow.

## Already Started In V2.2

- Task filters for priority and overdue work.
- Finance run-rate forecast and margin indicators.
- CI checks for frontend build and backend Docker build.

## Public Demo Track

- Keep demo mode isolated from production auth and persistence assumptions.
- Expand safe CRM seed data for clients, tasks, finance, and dashboard views.
- Add read-only hosted demo controls before enabling public review links.
- Document the demo-to-production boundary in README and deployment notes.

## Suggested V3.0 Feature Order

1. Client timeline and tags.
2. Dashboard command center.
3. Automation rules.
4. Audit viewer and permissions.
5. Webhooks and API keys.
6. E2E test pack and production checklist.
<!-- Project version: Nexora CRM V2.3 -->
