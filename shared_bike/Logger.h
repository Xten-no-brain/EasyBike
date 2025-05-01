#ifndef DISTRIBUTED_LOGGER_H_
#define DISTRIBUTED_LOGGER_H_

// 功能：基于log4cpp的日志系统，提供单例模式和不同级别的日志接口。

#include <string>
#include <log4cpp/Category.hh>


class Logger
{
public:
	bool init(const std::string& log_conf_file);
	static Logger* instance() 
	{
		return &instance_;
	}
	
	log4cpp::Category* GetHandle()	
	{
		return Category_;
	}
	
protected:
	static Logger instance_;
	log4cpp::Category* Category_;
};

#define LOG_INFO  Logger::instance()->GetHandle()->info
#define LOG_DEBUG Logger::instance()->GetHandle()->debug
#define LOG_ERROR Logger::instance()->GetHandle()->error
#define LOG_WARN  Logger::instance()->GetHandle()->warn

#endif