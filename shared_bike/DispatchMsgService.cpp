#include "DispatchMsgService.h"
#include <algorithm>
#include "bike.pb.h"
#include "events_def.h"

// 消息分发服务 ：实现了消息分发系统，负责将客户端发送的请求消息解析并分发给相应的处理模块。
// 线程池管理 ：提供了线程池的初始化和销毁功能，通过线程池来处理并发请求，
// 提高了系统的吞吐量和响应速度。
// 事件处理 ：通过事件订阅机制，将事件分发给对应的事件处理器进行处理。

DispatchMsgService* DispatchMsgService::DMS_ = nullptr;
int DispatchMsgService::clientNumber = 0;
std::forward_list<struct bufferevent*> DispatchMsgService::m_userList;
std::queue<iEvent *> DispatchMsgService::response_events;
pthread_mutex_t DispatchMsgService::queue_mutext;
NetworkInterface* DispatchMsgService::NTIF_ = nullptr;

DispatchMsgService::DispatchMsgService():tp(nullptr)
{
	
	
}

DispatchMsgService::~DispatchMsgService()
{

}

// 创建线程池并初始化相关的资源，记录线程池的初始化状态和性能参数
BOOL DispatchMsgService::open()
{
	svr_exit_ = false;

	thread_mutex_create(&queue_mutext);
	LOG_DEBUG("线程池初始化......");
	tp = thread_pool_init();
	LOG_DEBUG("当前线程池中的线程数为：tp->threads=%d", tp->threads);
	LOG_DEBUG("线程池能处理的最大任务数为：tp->max_queue=%d", tp->max_queue);
	LOG_DEBUG("线程池初始化成功！");
	return tp ? TURE : FALSE;
}

// 销毁线程池及相关的资源，清理订阅的事件处理器
void DispatchMsgService::close()
{
	svr_exit_ = true;

	thread_pool_destroy(tp);
	thread_mutex_destroy(&queue_mutext);
	subscribers_.clear();

	tp = NULL;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler * handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())
	{
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), 
			iter->second.end(), handler);
		if (hdl_iter == iter->second.end()) iter->second.push_back(handler);
	}
	else subscribers_[eid].push_back(handler);
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler * handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())
	{
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), 
			iter->second.end(), handler);
		if (hdl_iter != iter->second.end()) iter->second.erase(hdl_iter); // 相当于擦除键值
	}
}

i32 DispatchMsgService::enqueue(iEvent * ev)
{
	if (NULL == ev) return -1;
	ConnectSession* cs = (ConnectSession*)ev->get_args();
	LOG_DEBUG("将客户端[%s][%p]请求[%d]投递到任务队列中.......", 
		cs->remote_ip, cs->bev, cs->eid);
	thread_task_t* task = thread_task_alloc(0);
	task->handler = DispatchMsgService::svc;
	task->ctx = ev;

	return thread_task_post(tp, task);
}

// 处理事件的静态回调函数，调用 process 方法对事件进行处理
// 将处理后生成的响应事件放入响应事件队列中
void DispatchMsgService::svc(void * argv)
{
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	
	iEvent* ev = (iEvent*)argv;
	if (!dms->svr_exit_)
	{
		LOG_DEBUG("DispatchMsgService::svc....\n");

		// 根据用户请求生成对应的响应事件
		iEvent* rsp = dms->process(ev);
		if (rsp)
		{
			rsp->dump(std::cout);
			rsp->set_args(ev->get_args());
		}
		else
		{
			// 生成终止响应事件
			rsp = new ExitRspEv();
			rsp->set_args(ev->get_args());
		}

		ConnectSession* cs = (ConnectSession*)rsp->get_args();
		
		LOG_DEBUG("正在将事件[%s][%p]放入响应事件队列当中,eid=%d.......", 
			cs->remote_ip, cs->bev,cs->eid);
		thread_mutex_lock(&queue_mutext);
		response_events.push(rsp); // 无论什么事件，都统一放入响应事件队列当中
		thread_mutex_unlock(&queue_mutext);
	}
}

// 派发处理事件
iEvent* DispatchMsgService::process(const iEvent * ev)
{
	LOG_DEBUG("DispatchMsgService::process - ev: %p\n", ev);

	if (ev == NULL) return nullptr;
	u32 eid = ev->get_eid();
	LOG_DEBUG("DispatchMsgService::process-eid: %u\n", eid);
	
	if (eid == EEVNETID_UNKOWN)
	{
		LOG_WARN("DispatchMsgService: unknow event id %d\n", eid);
		printf("DispatchMsgService: unknow event id %d\n", eid);
		return nullptr;
	}

	T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
	if (handlers == subscribers_.end())
	{
		LOG_WARN("DispatchMsgService：no any event handler subscribed: %d\n", eid);
		printf("DispatchMsgService：no any event handler subscribed: %d\n", eid);
		return nullptr;
	}

	iEvent* rsp = NULL;
	for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
	{
		iEventHandler *handler = *iter;
		LOG_DEBUG("DispatchMsgService：get handler: %s\n", handler->get_name().c_str());
		
		// 若有需求，可以使用vector或list返回多个 rsp
		rsp = handler->handle(ev);
	}

	return rsp;
}

