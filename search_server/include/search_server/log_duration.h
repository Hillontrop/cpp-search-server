#pragma once

#include <chrono>
#include <iostream>


#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

class LogDuration
{
public:
    using Clock = std::chrono::steady_clock;

    explicit LogDuration(const std::string& id) : m_id(id) {}

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto endTime = Clock::now();
        const auto dur = endTime - m_startTime;
        std::cerr << m_id << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string m_id;
    const Clock::time_point m_startTime = Clock::now();
};