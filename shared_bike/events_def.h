#ifndef BRKS_COMMON_EVENTS_DEF_H_
#define BRKS_COMMON_EVENTS_DEF_H_

// 事件类定义 ：定义了各种事件类，如 MobileCodeReqEv、MobileCodeRspEv 等，
// 代表了不同的客户端请求和服务器响应。
// 事件 ID 和编码解码方法 ：每个事件类都有一个唯一的事件 ID，用于标识事件类型，
// 并且提供了消息的编码和解码方法。

#include "ievent.h"
#include <string>
#include "bike.pb.h"

// 定义了 MobileCodeReqEv 类，表示获取手机验证码的请求事件
// 使用 Protocol Buffers 的消息格式来存储事件数据
// 提供了获取用户名的方法、生成事件大小的方法和序列化消息的方法
class MobileCodeReqEv :public iEvent
{
public:
	MobileCodeReqEv(const std::string userName) :
		iEvent(EEVENTID_GET_MOBILE_CODE_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
	};

	const std::string& get_userName() { return msg_.username(); };
	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};


private:
	tutorial::mobile_request msg_;
};

class MobileCodeRspEv : public iEvent
{
public:
	MobileCodeRspEv(i32 resCode, i32 verCode) :
		iEvent(EEVENTID_GET_MOBILE_CODE_RSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(resCode); // 代号
		msg_.set_vercode(verCode); // 验证码
		msg_.set_data(getReasonByErrorCode(resCode));
	}

