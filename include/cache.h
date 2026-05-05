#pragma once

#include <string>
#include <hiredis/hiredis.h>
#include <memory>
#include <chrono>

class RedisCache {
public:
    static RedisCache& instance();

    void initialize(const std::string& host = "redis", int port = 6379);
    std::string get(const std::string& key);
    bool set(const std::string& key, const std::string& value, int ttl_seconds = 3600);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    bool deletePattern(const std::string& pattern);
    redisReply* exec(const std::string& cmd);

    struct Stats {
        long long hits = 0;
        long long misses = 0;
    };

    Stats getStats() const { return _stats; }
    void resetStats() { _stats = Stats(); }

private:
    RedisCache() = default;
    ~RedisCache() = default;

    void* _context = nullptr;  
    Stats _stats;
    bool _initialized = false;
};
