#pragma once

#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <memory>

class RateLimiter {
public:
    static RateLimiter& instance();

    bool isAllowed(const std::string& clientId, const std::string& endpoint, int limit, int window_seconds);
    int getRemainingRequests(const std::string& clientId, const std::string& endpoint, int limit, int window_seconds);
    long long getResetTime(const std::string& clientId, const std::string& endpoint, int window_seconds);
    void reset(const std::string& clientId, const std::string& endpoint);

private:
    RateLimiter() = default;
    ~RateLimiter() = default;

    struct ClientLimit {
        int requestCount = 0;
        std::chrono::high_resolution_clock::time_point windowStart;
    };

    std::map<std::string, ClientLimit> _limits;
    std::mutex _mutex;

    std::string makeKey(const std::string& clientId, const std::string& endpoint) const;
};