DispatchMsgService * DispatchMsgService::getInstance()
{
	if (DMS_ == nullptr) DMS_ = new DispatchMsgService();
	return DMS_;
}

void DispatchMsgService::addClientNumber()
{
	LOG_DEBUG("current client number:%d\n", ++clientNumber);
}

void DispatchMsgService::addUserToList(struct bufferevent* user)
{
	m_userList.push_front(user);
	LOG_DEBUG("current client number:%d\n", ++clientNumber);
}

void DispatchMsgService::decUserFromList(struct bufferevent* user)
{	
	m_userList.remove(user);
	LOG_DEBUG("current client number:%d\n", --clientNumber);

}

std::forward_list<struct bufferevent*> DispatchMsgService::getUserList()
{
	return m_userList;
}

iEvent * DispatchMsgService::parseEvent(const char * message, u32 len, i32 eid)
{
	if (!message)
	{
		LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid=%d]\n", eid);
		return nullptr;
	}
	switch (eid)
	{
		// 获取手机验证码
		case EEVENTID_GET_MOBILE_CODE_REQ:
		{	
			tutorial::mobile_request mr;
			LOG_DEBUG("客户端正在尝试获取手机验证码......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			if (mr.ParseFromArray(message, len)) // 解析数据
			{
				LOG_DEBUG("解析数据完成.客户端用户名为：%s", mr.username().c_str());
				MobileCodeReqEv* ev = new MobileCodeReqEv(mr.username());
				return ev;
			}
			break;
		}
		// 注册请求
		case EEVENTID_REGISTER_USER_REQ:
		{
			LOG_DEBUG("客户端正在请求注册......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::registerUser_request rr;
			
			if (rr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端注册的用户名为：%s， 密码为：%s，验证码为：%d", 
					rr.username().c_str(), rr.userpwd().c_str(),rr.vercode());
				RegisterReqEv* ev = new RegisterReqEv(rr.username(), rr.userpwd(), rr.vercode());
				return ev;
			}
			break;
		}
		// 添加单车请求
		case EEVENTID_ADDBIKE_REQ:
		{
			LOG_DEBUG("客户端正在尝试添加单车......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::addBike_request ar;
			
			if (ar.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端请求添加的单车Id为：%d， 经度：%lf, 纬度：%lf",
					ar.bikeid(), ar.longitude(), ar.latitude());
				AddBikeReqEv* ev = new AddBikeReqEv(ar.bikeid(), ar.longitude(), ar.latitude());
				return ev;
			}
			break;
		}
		// 删除单车请求
		case EEVENTID_DELETEBIKE_REQ:
		{
			LOG_DEBUG("客户端正在尝试移除单车......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::deleteBike_request dr;
			
			if (dr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端请求移除的单车Id为：%d", dr.bikeid());
				DeleteBikeReqEv* ev = new DeleteBikeReqEv(dr.bikeid());
				return ev;
			}
			break;
		}
		// 登陆
		case EEVENTID_LOGIN_REQ:
		{
			LOG_DEBUG("客户端正在尝试登陆......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::login_request lr;
			if (lr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端用户名为：%s， 密码为：%s", 
					lr.username().c_str(), lr.userpwd().c_str());
				LoginReqEv* ev = new LoginReqEv(lr.username(), lr.userpwd());
				return ev;
			}
			break;
		}
		// 获取扫码骑行请求
		case EEVENTID_SCANQRSTART_REQ:
		{
			LOG_DEBUG("客户端正在请求扫码骑行......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::bike_scanQR_start_request bssr;
			if (bssr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端手机号为：%s，时间戳为：%ld，北纬:%lf° 东经:%lf°)", 
						bssr.username().c_str(), bssr.timestamp(),bssr.longitude(), bssr.latitude());

				bikeScanQRStartReqEv* ev = new bikeScanQRStartReqEv(bssr.username(), 
					bssr.bikeid(), bssr.timestamp(), bssr.longitude(), bssr.latitude());
				return ev;
			}
			break;
		}
		// 获取扫码还车请求
		case EEVENTID_SCANQREND_REQ:
		{
			LOG_DEBUG("客户端正在请求扫码还车......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::bike_scanQR_end_request bser;
			if (bser.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.客户端手机号为：%s，时间戳为：%d，坐标:(北纬:%lf°,东经:%lf°))",
					bser.username().c_str(), bser.timestamp(), bser.longitude(), bser.latitude());
				bikeScanQREndReqEv* ev = new bikeScanQREndReqEv(bser.username(), 
					bser.bikeid(), bser.timestamp(), bser.longitude(), bser.latitude());
				return ev;
			}
			break;
		}
		// 获取扫码故障报修请求
		case EEVENTID_FAULT_REQ:
		{
			LOG_DEBUG("客户端正在请求扫码故障报修......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::bike_fault_request bfr;
			if (bfr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.报修单车id为：%d，时间戳为：%d，坐标:(北纬:%lf°,东经:%lf°))",
					bfr.bike_id(), bfr.timestamp(), bfr.longitude(), bfr.latitude());
				bikeScanQRFaultReqEv* ev = new bikeScanQRFaultReqEv(bfr.username().c_str(),
					bfr.bike_id(), bfr.timestamp(),bfr.longitude(), bfr.latitude());
				return ev;
			}
			break;
		}
		//获取扫码报修完成请求
		case EEVENTID_REPAIRFINISH_REQ:
		{
			LOG_DEBUG("客户端正在请求扫码报修完成......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::bike_repair_finish_request brfr;
			if (brfr.ParseFromArray(message, len))
			{
				LOG_DEBUG("解析数据完成.报修完成单车id为：%d，时间戳为：%d，坐标:(北纬:%lf°,东经:%lf°))",
					brfr.bike_id(), brfr.timestamp(), brfr.longitude(), brfr.latitude());
				bikeScanQRRepairFinishReqEv* ev = new bikeScanQRRepairFinishReqEv(brfr.username().c_str(),
					brfr.bike_id(), brfr.timestamp(), brfr.longitude(), brfr.latitude());
				return ev;
			}
			break;
		}
		//广播请求
		case EEVENTID_BROADCAST_MSG_REQ:
		{
			LOG_DEBUG("正在将单车信息广播到各个客户端......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::broadcast_msg_request bmr;
			if (bmr.ParseFromArray(message, len))
			{
				
				LOG_DEBUG("解析数据完成.广播信息: 单车Id:%d, 单车状态：%d，经纬度(%lf,%lf))", 
					bmr.bikeid(), bmr.bikestatus(),bmr.longitude(), bmr.latitude());
				broadcastMsgReqEv* ev = new broadcastMsgReqEv(bmr.bikeid(), bmr.bikestatus(), 
					bmr.longitude(), bmr.latitude());
				return ev;
			}
			break;
		}
		case EEVENTID_LIST_BIKEINFO_REQ:
		{
			tutorial::list_all_bikeinfo_request labr;			
			if (labr.ParseFromArray(message, len))
			{
				LOG_DEBUG("用户[%s]已登陆，正在请求信息同步......", labr.username().c_str());
				bikeInfoReqEv* ev = new bikeInfoReqEv(labr.username());
				return ev;
			}
			break;
		}
		//获取账户信息请求
		case EEVENTID_LIST_ACCOUNT_RECORDS_REQ:
		{
			LOG_DEBUG("客户端结束骑行，正在获取账户信息......");
			LOG_DEBUG("正在从客户端序列化的massage中解析数据.......");
			tutorial::list_account_records_request recordRequest;
			if (recordRequest.ParseFromArray(message, len))
			{				
				ListAccountRecordsReqEv* ev = new ListAccountRecordsReqEv(recordRequest.username());
				return ev;
			}
			break;
		}
		 
		
	default:
		break;
	}

	return nullptr;
}

