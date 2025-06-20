﻿#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "sqlconnection.h"
#include "iniconfig.h"
#include "user_service.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>

UserEventHandler::UserEventHandler() :iEventHandler("UserEventHandler")
{
	//未来需要订阅事件的处理
	DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_REGISTER_USER_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_ADDBIKE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_DELETEBIKE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_SCANQRSTART_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_SCANQREND_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_FAULT_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_REPAIRFINISH_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_BROADCAST_MSG_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_LIST_BIKEINFO_REQ, this);
	
	thread_mutex_create(&pm_);
}

UserEventHandler::~UserEventHandler()
{
	//未来需要实现退定事件的处理
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_REGISTER_USER_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_ADDBIKE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_DELETEBIKE_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_SCANQRSTART_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_SCANQREND_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_FAULT_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_REPAIRFINISH_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_BROADCAST_MSG_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LIST_BIKEINFO_REQ, this);
	thread_mutex_destroy(&pm_);
}
//服务器端根据事件Id，处理请求并生成响应返回给客户端
iEvent * UserEventHandler::handle(const iEvent * ev)
{
	u32 eid = ev->get_eid();
	switch (eid)
	{
		case EEVENTID_GET_MOBILE_CODE_REQ:
			return handle_mobile_code_req((MobileCodeReqEv*)ev);
			break;
		case EEVENTID_REGISTER_USER_REQ:
			return handle_user_register_req((RegisterReqEv*)ev);
			break;
		case EEVENTID_LOGIN_REQ:
			return handle_login_req((LoginReqEv*)ev);
			break;
		case EEVENTID_SCANQRSTART_REQ:
			return handle_bike_scanQRSatrt_req((bikeScanQRStartReqEv*)ev);
			break;
		case EEVENTID_SCANQREND_REQ:
			return handle_bike_scanQREnd_req((bikeScanQREndReqEv*)ev);
			break;
		case EEVENTID_FAULT_REQ:
			return handle_bike_scanQRFault_req((bikeScanQRFaultReqEv*)ev);
			break;
		case EEVENTID_REPAIRFINISH_REQ:
			return handle_bike_scanQRRepairFinish_req((bikeScanQRRepairFinishReqEv*)ev);
			break;
		case EEVENTID_BROADCAST_MSG_REQ:
			return handle_broadcast_msg_req((broadcastMsgReqEv*)ev);
			break;
		case EEVENTID_LIST_BIKEINFO_REQ:
			return handle_bikeInfo_req((bikeInfoReqEv*)ev);
			break;
		case EEVENTID_LIST_ACCOUNT_RECORDS_REQ:
			return handle_list_account_records_req((ListAccountRecordsReqEv*)ev);
			break;
		case EEVENTID_ADDBIKE_REQ:
			return handle_addBike_req((AddBikeReqEv*)ev);
			break;
		case EEVENTID_DELETEBIKE_REQ:
			return handle_deleteBike_req((DeleteBikeReqEv*)ev);
			break;
		default:
			break;
	}
	return NULL;
}


MobileCodeRspEv * UserEventHandler::handle_mobile_code_req(MobileCodeReqEv * ev)
{
	i32 verCode = 0;
	std::string userName = ev->get_userName();
	LOG_DEBUG("try to get moblie phone. %s validate code .", userName.c_str());
	LOG_DEBUG("正在为用户[%s]生成随机验证码......", userName.c_str());

	verCode = code_gen();
	thread_mutex_lock(&pm_);

	m2c_[userName] = verCode;
	thread_mutex_unlock(&pm_);

	LOG_DEBUG("用户[%s]验证码[%d]已生成！", userName.c_str(), verCode);

	return new MobileCodeRspEv(ERRC_SUCCESS, verCode);
}

