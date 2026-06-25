#pragma once

#include <string>
#include <thread>

namespace crm::notifications {

class TelegramBot {
public:
    static TelegramBot& instance();
    bool init(const std::string& bot_token);
    bool send_message(const std::string& chat_id, const std::string& text);
    bool send_async(const std::string& chat_id, const std::string& text);

private:
    std::string bot_token_;
};

} // namespace crm::notifications
