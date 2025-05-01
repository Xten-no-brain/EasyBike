#ifndef NS_EVENT_H_
#define NS_EVENT_H_

// 功能：定义事件基类 iEvent，包含事件ID、序列号及序列化方法。

#include "glo_def.h"
#include "eventtype.h"
#include <string>

class iEvent
{
public:
	iEvent(u32 eid, u32 sn); // 构造函数，初始化事件ID和序列号

	virtual ~iEvent();
	virtual std::ostream& dump(std::ostream& out) const { return out; };
	virtual i32 Bytesize() { return 0; }
	virtual bool SerializeToArray(char* buf, int len) { return true; };

	u32 generateSeqNo(); // 生成唯一序列号
	u32 get_eid() const { return eid_; };
	u32 get_sn() const { return sn_; };
	void* get_args() const { return args_; };

	void set_args(void* args) { args_ = args; };
	void set_eid(u32 eid) { eid_ = eid; };

private:
	u32 eid_;
	u32 sn_;
	void* args_;
};


#endif // !NS_EVENT_H_

