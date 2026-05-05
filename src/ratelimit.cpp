#include "ratelimit.h"
#include <algorithm>
#include <iostream>

RateLimiter& RateLimiter::instance() {
    static RateLimiter rl;
    return rl;
}

std::string RateLimiter::makeKey(const std::string& clientId, const std::string& endpoint) const {
    std::string key = clientId;
    key += "::";
    key += endpoint;
    return key;
}

bool RateLimiter::isAllowed(const std::string& clientId, const std::string& endpoint, int limit, int window_seconds) {
    std::lock_guard<std::mutex> lock(_mutex);

    std::string key = makeKey(clientId, endpoint);

    auto now = std::chrono::high_resolution_clock::now();
    auto it = _limits.find(key);

    if (it == _limits.end()) {
        ClientLimit cl;
        cl.requestCount = 1;
        cl.windowStart = now;

        _limits.insert({key, cl}); 
        
        //std::cout << "юзер добавлен: " << clientId << std::endl;
        return true;
    }

    ClientLimit& data = it->second;

    auto diff = now - data.windowStart;
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    long long seconds = sec.count();

    // окно закончилось
    if (seconds >= window_seconds) {
        data.requestCount = 0; 
        data.requestCount = data.requestCount + 1; 
        data.windowStart = now;

        //std::cout << "сбросили окно для юзера " << clientId << std::endl;
        return true;
    }

    if (data.requestCount < limit) {
        data.requestCount = data.requestCount + 1; 
        return true;
    }

    return false;
}


int RateLimiter::getRemainingRequests(const std::string& clientId, const std::string& endpoint, int limit, int window_seconds) {
    std::lock_guard<std::mutex> lock(_mutex);

    std::string key = makeKey(clientId, endpoint);

    auto now = std::chrono::high_resolution_clock::now();

    auto it = _limits.find(key);

    // если юзера нет — значит всё доступно
    if (it == _limits.end()) {
        return limit;
    }

    ClientLimit& data = it->second;

    auto diff = now - data.windowStart;
    long long seconds = std::chrono::duration_cast<std::chrono::seconds>(diff).count();

    // если окно уже прошло
    if (seconds >= window_seconds) {
        return limit; 
    }

    int remaining = limit - data.requestCount;

    if (remaining < 0) {
        return 0;
    } else {
        return remaining;
    }
}


long long RateLimiter::getResetTime(const std::string& clientId, const std::string& endpoint, int window_seconds) {
    std::lock_guard<std::mutex> lock(_mutex);

    std::string key = makeKey(clientId, endpoint);

    auto now = std::chrono::high_resolution_clock::now();

    auto it = _limits.find(key);

    if (it == _limits.end()) {
        auto t = std::chrono::high_resolution_clock::to_time_t(now);
        return t + window_seconds;
    }

    ClientLimit& data = it->second;

    auto resetPoint = data.windowStart + std::chrono::seconds(window_seconds);

    long long result = std::chrono::high_resolution_clock::to_time_t(resetPoint);

    return result;
}


void RateLimiter::reset(const std::string& clientId, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(_mutex);

    std::string key = makeKey(clientId, endpoint);

    auto it = _limits.find(key);

    if (it != _limits.end()) {
        _limits.erase(it);
        //std::cout << "сброс для " << clientId << std::endl;
    } else {

    }
}