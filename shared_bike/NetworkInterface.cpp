﻿// 网络事件处理 ：实现网络事件的处理逻辑，如连接建立、数据读取、数据写入、错误处理等
// 会话管理和缓冲区操作 ：管理客户端会话，包括会话的初始化和释放，以及数据的读写缓冲区操作

#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



//切记，ConnectSession 必须是C类型的成员变量
// 初始化客户端会话，分配内存并初始化会话结构体
static ConnectSession* session_init(int fd, struct bufferevent* bev)
{
	ConnectSession* temp = nullptr; //bufferevent
	temp = new ConnectSession();

	if (!temp)
	{
		fprintf(stderr, "malloc failed. reason: %m\n");
		return nullptr;
	}
	memset(temp, '\0', sizeof(ConnectSession));
	temp->bev = bev;//bufferevent
	temp->fd = fd;
	return temp;
}

void session_free(ConnectSession *cs)
{
	if (cs)
	{		
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
	
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		delete cs;
		cs = nullptr;
	}
}

NetworkInterface::NetworkInterface()
{
	base_ = nullptr;
	listener_ = nullptr;
}

NetworkInterface::~NetworkInterface()
{
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	base_ = event_base_new();

	listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, 
		base_, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		512, (struct sockaddr*)&sin,
		sizeof(struct sockaddr_in));
	return true;
}

void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}
	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::session_reset(ConnectSession * cs)
{
	if (cs)
	{
		LOG_DEBUG("正在释放资源......");
		if (cs->read_buf)
		{
			delete cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete cs->write_buf;
			cs->write_buf = nullptr;
		}

		cs->session_stat = SESSION_STATUS::SS_REQUEST;
		cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->read_header_len = 0;
		LOG_DEBUG("释放资源完成！");
	}
}

// 处理客户端连接请求
// 创建一个新的会话，设置会话的状态和缓冲区
// 记录客户端的 IP 地址和连接信息
// 将客户端会话添加到用户列表中
// 设置事件回调函数，开始读取客户端发送的数据
void NetworkInterface::listener_cb(evconnlistener * listener, evutil_socket_t fd, 
	sockaddr * sock, int socklen, void * arg)
{
	struct event_base* base = (struct event_base*)arg;
	
	//为这个客户端分配一个bufferevent
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	ConnectSession* cs = session_init(fd, bev);
	
	cs->session_stat = SESSION_STATUS::SS_REQUEST;//
	cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

	strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	LOG_DEBUG("接收到共享单车用户连接请求， ip :%s[%p]\n", cs->remote_ip, cs->bev);
	LOG_DEBUG("用户已成功连接服务器！ ip :%s[%p]\n", cs->remote_ip, cs->bev);

	DispatchMsgService::addUserToList(cs->bev);
	
	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);

	//设置超时值，10秒没有写或者读时，即超时时，调用出错函数 handle_error
	bufferevent_settimeout(bev, 1000, 1000); //超时值应设置在配置文件中
}

