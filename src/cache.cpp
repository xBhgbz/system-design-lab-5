#include "cache.h"
#include <hiredis/hiredis.h>
#include <iostream>
#include <sstream>


RedisCache& RedisCache::instance() {
    static RedisCache inst;
    return inst;
}

void RedisCache::initialize(const std::string& host, int port) {
    if (_initialized) {
        return;
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0; 

    redisContext* ctx = redisConnectWithTimeout(host.c_str(), port, tv);

    if (!ctx) {
        return;
    }

    if (ctx->err) {
        redisFree(ctx);
        return;
    }

    _context = ctx; 
    _initialized = true;
}

redisReply* RedisCache::exec(const std::string& cmd) {
    if (!_initialized || !_context) {
        return nullptr;
    }

    redisContext* ctx = (redisContext*)_context;

    redisReply* reply = (redisReply*)redisCommand(ctx, cmd.c_str());

    if (!reply) {
        //std::cout << "Redis error: " << ctx->errstr << std::endl;
    }

    return reply;
}

std::string RedisCache::get(const std::string& key) {
    if (!_initialized) {
        _stats.misses++;
        return "";
    }

    std::stringstream ss;
    ss << "GET " << key;

    redisReply* reply = exec(ss.str());

    if (!reply) {
        _stats.misses++;
        return "";
    }

    std::string value = "";

    if (reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
        _stats.hits++;
    } else {
        _stats.misses++; 
    }

    freeReplyObject(reply);
    return value;
}

bool RedisCache::set(const std::string& key, const std::string& val, int ttl) {
    if (!_initialized) {
        return false;
    }

    std::stringstream ss;

    if (ttl > 0) {
        ss << "SETEX " << key << " " << ttl << " " << val;
    } else {
        ss << "SET " << key << " " << val;
    }

    redisReply* reply = exec(ss.str());

    if (!reply) {
        return false;
    }

    bool ok = false;

    if (reply->type == REDIS_REPLY_STATUS) {
        std::string status = reply->str;

        if (status == "OK") {
            ok = true;
        } else {
            //std::cout << "не OK: " << status << std::endl;
        }
    }

    freeReplyObject(reply);
    return ok;
}

bool RedisCache::del(const std::string& key) {
    if (!_initialized) {
        return false;
    }

    std::string cmd = "DEL " + key;

    redisReply* reply = exec(cmd);

    if (!reply) {
        return false;
    }

    bool res = false;

    if (reply->type == REDIS_REPLY_INTEGER) {
        if (reply->integer > 0) {
            res = true;
        }
    }

    freeReplyObject(reply);
    return res;
}

bool RedisCache::exists(const std::string& key) {
    if (!_initialized) {
        return false;
    }

    redisReply* reply = exec("EXISTS " + key);

    if (!reply) {
        return false;
    }

    bool exists = false;

    if (reply->type == REDIS_REPLY_INTEGER) {
        exists = reply->integer > 0;
    }

    freeReplyObject(reply);
    return exists;
}

bool RedisCache::deletePattern(const std::string& pattern) {
    if (!_initialized) {
        return false;
    }

    redisReply* keys = exec("KEYS " + pattern);

    if (!keys) {
        return false;
    }

    bool ok = true;

    if (keys->type == REDIS_REPLY_ARRAY) {

        int total = keys->elements;

        //std::cout << "найдено ключей: " << total << std::endl;

        for (int i = 0; i < total; i++) {
            std::string k = keys->element[i]->str;

            redisReply* r = exec("DEL " + k);

            if (!r) {
                ok = false;
            } else {
                freeReplyObject(r);
            }
        }
    }

    freeReplyObject(keys);
    return ok;
}