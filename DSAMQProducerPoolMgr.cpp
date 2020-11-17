/** 
	@file		CDSAMQProducerPoolMgr.cpp
	@brief		wrapper class for producer object memory object pool
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#include "DSAMQProducerPoolMgr.h"
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <decaf/lang/Thread.h>

using namespace cms;
using namespace activemq::cmsutil;
using namespace activemq::core;

//brokerURI : failover://(tcp://127.0.0.1:61616)?startupMaxReconnectAttempts=0&timeout=3000&connection.sendTimeout=5000
CDSAMQProducerPoolMgr::CDSAMQProducerPoolMgr() : 
	m_strBrokerURI(""),
	m_strDestURI(""),
	m_bUseTopic(false),
	m_bClientAck(true),
	m_connection(NULL), 
	m_resourcemgr(NULL), 
	m_strError(""), 
	m_bStartedConnection(false)
{

}

CDSAMQProducerPoolMgr::~CDSAMQProducerPoolMgr()
{
	ReleaseResource();
}

void CDSAMQProducerPoolMgr::ReleaseResource()
{
	if( m_resourcemgr )
	{
		delete m_resourcemgr;
		m_resourcemgr = NULL;
	}
}

std::string CDSAMQProducerPoolMgr::GetBrokerURI() const
{
	return m_strBrokerURI;
}

std::string CDSAMQProducerPoolMgr::GetDestURI() const
{
	return m_strDestURI;
}

//use client ack mode
//use queue (not use topic)
bool CDSAMQProducerPoolMgr::CreatePool(unsigned int nPoolSize,
									const std::string &strBrokerURI,
									const std::string &strDestURI)
{
	if( m_pool.IsCreated() )
	{
		m_strError = "in CreatePool, already created pool";
		return true;
	}

	if( strBrokerURI.empty() || strDestURI.empty() )
	{
		m_strError = "in CreatePool, broker or destination URI empty";
		return false;
	}

	if( nPoolSize > 50 )
		nPoolSize = 50;

	m_strBrokerURI = strBrokerURI;
	m_strDestURI = strDestURI;

	try
	{
		m_pool.CreatePool(nPoolSize, m_strBrokerURI, m_strDestURI, m_bUseTopic, m_bClientAck);
	}
	catch(CDSAMQProducerPool<CDSAMQProducer> &e)
	{
		m_strError = "in CreatePool, exception occured, ";
		m_strError += e.GetLastError();
		return false;
	}

	return true;
}

bool CDSAMQProducerPoolMgr::CreatePool(unsigned int nPoolSize,
									const std::string &strBrokerURI,
									const std::string &strDestURI,
									const bool &bUseTopic,
									const bool &bClientAck)
{
	if( m_pool.IsCreated() )
	{
		m_strError = "in CreatePool, already created pool";
		return true;
	}

	if( nPoolSize > 50 )
		nPoolSize = 50;

	m_strBrokerURI = strBrokerURI;
	m_strDestURI = strDestURI;
	m_bUseTopic = bUseTopic;
	m_bClientAck = bClientAck;

	try
	{
		m_pool.CreatePool(nPoolSize, m_strBrokerURI, m_strDestURI, m_bUseTopic, m_bClientAck);		
	}
	catch(CDSAMQProducerPool<CDSAMQProducer> &e)
	{
		m_strError = "in CreatePool, exception occured, ";
		m_strError += e.GetLastError();
		return false;
	}
	
	return true;
}

bool CDSAMQProducerPoolMgr::CreateSessionPool()
{
	if( !m_pool.IsCreated() )
	{
		m_strError = "in CreateSessionPool, not created producer pool, ";
		m_strError += "must previous create producer pool, call CreatePool()";
		return false;
	}

	if( !m_bStartedConnection )
	{
		if( m_connection )
		{
			m_connection->close();
			delete m_connection;
			m_connection = NULL;
		}

		try
		{
			std::shared_ptr<ActiveMQConnectionFactory> factory(new ActiveMQConnectionFactory(m_strBrokerURI));
			m_connection = factory->createConnection();
			try
			{
				m_connection->start();
				m_bStartedConnection = true;
			}
			catch(CMSException &e)
			{
				printf("connection start, %s\n", e.getMessage().c_str());
				if( m_connection )
				{
					delete m_connection;
					m_connection = NULL;
				}
				return false;
			}
			if( m_resourcemgr )
			{
				delete m_resourcemgr;
				m_resourcemgr = NULL;
			}

			m_resourcemgr = new ResourceLifecycleManager();
			if( !m_resourcemgr )
			{
				m_strError = "fail to CreateSessionPool, create resourcemanager";
				if( m_connection )
				{
					m_connection->close();
					delete m_connection;
					m_connection = NULL;
				}
				return false;
			}

			m_resourcemgr->addConnection(m_connection);

			Session::AcknowledgeMode mode = Session::CLIENT_ACKNOWLEDGE;
			if( !m_bClientAck )
				mode = Session::AUTO_ACKNOWLEDGE;

			m_sessionpool.reset(new SessionPool(m_connection, mode, m_resourcemgr));
			m_bStartedConnection = true;
		}
		catch(CMSException &e)
		{
			m_strError = "fail to CreateSessionPool, create connection exception, ";
			m_strError += e.getMessage();

			if( m_connection )
			{
				if( m_bStartedConnection )
				{
					m_connection->close();
					m_bStartedConnection = false;
				}
				delete m_connection;
				m_connection = NULL;
				return false;
			}
		}
	}

	return true;
}

std::string CDSAMQProducerPoolMgr::getlasterror()
{
	return m_strError;
}

bool CDSAMQProducerPoolMgr::sendmessage(const std::string &strMessage)
{
	if( !m_pool.IsCreated() )
	{
		m_strError = "fail to sendmessage, not created object pool";
		return false;
	}

	if( !m_bStartedConnection )
	{
		
		bool bRet = false;
		{
			std::lock_guard<std::mutex> lock_guard(m_mutex);
			bRet = CreateSessionPool();
		}

		if( !bRet )
		{
			//decaf::lang::Thread::sleep(1000);
			return false;
		}
	}

	PooledSession* session = NULL;
	try
	{
		//get pool in session pool
		session = m_sessionpool->takeSession();
		if( session == NULL )
		{
			m_strError = "in sendmessage, fail to get session, NULL";
			return false;	
		}
	}
	catch(CMSException &e)
	{
		m_strError = "in sendmessage, fail to get session, ";
		m_strError += e.getMessage();
		return false;
	}

	CDSAMQProducer *producer = NULL;
	{
		std::lock_guard<std::mutex> lock_guard(m_mutex);
		producer = m_pool.CreatePoolItem();
	}
	
	if( !producer )
	{
		m_strError = "fail to sendmessage, ";
		m_strError += m_pool.GetLastError();
		//return session object to session pool
		session->close();
		return false;
	}

	bool bSent = true;
	if( !producer->sendmessage(session, strMessage) )
	{
		m_strError = "fail to sendmessage, ";
		m_strError += producer->getlasterror();
		bSent = false;
	}

	//return session object to session pool
	session->close();

	std::lock_guard<std::mutex> lock_guard(m_mutex);
	m_pool.FreePoolItem(producer);

	return bSent;
}