// 处理客户端发送的请求
// 读取请求头部，解析事件 ID 和消息长度
// 根据消息长度读取消息内容
// 将消息内容解析为事件对象，分发给消息分发服务进行处理
void NetworkInterface::handle_request(bufferevent * bev, void * arg)
{
	ConnectSession *cs = (ConnectSession*)(arg);
	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)
	{
		LOG_WARN("NetworkInterface::handle_request wrong session stat[%d]\n", cs->session_stat);
		return;
	}
	
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)//读取头部
	{
		i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, 
			MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;
		cs->header[cs->read_header_len] = '\0';
		LOG_DEBUG("recv from client cs->header[%s], lenth[%d]", 
			cs->header, cs->read_header_len);

		if (cs->read_header_len == MESSAGE_HEADER_LEN)//读取头部长度
		{
			if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)
			{
				//FBEB	   事件ID	   数据长度N	 数据内容
				//4字节    2个字节      4个字节
				cs->eid = *((u16*)(cs->header + 4)); //获取事件ID
				cs->message_len = *((u32 *)(cs->header + 6));//获取数据长度
				if (cs->message_len < 1 || cs->message_len > MAX_MESSAGE_LEN)
				{
					LOG_ERROR("NetworkInterface::handle_request erron: message_len:%d\n", 
						cs->message_len);
					printf("NetworkInterface::handle_request erron: message_len:%d\n", 
						cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}
				LOG_DEBUG("cs->message_len: %d", cs->message_len);
				LOG_DEBUG("cs->read_header_len: %d", cs->read_header_len);
				cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
				cs->read_buf = new char[cs->message_len];
				//cs->read_buf = NULL;
				cs->read_message_len = 0;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request -Invalid request from:%s\n", 
					cs->remote_ip);
				
				//直接关闭请求，不给予任何相应，防止客户端恶意试探
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
			
		}
		
	}
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && 
		evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, 
			cs->message_len - cs->read_message_len);
		cs->read_message_len += len;
		LOG_DEBUG("读取信息：NetworkInterface::handle_request: bufferevnet_read: %d byte\n, read_message_len: %d, message_len:%d\n",
			len, cs->read_message_len, cs->message_len);
		

		if (cs->read_message_len == cs->message_len)
		{
			LOG_DEBUG("cs->read_message_len:%d", cs->read_message_len);
			LOG_DEBUG("cs->message_len:%d\n", cs->message_len);
			
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;
			//解析收到的数据，生成响应的请求事件			
			iEvent *ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);
			//释放cs->read_buf的空间
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
			cs->read_message_len = 0;

			if (ev)
			{			
				ev->set_args(cs);
				DispatchMsgService::getInstance()->enqueue(ev);
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", 
					cs->remote_ip, cs->eid);
				printf("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", 
					cs->remote_ip, cs->eid);
				//直接关闭请求，不给予任何相应，防止客户端恶意试探
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	
	}
		
}

void NetworkInterface::handle_response(bufferevent * bev, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_response。。。");
}

//超时，连接关闭，读写出错等异常情况指定调用的出错回调函数
void NetworkInterface::handle_error(bufferevent * bev, short event, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_error。。。\n");

	ConnectSession *cs = (ConnectSession *)(arg);
	if (event & BEV_EVENT_EOF)
	{
		LOG_DEBUG("client[%s][%p] connection closed.\n", cs->remote_ip, cs->bev);
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))
	{
		LOG_WARN("client[%s][%p] reading timeout", cs->remote_ip, cs->bev);
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))
	{
		LOG_WARN("NetworkInterface::writting timeout ...\
				 client ip: %s\n", cs->remote_ip);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG_WARN("NetworkInterface::other some error ...\
				 client ip: %s\n", cs->remote_ip);
	}
	DispatchMsgService::decUserFromList(bev);
	//关闭该客户端bev, 和cs读写缓冲区
	bufferevent_free(bev);
	session_free(cs);
}

void NetworkInterface::network_event_dispatch()
{
	event_base_loop(base_, EVLOOP_NONBLOCK);
	//处理响应事件，回复响应消息
	DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_message(ConnectSession * cs)
{
	if (cs->response == nullptr)
	{
		bufferevent_free(cs->bev);
		if (cs->request)
		{
			delete cs->request;
			cs->request = nullptr;
		}
		session_free(cs);
	}
	else
	{
		if (cs->bev)
		{			
			LOG_DEBUG("序列化数据成功!");
 			if (cs->eid == EEVENTID_BROADCAST_MSG_RSP)
			{
				std::forward_list<struct bufferevent*> userList= DispatchMsgService::getUserList();
				LOG_DEBUG("正在广播响应中......");
				for (auto iter = userList.begin(); iter != userList.end(); iter++)
				{
					LOG_DEBUG("broadcast response to client: %p", *iter);
					bufferevent_write(*iter, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
				}								
				LOG_DEBUG("广播响应完成！");
			}
			else 
			{
				LOG_DEBUG("正在回应客户端cs->bev[%p],message[%s]......", cs->bev, cs->write_buf);
				bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
				LOG_DEBUG("回应客户端成功！");
			}
		}
		else
			LOG_ERROR("cs->bev is empty! cann't send to client\n, NetworkInterface.cpp, line[304]");
	
		if(cs)	
			session_reset(cs);
	}
}
