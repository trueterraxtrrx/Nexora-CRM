#include "notifications/websocket.hpp"

namespace crm::notifications {

std::map<int, std::set<crow::websocket::connection*>> WebSocketManager::connections;
std::mutex WebSocketManager::connections_mutex;

void WebSocketManager::add_connection(int company_id, crow::websocket::connection* conn) {
    std::lock_guard lock(connections_mutex);
    connections[company_id].insert(conn);
}

void WebSocketManager::remove_connection(int company_id, crow::websocket::connection* conn) {
    std::lock_guard lock(connections_mutex);
    connections[company_id].erase(conn);
}

void WebSocketManager::broadcast(int company_id, const nlohmann::json& msg) {
    std::lock_guard lock(connections_mutex);
    auto it = connections.find(company_id);
    if (it != connections.end()) {
        for (auto* conn : it->second) {
            conn->send_text(msg.dump());
        }
    }
}

} // namespace crm::notifications
// Project version: Nexora CRM V2.3
