#pragma once

#include <map>
#include <mutex>
#include <set>
#include <crow.h>
#include <nlohmann/json.hpp>

namespace crm::notifications {

class WebSocketManager {
public:
    static void add_connection(int company_id, crow::websocket::connection* conn);
    static void remove_connection(int company_id, crow::websocket::connection* conn);
    static void broadcast(int company_id, const nlohmann::json& msg);

private:
    static std::map<int, std::set<crow::websocket::connection*>> connections;
    static std::mutex connections_mutex;
};

} // namespace crm::notifications
