#ifndef _DS_AMQ_STOPPER_H_
#define _DS_AMQ_STOPPER_H_

#include "DSRTSingleton.h"

class CDSAMQStopper : public CDSRTSingleton<CDSAMQStopper>
{
	friend class CDSRTSingleton<CDSAMQStopper>;

private:
	bool m_bRunning;

protected:
	CDSAMQStopper() : m_bRunning(true)
	{

	}

	virtual ~CDSAMQStopper()
	{

	}

public:
	inline void stop()
	{
		m_bRunning = false;
	}

	inline bool isrun()
	{
		return m_bRunning;
	}
};
#endif