void DispatchMsgService::handleAllResponseEvent(NetworkInterface* interface)
{
	bool done = false;
	while (!done)
	{
		iEvent* ev = nullptr;
		thread_mutex_lock(&queue_mutext);
		if (!response_events.empty())
		{
			printf("!response_events.empty()\n");
			ev = response_events.front(); // 拿出响应的第一个事件
			response_events.pop();
		}
		else
		{
			done = true;
		}
		thread_mutex_unlock(&queue_mutext);

		if (!done)
		{			
			u32 EventId = ev->get_eid();
			LOG_DEBUG("正在响应事件--eid=%d", EventId);
			switch (EventId)
			{
				// 获取手机验证码响应
				case EEVENTID_GET_MOBILE_CODE_RSP:
				{
					LOG_DEBUG("正在响应用户获取手机验证码请求--id:EEVENTID_GET_MOBILE_CODE_RSP\n");
					// 发送响应信息
					sendPesponseMessage(ev, EEVENTID_GET_MOBILE_CODE_RSP, interface);
					break;
				}
				// 用户注册响应
				case EEVENTID_REGISTER_USER_RSP:
				{
					LOG_DEBUG("正在响应用户注册请求--id:EEVENTID_REGISTER_USER_RSP\n");
					// 发送响应信息
					sendPesponseMessage(ev, EEVENTID_REGISTER_USER_RSP, interface);
					break;
				}
				// 登陆响应
				case EEVENTID_LOGIN_RSP:
				{
					LOG_DEBUG("正在响应用户手机登陆请求--id:EEVENTID_LOGIN_RSP\n");
					ConnectSession* cs = (ConnectSession*)ev->get_args();
					sendPesponseMessage(ev, EEVENTID_LOGIN_RSP, interface);
					break;
				}
				// 扫码骑行响应
				case EEVENTID_SCANQRSTART_PSP:
				{
					LOG_DEBUG("正在响应用户扫码骑行请求--id:EEVENTID_SCANQRSTART_PSP\n");
					sendPesponseMessage(ev, EEVENTID_SCANQRSTART_PSP, interface);
					break;
				}
				// 扫码还车响应
				case EEVENTID_SCANQREND_PSP:
				{
					LOG_DEBUG("正在响应用户扫码还车请求--id:EEVENTID_SCANQREND_PSP\n");
					sendPesponseMessage(ev, EEVENTID_SCANQREND_PSP, interface);
					break;
				}
				// 扫码故障报修响应
				case EEVENTID_FAULT_PSP:
				{
					LOG_DEBUG("正在响应用户扫码故障报修请求--id:EEVENTID_FAULT_PSP\n");
					sendPesponseMessage(ev, EEVENTID_FAULT_PSP, interface);
					break;
				}
				// 扫码报修完成响应
				case EEVENTID_REPAIRFINISH_PSP:
				{
					LOG_DEBUG("正在响应用户扫码故障报修请求--id:EEVENTID_FAULT_PSP\n");
					sendPesponseMessage(ev, EEVENTID_REPAIRFINISH_PSP, interface);
					break;
				}
				// 广播响应
				case EEVENTID_BROADCAST_MSG_RSP:
				{
					LOG_DEBUG("正在广播到各客户端--id:EEVENTID_BROADCAST_MSG_RSP\n");
					sendPesponseMessage(ev, EEVENTID_BROADCAST_MSG_RSP, interface);
					break;
				}
				// 单车信息同步响应
				case EEVENTID_LIST_BIKEINFO_RSP:
				{
					LOG_DEBUG("正在广播到各客户端--id:EEVENTID_BROADCAST_MSG_RSP\n");
					sendPesponseMessage(ev, EEVENTID_LIST_BIKEINFO_RSP, interface);
					break;
				}
				// 账户记录请求响应
				case EEVENTID_ACCOUNT_RECORDS_RSP:
				{
					LOG_DEBUG("正在响应用户获取账户记录请求--id:EEVENTID_ACCOUNT_RECORDS_RSP\n");
					sendPesponseMessage(ev, EEVENTID_ACCOUNT_RECORDS_RSP, interface);
					break;
				}
				// 添加单车响应
				case EEVENTID_ADDBIKE_RSP:
				{
					LOG_DEBUG("正在响应用户添加单车请求--id:EEVENTID_ADDBIKE_RSP\n");
					// 发送响应信息
					sendPesponseMessage(ev, EEVENTID_ADDBIKE_RSP, interface);
					break;
				}
				// 删除单车响应
				case EEVENTID_DELETEBIKE_RSP:
				{
					LOG_DEBUG("正在响应用户删除单车请求--id:EEVENTID_DELETEBIKE_RSP\n");
					// 发送响应信息
					sendPesponseMessage(ev, EEVENTID_DELETEBIKE_RSP, interface);
					break;
				}
				// 退出响应
				case EEVNETID_EXIT_RSP:
				{
					LOG_DEBUG("退出响应--ev->get_eid() == EEVNETID_EXIT_RSP\n");
					ConnectSession* cs = (ConnectSession*)ev->get_args();
					cs->response = ev;
					interface->send_response_message(cs);
					break;
				}
				
				default:
					LOG_DEBUG("收到未知的响应，EventId=%d\n", EventId);
					break;
			}										
		}
	}
}

void DispatchMsgService::sendPesponseMessage(iEvent* ev, EventID Eid,
	NetworkInterface* interface)
{
	ConnectSession* cs = (ConnectSession*)ev->get_args();
	cs->response = ev;

	// 系列化请求数据
	cs->message_len = ev->Bytesize();
	cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];
	cs->eid = Eid;

	// 组装头部
	memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
	*(u16*)(cs->write_buf + 4) = Eid;
	*(i32*)(cs->write_buf + 6) = cs->message_len;
	LOG_DEBUG("正在序列化数据......");
	ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->message_len);

	interface->send_response_message(cs);
}