	const i32 get_resCode() { return msg_.rescode(); };
	const i32 get_verCode() { return msg_.vercode(); };
	const std::string& get_data() { return msg_.data(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::mobile_response msg_;
};

class ExitRspEv: public iEvent
{
public:
	ExitRspEv() :iEvent(EEVNETID_EXIT_RSP, iEvent::generateSeqNo()) {};

private:

};

// 用户注册请求
class RegisterReqEv : public iEvent
{
public:
	RegisterReqEv(const std::string userName, 
		std::string password, const i32 verCode) :
		iEvent(EEVENTID_REGISTER_USER_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_userpwd(password);
		msg_.set_vercode(verCode);
	}

	const std::string& get_userName() { return msg_.username(); };
	const std::string& get_userPwd() { return msg_.userpwd(); };
	const i32 get_verCode() { return msg_.vercode(); };
	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::registerUser_request msg_;
};

// 用户注册响应
class RegisterResEv : public iEvent
{
public:
	RegisterResEv(const std::string userName, 
		std::string password, const i32 resCode) :
		iEvent(EEVENTID_REGISTER_USER_RSP, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_userpwd(password);
		msg_.set_rescode(resCode);
	}

	const std::string& get_userName() { return msg_.username(); };
	const std::string& get_userPwd() { return msg_.userpwd(); };
	
	const i32 get_resCode() { return msg_.rescode(); };
	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::registerUser_response msg_;
};

// 登陆请求
class LoginReqEv: public iEvent
{
public:
	LoginReqEv(const std::string userName, std::string password) :
		iEvent(EEVENTID_LOGIN_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_userpwd(password);
	}
	
	const std::string& get_userName() { return msg_.username(); };
	const std::string& get_userPwd() { return msg_.userpwd(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::login_request msg_;
};
// 登陆响应
class LoginResEv : public iEvent
{
public:
	LoginResEv(std::string& userName, std::string& password, i32 userLevel, i32 resCode) :
		iEvent(EEVENTID_LOGIN_RSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(resCode);
		msg_.set_username(userName);
		msg_.set_userlevel(userLevel);
		msg_.set_userpwd(password);
	}

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::login_response msg_;
};

// 添加单车请求
class AddBikeReqEv : public iEvent
{
public:
	AddBikeReqEv(i32 bikeId, r64 longitude, r64 latitude) :
		iEvent(EEVENTID_ADDBIKE_REQ, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const i32 get_bikeId() { return msg_.bikeid(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::addBike_request msg_;
};

// 添加单车响应
class AddBikeResEv : public iEvent
{
public:
	AddBikeResEv(i32 bikeId, r64 longitude, r64 latitude, i32 resCode) :
		iEvent(EEVENTID_ADDBIKE_RSP, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
		msg_.set_rescode(resCode);
	}

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::addBike_response msg_;
};

// 删除单车请求
class DeleteBikeReqEv : public iEvent
{
public:
	DeleteBikeReqEv(i32 bikeId) :
		iEvent(EEVENTID_DELETEBIKE_REQ, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
	}
	const i32 get_bikeId() { return msg_.bikeid(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::deleteBike_request msg_;
};

// 删除单车响应
class DeleteBikeResEv : public iEvent
{
public:
	DeleteBikeResEv(i32 bikeId, i32 resCode) :
		iEvent(EEVENTID_DELETEBIKE_RSP, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
		msg_.set_rescode(resCode);
	}

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::deleteBike_response msg_;
};

// 账户记录请求
class ListAccountRecordsReqEv : public iEvent
{
public:
	ListAccountRecordsReqEv(std::string userName) :
		iEvent(EEVENTID_LIST_ACCOUNT_RECORDS_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
	}
	const std::string get_userName() { return msg_.username(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize();};
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::list_account_records_request msg_;
};

class ListAccountRecordsResonseEv : public iEvent
{
public:
	ListAccountRecordsResonseEv(i32 resCode, i32 type, 
		i32 consume, uint timestamp) :
		iEvent(EEVENTID_ACCOUNT_RECORDS_RSP, iEvent::generateSeqNo())
	{
		arRes = msg_.add_records();
		
		msg_.set_rescode(resCode);
		msg_.set_desc(getReasonByErrorCode(resCode));
		arRes->set_type(type);
		arRes->set_consume(consume);
		arRes->set_timestamp(timestamp);
	}

	const i32 get_resCode() { return msg_.rescode(); };
	const std::string get_desc() { return msg_.desc(); };
	const i32 get_type() { return arRes->type(); };
	const i32 get_consume() { return arRes->consume(); };
	const uint64_t get_timestamp() { return arRes->timestamp(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::list_account_records_response msg_;
	tutorial::list_account_records_response_account_record* arRes;
};

// 单车扫码开始骑行请求
class bikeScanQRStartReqEv : public iEvent
{
public:
	bikeScanQRStartReqEv(std::string userName, i32 bikeId, uint64_t timestamp, 
		r64 longitude, r64 latitude) :
		iEvent(EEVENTID_SCANQRSTART_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_bikeid(bikeId);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);	
	}


	const std::string& get_userName() { return msg_.username(); };
	const i32 get_bikeId() { return msg_.bikeid(); };
	
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	// 不能用这种类型的返回值引用，内存访问时会崩溃
	// const r64& get_longitude() { return msg_.longitude(); };
	// const r64& get_latitud() { return msg_.latitude(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };
	
	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_scanQR_start_request msg_;
};

// 单车扫码开始骑行响应
class bikeScanQRStartResponseEv : public iEvent
{
public:
	bikeScanQRStartResponseEv(i32 resCode, uint64_t timestamp, 
		r64 longitude, r64 latitude) :
		iEvent(EEVENTID_SCANQRSTART_PSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(resCode);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const i32 get_resCode() { return msg_.rescode(); };
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_scanQR_start_response msg_;
};
// 单车扫码结束骑行请求r64& get_longitude()
class bikeScanQREndReqEv : public iEvent
{
public:
	// 规定经纬度的顺序是为先纬后经，中间用逗号隔开，比如北京的经纬度的书写：
	// 用文字书写是北纬40度，东经116度；用数字和字母书写是：40°N，116°′E
	bikeScanQREndReqEv(std::string userName, i32 bikeId, uint64_t timestamp, 
		r64 longitude, r64 latitude) :
		iEvent(EEVENTID_SCANQREND_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_bikeid(bikeId);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const std::string& get_userName() { return msg_.username(); };
	const i32 get_bikeId() { return msg_.bikeid(); };
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) 
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_scanQR_end_request msg_;
};

// 单车扫码结束骑行响应
class bikeScanQREndResponseEv : public iEvent
{
public:
	bikeScanQREndResponseEv(i32 resCode, uint64_t timestamp, 
		r64 longitude, r64 latitude) :
		iEvent(EEVENTID_SCANQREND_PSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(resCode);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const i32 get_resCode() { return msg_.rescode(); };
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_scanQR_end_response msg_;
};

// 单车扫码故障报修请求
class bikeScanQRFaultReqEv : public iEvent
{
public:
	bikeScanQRFaultReqEv(std::string userName, i32 bikeId, uint64_t timestamp, r64 longitude, r64 latitude) :
		iEvent(EEVENTID_FAULT_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_bike_id(bikeId);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const std::string& get_userName() { return msg_.username(); };
	const i32 get_bikeId() { return msg_.bike_id(); };
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_fault_request msg_;
};

// 单车扫码故障报修响应
class bikeScanQRFaultResponseEv : public iEvent
{
public:
	bikeScanQRFaultResponseEv(i32 code) :
		iEvent(EEVENTID_FAULT_PSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(code);		
	}

	const i32 get_resCode() { return msg_.rescode(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_fault_response msg_;
};

// 单车扫码修复完成请求
class bikeScanQRRepairFinishReqEv : public iEvent
{
public:
	bikeScanQRRepairFinishReqEv(std::string userName, i32 bikeId, 
		uint64_t timestamp, r64 longitude, r64 latitude) :
		iEvent(EEVENTID_REPAIRFINISH_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
		msg_.set_bike_id(bikeId);
		msg_.set_timestamp(timestamp);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const std::string& get_userName() { return msg_.username(); };
	const i32 get_bikeId() { return msg_.bike_id(); };
	const uint64_t get_timestamp() { return msg_.timestamp(); };
	const r64 get_longitude() { return msg_.longitude(); };
	const r64 get_latitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_repair_finish_request msg_;
};

// 单车扫码修复完成响应
class bikeScanQRRepairFinishResponseEv : public iEvent
{
public:
	bikeScanQRRepairFinishResponseEv(i32 code) :
		iEvent(EEVENTID_REPAIRFINISH_PSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(code);
	}

	const i32 get_resCode() { return msg_.rescode(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::bike_repair_finish_response msg_;
};


// 广播请求事件
class broadcastMsgReqEv : public iEvent
{
public:
	broadcastMsgReqEv(i32 bikeId, i32 bikeStatus, r64 longitude, r64 latitude) :
		iEvent(EEVENTID_BROADCAST_MSG_REQ, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
		msg_.set_bikestatus(bikeStatus);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const i32 get_bikeid() { return msg_.bikeid(); };
	const i32 get_bikestatus() { return msg_.bikestatus(); };
	const r64 get_bikeLongitude() { return msg_.longitude(); };
	const r64 get_bikeLatitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::broadcast_msg_request msg_;
};

// 广播响应事件
class broadcastMsgResponseEv : public iEvent
{
public:
	broadcastMsgResponseEv(i32 bikeId, i32 bikeStatus, r64 longitude, r64 latitude) :
		iEvent(EEVENTID_BROADCAST_MSG_RSP, iEvent::generateSeqNo())
	{
		msg_.set_bikeid(bikeId);
		msg_.set_bikestatus(bikeStatus);
		msg_.set_longitude(longitude);
		msg_.set_latitude(latitude);
	}

	const i32 get_bikeid() { return msg_.bikeid(); };
	const i32 get_bikestatus() { return msg_.bikestatus(); };
	const r64 get_bikeLongitude() { return msg_.longitude(); };
	const r64 get_bikeLatitude() { return msg_.latitude(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::broadcast_msg_response msg_;
};

// 单车信息同步请求事件
class bikeInfoReqEv : public iEvent
{
public:
	bikeInfoReqEv(std::string userName) :
		iEvent(EEVENTID_LIST_BIKEINFO_REQ, iEvent::generateSeqNo())
	{
		msg_.set_username(userName);
	}

	const std::string& get_userName() { return msg_.username(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::list_all_bikeinfo_request msg_;
};

// 单车信息同步响应事件
class bikeInfoResponseEv : public iEvent
{
public:
	bikeInfoResponseEv(i32 resCode):
		iEvent(EEVENTID_LIST_BIKEINFO_RSP, iEvent::generateSeqNo())
	{
		msg_.set_rescode(resCode);
	}
	void set_bikeResponse_msg(i32 bikeId, i32 bikeStatus, 
		r64 longitude, r64 latitude);
	const std::string& get_userName() { return msg_.username(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len)
	{
		return msg_.SerializeToArray(buf, len);
	};

private:
	tutorial::list_all_bikeinfo_response msg_;
};

#endif // !BRKS_COMMON_EVENTS_DEF_H_

