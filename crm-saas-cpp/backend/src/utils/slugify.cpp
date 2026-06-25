#include "utils/slugify.hpp"
#include <algorithm>
#include <cctype>

namespace crm::utils {

std::string slugify(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    bool last_was_dash = false;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            result += static_cast<char>(std::tolower(c));
            last_was_dash = false;
        } else if (!last_was_dash && !result.empty()) {
            result += '-';
            last_was_dash = true;
        }
    }

    // Убираем trailing dash
    while (!result.empty() && result.back() == '-')
        result.pop_back();

    // Обрезаем до 100 символов
    if (result.size() > 100)
        result.resize(100);

    return result;
}

} // namespace crm::utils
