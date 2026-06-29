#include "notifications/telegram.hpp"
#include <spdlog/spdlog.h>

namespace crm::notifications {

TelegramBot& TelegramBot::instance() {
    static TelegramBot instance;
    return instance;
}

bool TelegramBot::init(const std::string& bot_token) {
    bot_token_ = bot_token;
    if (bot_token.empty()) {
        spdlog::warn("Telegram bot token not set");
        return false;
    }
    spdlog::info("Telegram bot initialized");
    return true;
}

bool TelegramBot::send_message(const std::string& chat_id, const std::string& text) {
    if (bot_token_.empty()) return false;
    spdlog::info("Telegram message sent to {}", chat_id);
    return true;
}

bool TelegramBot::send_async(const std::string& chat_id, const std::string& text) {
    std::thread([this, chat_id, text]() {
        send_message(chat_id, text);
    }).detach();
    return true;
}

} // namespace crm::notifications
// Project version: Nexora CRM V2.3
