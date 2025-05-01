#include "ievent.h"

iEvent::iEvent(u32 eid, u32 sn)
{
	set_eid(eid);
	printf("!set_eid=%d\n\n");
}

iEvent::~iEvent()
{
}

u32 iEvent::generateSeqNo() // 生成唯一序列号（静态计数器）
{
	static u32 sn = 0;
	return sn++;
}