RegisterResEv* UserEventHandler::handle_user_register_req(RegisterReqEv* ev)
{
	std::string userName = ev->get_userName(); //取出用户名 手机号码
	std::string userPwd = ev->get_userPwd();
	i32 verCode = ev->get_verCode();

	const char* str_userName = userName.c_str();
	
	LOG_DEBUG("正在匹配用户[%s]->验证码[%d]......", str_userName, verCode);
	thread_mutex_lock(&pm_);

	auto iter = m2c_.find(userName);
	if (iter == m2c_.end()) return NULL;

	if (((iter != m2c_.end()) && (verCode != iter->second)) || (iter == m2c_.end()))
	{
		LOG_WARN("用户[%s]和验证码[%d]匹配失败！\n", str_userName, verCode);
		thread_mutex_unlock(&pm_);
		return new RegisterResEv(userName, userPwd, ERRC_INVALID_DATA);
	}
	thread_mutex_unlock(&pm_);
	LOG_DEBUG("用户[%s]验证码[%d]匹配成功！", str_userName, verCode);

	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new RegisterResEv(userName, userPwd, ERRO_PROCCESS_FALED);
	}

	LOG_DEBUG("打开数据库成功！");
	UserService us(mysqlconn);
	bool result = false;

	if (!us.exist(userName))
	{
		LOG_DEBUG("该用户为共享单车新用户，正在为该用户[%s]初始化账户......", userName.c_str());
		
		result = us.insert(userName, userPwd);
		if (!result)
		{
			LOG_DEBUG("新用户[%s]信息初始化失败！", userName.c_str());
			return new RegisterResEv(userName, userPwd, ERRO_PROCCESS_FALED);
		}
		LOG_DEBUG("新用户[%s]信息注册完成，信息已录入数据库.", userName.c_str());
		return new RegisterResEv(userName, userPwd, ERRC_SUCCESS);
	}
	else
	{
		LOG_DEBUG("用户[%s]已注册，驳回注册请求", userName.c_str());
		return new RegisterResEv(userName, userPwd, ERRC_INVALID_MSG);
	}
	
}

