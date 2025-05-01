// 日志和配置加载 ：加载日志配置文件和系统配置文件，初始化日志系统和数据库连接
// 系统初始化 ：启动网络接口，监听客户端的连接请求
// 主事件循环 ：通过事件循环持续监听网络事件，处理客户端的请求

#include <iostream>
#include <unistd.h>
#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "glob.h"
#include "iniconfig.h"
#include "Logger.h"
#include "sqlconnection.h"
#include "SqlTables.h"
#include "BusProcessor.h"

using namespace std;

int main(int argc, char **argv)
{
	//如果初始化日记失败，不成功
	
	// 初始化日志系统，加载日志配置文件
	if (!Logger::instance()->init("/home/projects/conf/log.conf"))
	{
		fprintf(stderr, "init log module failed.\n");
		return -2;
	}

	Iniconfig* config = Iniconfig::getInstance();
	if (!config->loadfile("/home/projects/conf/share_bike.ini"))
	{
		printf("load %s failed.\n", argv[1]);
		LOG_ERROR("load %s failed.", argv[1]);
		Logger::instance()->GetHandle()->error("load %s failed.", argv[1]);

		return -3;
	}
	if (argc == 4 && std::string(argv[3]) == "TCP/UDP")
	{
		LOG_DEBUG("当前服务器工作模式为TCP/UDP并发量测试\n");
	}
	
	st_env_config conf_args = config->getconfig();
	LOG_INFO("[data ip]:%s, port: %d, user:%s, pwd:%s, db:%s [server] port:%d\n",
		conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(), 
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str(), conf_args.svr_port);
	
	// 加载系统配置文件，获取数据库和服务器的配置信息
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);

	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("Database init failed. exit!\n");
		return -4;
	}

	// 初始化数据库连接，并创建数据库表
	BusinessProcessor busPro(mysqlconn);
	busPro.init();
	
	// 启动线程池和消息分发服务
	DispatchMsgService* DMS = DispatchMsgService::getInstance();
	DMS->open();
	
	// 启动网络接口，开始处理客户端的连接请求
	NetworkInterface *NTIF = new NetworkInterface();
	NTIF->start(conf_args.svr_port);
	LOG_DEBUG("等待客户端的连接.......\n\n");
	printf("等待客户端的连接.......\n\n");
	while (1)
	{
		NTIF->network_event_dispatch();
	}
	
	return 0;
}