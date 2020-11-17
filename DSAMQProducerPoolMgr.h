/** 
	@file		DSAMQProducerPoolMgr.h
	@brief		wrapper class for simple management producer object pool
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/06
*/

#ifndef _DS_AMQ_PRODUCER_POOL_MGR_H_
#define _DS_AMQ_PRODUCER_POOL_MGR_H_

#include "DSAMQProducerPool.h"
#include "DSRTSingleton.h"
#include <activemq/cmsutil/SessionPool.h>
#include <activemq/cmsutil/ResourceLifecycleManager.h>
#include <mutex>

class CDSAMQProducerPoolMgr : public CDSRTSingleton<CDSAMQProducerPoolMgr>
{
	friend class CDSRTSingleton<CDSAMQProducerPoolMgr>;

protected:
	CDSAMQProducerPoolMgr();
	virtual ~CDSAMQProducerPoolMgr();

public:
	//max 50
	bool CreatePool(unsigned int nPoolSize, 
				const std::string &strBrokerURI,
				const std::string &strDestURI);
	bool CreatePool(unsigned int nPoolSize,
				const std::string &strBrokerURI,
				const std::string &strDestURI,
				const bool &bUseTopic,
				const bool &bClientAck);

	bool CreateSessionPool();

	void ReleaseResource();

	std::string GetBrokerURI() const;
	std::string GetDestURI() const;

	bool sendmessage(const std::string &strMessage);

	std::string getlasterror();

private:
	std::string m_strBrokerURI;
	std::string m_strDestURI;
	bool m_bUseTopic;
	bool m_bClientAck;
	cms::Connection* m_connection;
	activemq::cmsutil::ResourceLifecycleManager* m_resourcemgr;
	std::string m_strError;
	bool m_bStartedConnection;	//flag of connection started && create session pool
	std::shared_ptr<activemq::cmsutil::SessionPool> m_sessionpool;
	CDSAMQProducerPool<CDSAMQProducer> m_pool;
	std::mutex m_mutex;
};
#endif