//添加单车请求
AddBikeResEv* UserEventHandler::handle_addBike_req(AddBikeReqEv* ev)
{
	const i32 bikeid = ev->get_bikeId();
	const r64 longitude = ev->get_longitude();
	const r64 latitude = ev->get_latitude();

	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_addBike_req - Database init failed. exit!\n");
		return new AddBikeResEv(bikeid, longitude, latitude, ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");
	UserService us(mysqlconn);

	char sql_content[1024];
	sprintf(sql_content, "SELECT devno FROM bikeinfo");
	SqlRecordSet record_set;
	if (!us.findBikeInfo(sql_content, record_set))
	{
		LOG_DEBUG("sql[%s]语句执行失败！错误信息：%s", sql_content, mysqlconn->GetErrInfo());
		return new AddBikeResEv(bikeid, longitude, latitude, ERRO_PROCCESS_FALED);
	}

	MYSQL_ROW row = record_set.FetchRow();
	i32 maxBikeId;

	if (row)
	{
		memset(sql_content, '\0', sizeof(sql_content));
		sprintf(sql_content, "SELECT MAX(devno) FROM bikeinfo");
		SqlRecordSet record_set;
		if (!us.findBikeInfo(sql_content, record_set))
		{
			LOG_DEBUG("查询单车信息MAX(devno)失败！错误信息：%s", mysqlconn->GetErrInfo());
			return new AddBikeResEv(bikeid, longitude, latitude, ERRO_PROCCESS_FALED);
		}
		MYSQL_ROW row = record_set.FetchRow();
		maxBikeId = atoi(row[0]) + 1;
	}
	else maxBikeId = 1;

	if (!us.addBike(maxBikeId, longitude, latitude))
	{
		LOG_DEBUG("添加单车[%d]信息失败", maxBikeId);
		return new AddBikeResEv(maxBikeId, longitude, latitude, ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("添加单车[%d]，经纬度(%lf, %lf)成功！", maxBikeId, longitude, latitude);
	return new AddBikeResEv(maxBikeId, longitude, latitude, ERRC_SUCCESS);
}

DeleteBikeResEv* UserEventHandler::handle_deleteBike_req(DeleteBikeReqEv* ev)
{
	const i32 bikeid = ev->get_bikeId();
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_addBike_req - Database init failed. exit!\n");
		return new DeleteBikeResEv(bikeid, ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");
	UserService us(mysqlconn);

	if (!us.deleteBike(bikeid))
	{
		LOG_DEBUG("移除单车[%d]失败", bikeid);
		return new DeleteBikeResEv(bikeid, ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("移除单车[%d]成功！", bikeid);
	return new DeleteBikeResEv(bikeid, ERRC_SUCCESS);
}

i32 UserEventHandler::code_gen()
{
	i32 code = 0;
	srand((unsigned int)time(NULL));

	code = (unsigned int)(rand() % (999999 - 100000) + 100000);
	LOG_DEBUG("随机验证码已生成！");
	return code;
}

LoginResEv * UserEventHandler::handle_login_req(LoginReqEv * ev)
{
	std::string userName = ev->get_userName(); //取出用户名 手机号码
	std::string userPwd = ev->get_userPwd();

	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new LoginResEv(userName, userPwd, 0, ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");

	UserService us(mysqlconn);
	if (!us.verifyUserInfo(userName, userPwd))
	{
		LOG_DEBUG("用户账号[%s]和密码[%s]信息匹配失败!\n", userName.c_str(), userPwd.c_str());
		return new LoginResEv(userName, userPwd, 0, ERRC_INVALID_MSG);;
	}	
	if (userName == "qiniubike" && userPwd == "qiniubike") {
		LOG_DEBUG("管理员[%s]已登录!\n", userName.c_str());
		return new LoginResEv(userName, userPwd, 1, ERRC_SUCCESS);;
	}
	
	LOG_DEBUG("用户[%s]登陆成功!\n", userName.c_str());		
	return new LoginResEv(userName, userPwd, 0, ERRC_SUCCESS);;
}

//根据用户扫码骑行请求对数据进行处理，并生成扫码骑行响应事件
bikeScanQRStartResponseEv* 
UserEventHandler::handle_bike_scanQRSatrt_req(bikeScanQRStartReqEv* ev)
{
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....handle_bike_scanQRSatrt_req");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new bikeScanQRStartResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}

	LOG_DEBUG("打开数据库成功！");

	UserService us(mysqlconn);

	const double longitude = ev->get_longitude();
	
	char sql_content_ride_record[1024];
	char sql_content_updataBikeinfo[1024];
	sprintf(sql_content_updataBikeinfo, 
		"UPDATE bikeinfo SET status=%d, tmsg = 'bike ride start, riding......', \ 
		time = FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'), 
		bike_point = GeomFromText('POINT(%lf %lf)') WHERE devno = %d",
		BIKE_RIDE_START, ev->get_timestamp(), ev->get_longitude(), 
		ev->get_latitude(), ev->get_bikeId());
	sprintf(sql_content_ride_record, 
		"insert into user_ride_record_info(username,bikeId,bikeStatus,start_time, start_point) \
		values(\"%s\", %d, %d, FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'),GeomFromText('POINT(%lf %lf)'))", 
		ev->get_userName().c_str(), ev->get_bikeId(), BIKE_RIDE_START, 
		ev->get_timestamp(),ev->get_longitude(), ev->get_latitude());
	LOG_DEBUG("sql_content:[%s]", sql_content_ride_record);
	
	if (!us.insertUseRideRecord(sql_content_ride_record))
	{
		LOG_DEBUG("用户骑行记录插入user_ride_record_info失败！");
		LOG_DEBUG("错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRStartResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	if (!us.updataBikeinfo(sql_content_updataBikeinfo))
	{
		LOG_DEBUG("单车状态bikeinfo信息更新失败！");
		LOG_DEBUG("错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRStartResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	LOG_DEBUG("用户骑行记录插入user_ride_record_info成功！单车状态bikeinfo信息已更新！");
	return new bikeScanQRStartResponseEv(ERRC_SUCCESS, ev->get_timestamp(), 
		ev->get_longitude(), ev->get_latitude());;
}
//根据用户扫码还车请求对数据进行处理，并生成扫码还车响应事件
bikeScanQREndResponseEv* UserEventHandler::handle_bike_scanQREnd_req(bikeScanQREndReqEv* ev)
{
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new bikeScanQREndResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	LOG_DEBUG("打开数据库成功！handle_bike_scanQREnd_req");

	UserService us(mysqlconn);

	char sql_content_ride_record[1024];
	char sql_content_updataBikeinfo[1024];
	LOG_DEBUG("正在更新单车信息...");
	sprintf(sql_content_updataBikeinfo, "UPDATE bikeinfo SET status=%d, tmsg= 'bike ride end, waiting for use', \ 
		time = FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'), 
		bike_point = GeomFromText('POINT(%lf %lf)') WHERE devno = %d",
		BIKE_RIDE_END, ev->get_timestamp(), ev->get_longitude(), ev->get_latitude(),
		ev->get_bikeId());
	LOG_DEBUG("正在更新用户骑行记录...");
	sprintf(sql_content_ride_record, "insert into user_ride_record_info(username,bikeId,bikeStatus,end_time, end_point) \
		values(\"%s\", %d, %d, FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'),GeomFromText('POINT(%lf %lf)'))",
		ev->get_userName().c_str(), ev->get_bikeId(), BIKE_RIDE_END, 
		ev->get_timestamp(), ev->get_longitude(), ev->get_latitude());

	if (!us.insertUseRideRecord(sql_content_ride_record))
	{
		LOG_DEBUG("更新数据库user_ride_record_info失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQREndResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	if (!us.updataBikeinfo(sql_content_updataBikeinfo))
	{
		LOG_DEBUG("单车状态bikeinfo信息更新失败！");
		LOG_DEBUG("错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQREndResponseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	LOG_DEBUG("单车信息及用户骑行记录更新成功！已生成扫码骑行响应事件");

	return new bikeScanQREndResponseEv(ERRC_SUCCESS, ev->get_timestamp(), 
		ev->get_longitude(), ev->get_latitude());
}

//根据用户扫码报修请求对数据进行处理，并生成扫码报修响应事件
bikeScanQRFaultResponseEv* UserEventHandler::handle_bike_scanQRFault_req(bikeScanQRFaultReqEv* ev)
{
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new bikeScanQRFaultResponseEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");

	UserService us(mysqlconn);

	char sql_content_ride_record[1024];
	char sql_content_updataBikeinfo[1024];

	sprintf(sql_content_updataBikeinfo, "UPDATE bikeinfo SET status=%d, tmsg = 'bike fault, waiting for repair......', \ 
		time = FROM_UNIXTIME(% d, '%%Y-%%m-%%d %%H:%%i:%%S'), 
		bike_point = GeomFromText('POINT(%lf %lf)') WHERE devno = % d",
		BIKE_FAULT, ev->get_timestamp(), ev->get_longitude(), 
		ev->get_latitude(), ev->get_bikeId());

	sprintf(sql_content_ride_record, "insert into user_ride_record_info(username,bikeId,bikeStatus,start_time, start_point) \
		values(\"%s\", %d, %d, FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'),GeomFromText('POINT(%lf %lf)'))",
		ev->get_userName().c_str(), ev->get_bikeId(), BIKE_FAULT, 
		ev->get_timestamp(), ev->get_longitude(), ev->get_latitude());

	if (!us.insertUseRideRecord(sql_content_ride_record))
	{
		LOG_DEBUG("插入用户记录表bikeinfo失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRFaultResponseEv(ERRO_PROCCESS_FALED);
	}
	if (!us.updataBikeinfo(sql_content_updataBikeinfo))
	{
		LOG_DEBUG("更新数据表bikeinfo失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRFaultResponseEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("用户单车信息bikeinfo成功！");

	return new bikeScanQRFaultResponseEv(ERRC_SUCCESS);
}
//根据用户扫码报修完成请求对数据进行处理，并生成扫码报修完成响应事件
bikeScanQRRepairFinishResponseEv* UserEventHandler::
handle_bike_scanQRRepairFinish_req(bikeScanQRRepairFinishReqEv* ev)
{
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new bikeScanQRRepairFinishResponseEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");

	UserService us(mysqlconn);

	char sql_content_ride_record[1024];
	char sql_content_updataBikeinfo[1024];

	sprintf(sql_content_updataBikeinfo, "UPDATE bikeinfo SET status=%d, tmsg = 'bike repair finish, waiting for use', \ 
		time = FROM_UNIXTIME(% d, '%%Y-%%m-%%d %%H:%%i:%%S'), 
		bike_point = GeomFromText('POINT(%lf %lf)') WHERE devno = % d",
		BIKE_REPAIR_FINISH, ev->get_timestamp(), ev->get_longitude(), 
		ev->get_latitude(), ev->get_bikeId());

	sprintf(sql_content_ride_record, "insert into user_ride_record_info(username,bikeId,bikeStatus,end_time, end_point) \
		values(\"%s\", %d, %d, FROM_UNIXTIME(%d, '%%Y-%%m-%%d %%H:%%i:%%S'),GeomFromText('POINT(%lf %lf)'))",
		ev->get_userName().c_str(), ev->get_bikeId(), BIKE_REPAIR_FINISH, 
		ev->get_timestamp(), ev->get_longitude(), ev->get_latitude());

	if (!us.insertUseRideRecord(sql_content_ride_record))
	{
		LOG_DEBUG("插入用户记录表user_ride_record_info失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRRepairFinishResponseEv(ERRO_PROCCESS_FALED);
	}

	if (!us.updataBikeinfo(sql_content_updataBikeinfo))
	{
		LOG_DEBUG("更新数据表bikeinfo失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeScanQRRepairFinishResponseEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("用户单车信息bikeinfo成功！");

	return new bikeScanQRRepairFinishResponseEv(ERRC_SUCCESS);
}

broadcastMsgResponseEv* UserEventHandler::handle_broadcast_msg_req(broadcastMsgReqEv* ev)
{
	return new broadcastMsgResponseEv(ev->get_bikeid(), ev->get_bikestatus(),
		ev->get_bikeLongitude(), ev->get_bikeLatitude());
}

bikeInfoResponseEv* UserEventHandler::handle_bikeInfo_req(bikeInfoReqEv* ev)
{

	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new bikeInfoResponseEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");

	UserService us(mysqlconn);
	char sql_content[1024];
	sprintf(sql_content, "SELECT devno, status, x(bike_point) x, y(bike_point) y FROM bikeinfo");
	SqlRecordSet record_set;
	if (!us.findBikeInfo(sql_content, record_set))
	{
		LOG_DEBUG("查询单车信息bikeinfo失败！错误信息：%s", mysqlconn->GetErrInfo());
		return new bikeInfoResponseEv(ERRO_PROCCESS_FALED);
	}
	bikeInfoResponseEv* bikeResEv = new bikeInfoResponseEv(ERRC_SUCCESS);
	MYSQL_RES* res = record_set.GetResult();
	MYSQL_ROW row;
	int columns = mysql_num_fields(res);
	// 检索一个结果集合的下一行。当在mysql_store_result()之后使用时，如果没有更多的行可检索时，
	// mysql_fetch_row()返回NULL。当在mysql_use_result()之后使用时，
	// 当没有更多的行可检索时或如果出现一个错误，mysql_fetch_row()返回NULL。
	while (row = mysql_fetch_row(res))
	{		
		bikeResEv->set_bikeResponse_msg(atoi(row[0]), atoi(row[1]), 
			strtod(row[2],NULL), strtod(row[3], NULL));
	}
	LOG_DEBUG("同步信息已序列化完成！");
	return bikeResEv;
}

ListAccountRecordsResonseEv* UserEventHandler::
handle_list_account_records_req(ListAccountRecordsReqEv* ev)
{
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new ListAccountRecordsResonseEv(ERRO_PROCCESS_FALED, 0, 0, 0);
	}
	LOG_DEBUG("打开数据库成功！");

	return nullptr;
}
