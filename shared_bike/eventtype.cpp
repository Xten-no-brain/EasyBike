// 功能：实现错误码与错误描述的映射，提供通过错误码获取错误信息的接口。

#include "eventtype.h"

// 静态错误码-原因映射表
static EErrorReason EER[] = 
{
	{ERRC_SUCCESS, "OK."},
	{ERRC_INVALID_MSG, "Invalid message."},
	{ERRC_INVALID_DATA, "Invalid data."},
	{ERRC_METHOD_NOT_ALLOWED, "Method not allowed."},
	{ERRO_PROCCESS_FALED, "Proccess failed."},
	{ERRO_BIKE_IS_TOOK, "Bike is took."},
	{ERRO_BIKE_IS_RUNNING, "Bike is running."},
	{ERRO_BIKE_IS_DAMAGED, "Bike is damaged."},
	{ERRO_NULL, "Undefined"} // 结束标记
};

// 根据错误码获取对应的错误描述
const char * getReasonByErrorCode(i32 code)
{
	i32 i = 0;
	for (i = 0; EER[i].code != ERRO_NULL; i++) // 遍历数组直到结束标记
	{
		if (EER[i].code == code)
		{
			return EER[i].reason;
		}
	}
	return EER[i].reason; // 返回未定义错误
}
