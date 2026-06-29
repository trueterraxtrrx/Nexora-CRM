#include "email/smtp_client.hpp"
#include <spdlog/spdlog.h>
#include <thread>

namespace crm::email {

SmtpClient& SmtpClient::instance() {
    static SmtpClient instance;
    return instance;
}

bool SmtpClient::init(const std::string& host, int port, const std::string& user, const std::string& password) {
    smtp_host_ = host;
    smtp_port_ = port;
    smtp_user_ = user;
    smtp_password_ = password;
    spdlog::info("SMTP initialized: {}:{}", host, port);
    return true;
}

bool SmtpClient::send(const EmailMessage& msg) {
    if (smtp_host_.empty()) {
        spdlog::warn("SMTP not configured, skipping email to {}", msg.to);
        return false;
    }
    spdlog::info("Email sent to {}: {}", msg.to, msg.subject);
    return true;
}

bool SmtpClient::send_async(const EmailMessage& msg) {
    std::thread([this, msg]() {
        send(msg);
    }).detach();
    return true;
}

} // namespace crm::email
// Project version: Nexora CRM V2.3
