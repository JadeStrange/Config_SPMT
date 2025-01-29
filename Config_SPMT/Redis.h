#ifndef REDIS_H
#define REDIS_H

#include <cstdlib>
#include <hiredis/hiredis.h>
// #include <hiredis/async.h>
// #include <hiredis/adapters/libevent.h>
#include <functional>
#include <thread>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

class Redis
{
    public:
        // static void configure(std::string ip, int port);
        // static Redis *instance();
        Redis(std::string ip, int port);
        ~Redis();
        template<typename T>
        void set(std::string key, T value) ;//throw(ExecuteFail);
        /*! 
        *  same function as the "setex" command in redis-cli teminal.
        */
        template<typename T>
        void setex(std::string key, T value, int sec) ;//throw(ExecuteFail);
        template<typename T>
        void get(std::string key, T &value) ;//throw(ExecuteFail, ReplyTypeError);
        /*! 
        *  @see void get(std::string key, T &value)
        */
        void get(std::string key, std::string &value) ;//throw(ExecuteFail, ReplyTypeError);

        /*! 
        *  @see void get(std::string key, T &value)
        */
        void get(std::string key, int &value);
        /*! 
        *  @see void get(std::string key, T &value)
        */
        void get(std::string key, std::vector<std::string> &value);

        /*!
        *  set value of hash in redis server
        */
        void hmset(std::string key, std::map<std::string, std::string> &vals);
        void hmset(std::string key, std::map<std::string, int> &vals);

        /*!
        *  modify or set value of hash in redis server
        */
        template<typename T>
        void hset(std::string key, std::string hkey, T value);
        
        /*!
        *  get all keys(as array) of a hash
        */
        void hkeys(std::string key, std::vector<std::string> &ret);

        /*!
        *  get value of hash with specific key(hkey)
        *  \param key key of hash in redis
        *  \param hkey key in hash
        *  \param[out] value return value
        */
        template<typename T>
        void hget(std::string key, std::string hkey, T &hvalue) ;//throw(ExecuteFail, ReplyTypeError);

        /*!
        *  get all keys/values of hash
        *  \param key key of hash in redis
        *  \param hkey key in hash
        *  \param[out] ret return keys/values of the hash
        */
        template<typename T>
        void hgetall(std::string key, std::map<std::string, T> &ret) ;//throw(ExecuteFail, ReplyTypeError);


    private:
        static Redis *m_self;
        
        void connect();

        redisContext *m_context;

        std::string m_ip;
        int m_port;

        boost::mutex m_execMutex;
};

template<class T>
void Redis::get(std::string key, T &value) //throw(ExecuteFail, ReplyTypeError)
{
    const char *argv[2];
    size_t argvlen[2];

    argv[0] = "GET";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 2, argv, argvlen);
    if(!reply) {
        std::cout<<"GET Template Value Error"<<std::endl;
    }
 
    if(reply->type == REDIS_REPLY_STRING) {
        std::stringstream ss(reply->str);
        ss >> value;
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "GET reply type error: not string";
    }
}

template<typename T>
void Redis::set(std::string key, T value) //throw(ExecuteFail)
{
    const char *argv[3];
    size_t argvlen[3];

    argv[0] = "SET";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    std::stringstream ss;
    ss << value;
    std::string value_str = ss.str();
    argv[2] = value_str.c_str();
    argvlen[2] = value_str.length();

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 3, argv, argvlen);
    if(!reply) {
        std::cout<<"SET Template Value Error"<<std::endl;
    }

    freeReplyObject(reply);
}

template<typename T>
void Redis::setex(std::string key, T value, int sec) //throw(ExecuteFail)
{
    const char *argv[4];
    size_t argvlen[4];
    std::stringstream ss;

    argv[0] = "SETEX";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    std::string secStr;
    ss.str("");
    ss << sec;
    secStr = ss.str();
    argv[2] = secStr.c_str();
    argvlen[2] = secStr.length();

    std::string valueStr;
    ss.str("");
    ss << value;
    valueStr = ss.str();
    argv[3] = valueStr.c_str();
    argvlen[3] = valueStr.length();

    //LOG_DEBUG("redis setex: {} {}", key, valueStr);

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 4, argv, argvlen);
    if(!reply) {
        std::cout<<"SETEX Value Error"<<std::endl;
        // throw ExecuteFail("SETEX");
    }
    //std::stringstream cmd;
    //cmd << "SET " << key << " " << value << " EX " << sec;
    //redisReply *reply = execute(cmd.str());
    freeReplyObject(reply);
}

template<typename T>
void Redis::hset(std::string key, std::string hkey, T value)
{
    const char *argv[4];
    size_t argvlen[4];

    argv[0] = "HSET";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();

    std::stringstream ss;
    ss << value;
    std::string valueStr = ss.str();
    argv[3] = value.c_str();
    argvlen[3] = value.length();

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 4, argv, argvlen);
    if(!reply) {
        // throw ExecuteFail("HSET");
        std::cout<<"HSET Value Error"<<std::endl;
    }

    freeReplyObject(reply);
}

template<typename T>
void Redis::hget(std::string key, std::string hkey, T &hvalue) //throw(ExecuteFail, ReplyTypeError)
{
    const char *argv[3];
    size_t argvlen[3];

    argv[0] = "HGET";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 3, argv, argvlen);
    if(!reply) {
        // throw ExecuteFail("HGET");
        std::cout<<"HGET Value Error"<<std::endl;
    }
 
    if(reply->type == REDIS_REPLY_STRING) {
        std::string valueStr = reply->str;
        std::istringstream ss(valueStr);
        ss >> hvalue;
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "HGET reply type error: not string";
        // ReplyTypeError iss(CTX_ISSUE, ss.str());
        std::cout<<"HGET reply type error: not string"<<std::endl;
        // throw iss;
    }
}

template<typename T>
void Redis::hgetall(std::string key, std::map<std::string, T> &ret) //throw(ExecuteFail, ReplyTypeError)
{
    ret.clear();
    const char *argv[2];
    size_t argvlen[2];

    argv[0] = "HGETALL";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 2, argv, argvlen);
    if(!reply) {
        // throw ExecuteFail("HGETALL");
        std::cout<<"HGETALL Value Error"<<std::endl;
    }
 
    if(reply->type == REDIS_REPLY_ARRAY) {
        // std::cout << "hgetall len: " << reply->elements << std::endl;
        bool isKey = true;
        std::string k = "";
        for(size_t i=0; i<reply->elements; i++) {
            std::string s = reply->element[i]->str;
            // std::cout << s << std::endl;
            if(isKey) {
                k = s;
                isKey = false;
            }
            else {
                // std::cout<<"s:"<<s<<std::endl;
                T value;
                std::istringstream ss(s);
                ss >> value;
                ret[k] = value;
                // ret[k] = s;
                isKey = true;
            }
        }
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "HGETALL reply type error: not array";
        // ReplyTypeError iss(CTX_ISSUE, ss.str());
        // throw iss;
        std::cout<<"HGET reply type error: not array"<<std::endl;
    }
}

#endif