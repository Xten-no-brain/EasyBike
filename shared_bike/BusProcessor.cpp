#include "BusProcessor.h"

// mysqlconn_ 用于存储数据库连接对象，以便后续进行数据库操作。
// ueh_ 用于存储用户事件处理器对象，负责处理与用户相关的各类事件，如注册、登录等。
BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> sqlconn)
	:mysqlconn_(sqlconn), ueh_(new UserEventHandler())
{

}

BusinessProcessor::~BusinessProcessor()
{
	ueh_.reset();
}

bool BusinessProcessor::init()
{
	SqlTables tables(mysqlconn_);
	LOG_DEBUG("数据库正在初始化......");
	if (!tables.CreateBikeTable())
	{
		LOG_ERROR("create shared_bike table failed!\n");
		printf("create shared_bike table failed!\n");
		return false;
	}
	if(!tables.CreateUserInfo())
	{
		LOG_ERROR("create user table failed！\n");
		printf("create user table failed！\n");
		return false;
	}
	if (!tables.UserRideRecordInfo())
	{
		LOG_ERROR("create UserRideRecordInfo table failed！\n");
		printf("create UserRideRecordInfo table failed！\n");
		return false;
	}
	LOG_DEBUG("数据库初始化完成......\n");
	return true;
}
