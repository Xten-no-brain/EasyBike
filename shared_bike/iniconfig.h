#ifndef SHBK_COMMON_INICONFIG_H_
#define SHBK_COMMON_INICONFIG_H_

// 功能：单例模式读取和解析INI配置文件，获取数据库和服务器配置。

#include <string>
#include "configdef.h"

class Iniconfig
{
protected:
	Iniconfig();
	
public:
	static Iniconfig* getInstance();
	~Iniconfig();
	
	bool loadfile(const std::string& path);
	const st_env_config& getconfig();
	
private:
	st_env_config _config;
	static Iniconfig* config;
	bool _isloaded;
	
};


#endif