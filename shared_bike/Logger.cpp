#include "Logger.h"

#include <iostream>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RemoteSyslogAppender.hh>
#include <log4cpp/PropertyConfigurator.hh>

Logger Logger::instance_; // 定义类的单列成员，static Logger instance_;

bool Logger::init(const std::string & log_conf_file)
{
    try
    {
        log4cpp::PropertyConfigurator::configure(log_conf_file); // 加载日志配置
    }
    catch(log4cpp::ConfigureFailure& f) 
    {
        std::cerr << " load log config file " << log_conf_file.c_str()
            << " failed with result : " << f.what()<< std::endl;
        return false;
    }

    Category_ = &log4cpp::Category::getRoot(); // 获取根日志器
	
    return true;
}

