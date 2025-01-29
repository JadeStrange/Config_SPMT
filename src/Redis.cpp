#include <Redis.h>
#include <string.h>
#include <hiredis/hiredis.h>

Redis *Redis::m_self = NULL;
// Redis *Redis::instance()
// {
//     if(!m_self) {
//         std::cout << "redis was not initialized." << std::endl;
//     }
//     return m_self;
// }
// void Redis::configure(std::string ip, int port)
// {
//     m_self = new Redis(ip, port);
// }

Redis::Redis(std::string ip, int port) //throw(ConnectFail)
    : m_ip(ip)
    , m_port(port)
{
    connect();
}

void Redis::connect() //throw(ConnectFail)
{
    m_context = redisConnect(m_ip.c_str(), m_port);   
    if(!m_context || m_context->err) {
        std::stringstream ss;
        ss << "can't connect redis server 0: server url is " << m_ip << ":" << m_port;
        if(m_context) {
            ss << ", reason: " << m_context->errstr;
        }
    }
}

void Redis::get(std::string key, std::string &value) //throw(ExecuteFail, ReplyTypeError)
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
        freeReplyObject(reply);
        // throw ExecuteFail("GET");
        std::cout<<"GET INT ERROR"<<std::endl;
    }
 
    if(reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "GET reply type error: not string";
    }
}

void Redis::get(std::string key, int &value)
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
        std::cout<<"GET INT ERROR"<<std::endl;
    }
 
    //std::stringstream cmd;
    //cmd << "GET " << key;
    //redisReply *reply = execute(cmd.str());
    if(reply->type == REDIS_REPLY_INTEGER) {
        value = reply->integer;
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "GET reply type error: not integer";
    }
}

void Redis::get(std::string key, std::vector<std::string> &value)
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
        std::cout<<"GET Vector Error"<<std::endl;
    }

    if(reply->type == REDIS_REPLY_ARRAY) {
        for(size_t i=0; i<reply->elements; i++) {
            value.push_back(reply->element[i]->str);
        }
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        std::stringstream ss;
        ss << key << " ";
        ss << "GET reply type error: not array 1";
    }
}

void Redis::hmset(std::string key, std::map<std::string, std::string> &vals)
{
    size_t len = 2+vals.size()*2;
    const char **argv = new const char*[len];
    size_t *argvlen = new size_t[len];

    size_t argIndex = 0;
    argv[argIndex] = "HMSET";
    argvlen[argIndex] = strlen("HMSET");
    argIndex++;

    argv[argIndex] = key.c_str();
    argvlen[argIndex] = key.length();
    argIndex++;

    for(std::map<std::string, std::string>::iterator it=vals.begin(); it!=vals.end(); it++) {
        argv[argIndex] = it->first.c_str();
        argvlen[argIndex] = it->first.length();
        argIndex++;

        argv[argIndex] = it->second.c_str();
        argvlen[argIndex] = it->second.length();
        argIndex++;
    }

    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, len, argv, argvlen);
    if(!reply) {
        // throw ExecuteFail("HMSET");
        std::cout<<"HMSET String Value Error"<<std::endl;
    }
}

void Redis::hmset(std::string key, std::map<std::string, int> &vals)
{
    std::vector<std::string> tVec;
    tVec.push_back("HMSET");
    tVec.push_back(key);

    std::map<std::string, int>::const_iterator it = vals.begin();
    for (; it != vals.end(); it++)
    {
        tVec.push_back(it->first);
        tVec.push_back(std::to_string(it->second));
    }

    std::vector<const char *> argv( tVec.size());
    std::vector<size_t> argvlen( tVec.size() );
    unsigned int j = 0;
    for ( std::vector<std::string>::const_iterator i = tVec.begin(); i != tVec.end(); ++i, ++j )
    {
        argv[j] = i->c_str();
        argvlen[j] = i->length(); 
    }
    boost::mutex::scoped_lock lock(m_execMutex);

    redisReply* reply =(redisReply*) redisCommandArgv(m_context, argv.size(), &(argv[0]), &(argvlen[0]));
    if(!reply) {
        // throw ExecuteFail("HMSET");
        std::cout<<"HMSET Int Value Error"<<std::endl;
    }
}

void Redis::hkeys(std::string key, std::vector<std::string> &ret)
{
    ret.clear();
    const char *argv[2];
    size_t argvlen[2];

    argv[0] = "HKEYS";
    argvlen[0] = strlen(argv[0]);

    argv[1] = key.c_str();
    argvlen[1] = key.length();

    //std::stringstream ss;
    //ss << argv[0] << " " << argv[1];
    //std::string cmd = ss.str();
    //std::cout << argv[0] << " " << argv[1] << std::endl;
    //redisReply *reply = (redisReply*)redisCommand(m_context, cmd.c_str());
 
    boost::mutex::scoped_lock lock(m_execMutex);
    redisReply* reply =(redisReply*) redisCommandArgv(m_context, 2, argv, argvlen);
    if(!reply) {
        // throw ExecuteFail("HKEYS");
        std::cout<<"HKEYS Value Error"<<std::endl;
    }
    
    //std::stringstream cmd;
    //cmd << "HKEYS " << key;
    //redisReply *reply = execute(cmd.str());
    if(reply->type == REDIS_REPLY_ARRAY) {
        std::cout << "len: " << reply->elements<< std::endl;
        for(size_t i=0; i<reply->elements; i++) {
            ret.push_back(reply->element[i]->str);
        }
        freeReplyObject(reply);
    }
    else {
        freeReplyObject(reply);
        // ReplyTypeError iss(CTX_ISSUE, "reply type error: not array 3");
        // throw iss;
        std::cout<<"reply type error: not array 3"<<std::endl;
    }
}
