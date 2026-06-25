#pragma once

#include <string>

namespace crm::email {

struct EmailMessage {
    std::string to;
    std::string subject;
    std::string body;
};

class SmtpClient {
public:
    static SmtpClient& instance();
    bool init(const std::string& host, int port, const std::string& user, const std::string& password);
    bool send(const EmailMessage& msg);
    bool send_async(const EmailMessage& msg);

private:
    std::string smtp_host_;
    int smtp_port_{};
    std::string smtp_user_;
    std::string smtp_password_;
};

} // namespace crm::email
