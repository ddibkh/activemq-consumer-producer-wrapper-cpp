/** 
	@file		DSAMQTask.h
	@brief		for received message process interface
	            must use implemented this interface class when use DSAMQConsumer
				business logic class must extends this class
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#ifndef _DS_AMQ_TASK_H_
#define _DS_AMQ_TASK_H_

#include <string>

enum ACK_RESULT
{
	QUEUE_REMOVE = 0,
	QUEUE_RETRY = 1
};

class CDSAMQTask
{
public:
	CDSAMQTask() {};
	virtual ~CDSAMQTask() {};
	virtual ACK_RESULT process(const std::string &strMessage) = 0;
	void set_task_id(const int &nTaskID)
	{
		m_nTaskID = nTaskID;
	}

protected:
	int m_nTaskID;
};
#endif
