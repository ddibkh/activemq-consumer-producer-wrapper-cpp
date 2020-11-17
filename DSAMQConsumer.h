/** 
	@file		DSAMQConsumer.h
	@brief		wrapper class for activemq consumer message
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/04
*/

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DS_AMQ_CONSUMER_H_
#define _DS_AMQ_CONSUMER_H_

#include <decaf/lang/Runnable.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/Connection.h>
//#include <cms/Session.h>
#include <activemq/cmsutil/SessionPool.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <string>
#include <memory>

#include "DSAMQTask.h"
#include "DSAMQStopper.h"

#define T          template< class CDSAMQTask >
#define CLS        CDSAMQConsumer< CDSAMQTask >

using namespace activemq;
using namespace activemq::core;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

T class CDSAMQConsumer : public decaf::lang::Runnable
					   //public ExceptionListener
{
public:
	CDSAMQConsumer(std::shared_ptr<activemq::cmsutil::SessionPool> sessionpool,
	               const std::string &destURI,
				   const int &nTaskID,
				   bool useTopic = false,
				   bool clientAck = true,
				   const int &nTimeout = 3000);

	virtual ~CDSAMQConsumer();

	//override Runnable interface
	virtual void run();
	// If something bad happens you see it here as this class is also been
    // registered as an ExceptionListener with the connection.
	//override ExceptionListener
    //virtual void onException(const CMSException& ex AMQCPP_UNUSED);
    //virtual void onException(const decaf::lang::Exception& ex);
	std::string getlasterror();
	int get_task_id();

private:
	//prevent copy object
	CDSAMQConsumer(const CDSAMQConsumer&);
	CDSAMQConsumer& operator=(const CDSAMQConsumer&);
	void cleanup();

private:
	std::shared_ptr<activemq::cmsutil::SessionPool> m_sessionpool;
	cms::Destination *m_destination;
	bool m_busetopic;
	std::string m_strDestURI;
	bool m_bclientack;
	std::string m_strError;
	int m_nTimeout;	//receive timeout milli sec. (default 3000)
	int m_nTaskID;
	CDSAMQTask m_process;
};

//failover:(tcp://127.0.0.1:61616?connection.dispatchAsync=false)
//observer.list.queue?consumer.prefetchSize=1
T CLS::CDSAMQConsumer(std::shared_ptr<activemq::cmsutil::SessionPool> sessionpool,
    const std::string &destURI,
	const int &nTaskID,
	bool useTopic/*=false*/,
	bool clientAck/*=true*/,
	const int &nTimeout/*=3000*/) :
		m_sessionpool(sessionpool),
		m_destination(NULL),
		m_busetopic(useTopic),
		m_strDestURI(destURI),
		m_bclientack(clientAck),
		m_strError(""),
		m_nTimeout(nTimeout),
		m_nTaskID(nTaskID)
{

}

T CLS::~CDSAMQConsumer()
{
	cleanup();
}

T string CLS::getlasterror()
{
	return m_strError;
}

T int CLS::get_task_id()
{
	return m_nTaskID;
}

T void CLS::cleanup()
{
	//*************************************************
	// Always close destination, consumers and producers before
	// you destroy their sessions and connection.
	//*************************************************

	m_strError = "consumer cleanup";
	// Destroy resources.
	try
	{
		if( m_destination != NULL ) 
			delete m_destination;
	}
	catch( CMSException& e )
	{
		m_strError += "|";
		m_strError += e.getMessage();
	}
	m_destination = NULL;

	m_strError += "|";
	m_strError += "complete";
}

T void CLS::run()
{
	activemq::cmsutil::PooledSession* session = m_sessionpool->takeSession();
	m_process.set_task_id(m_nTaskID);

	try
	{
		// Create the destination (Topic or Queue)
		if( m_destination == NULL )
		{
			if( m_busetopic )
				m_destination = session->createTopic(m_strDestURI);
			else
				m_destination = session->createQueue(m_strDestURI);
		}

		// Create a MessageConsumer from the Session to the Topic or Queue
		MessageConsumer* consumer = session->createCachedConsumer(m_destination, "", false);

		if( m_nTimeout <= 0 )
			m_nTimeout = 3000;

		while( CDSAMQStopper::instance()->isrun() )
		{
			try
			{
				shared_ptr<Message> message;
				try
				{
					message.reset(consumer->receive(m_nTimeout));
					if( message.get() == NULL )
						continue;
				}
				catch( CMSException &e)
				{
					m_strError = "receive exception, " + e.getMessage();
					break;
				}
				
				const TextMessage* textMessage = dynamic_cast<const TextMessage*>(message.get());
				string text = "";
				if( textMessage != NULL )
				{
					text = textMessage->getText();
				}
				else
				{
					message->acknowledge();
				}

				/*
					process received message
				*/
				ACK_RESULT AR = m_process.process(text);

				if( m_bclientack ) {
					//remove broker queue message
					if( AR == QUEUE_REMOVE )
						message->acknowledge();
					else
						session->recover();
				}
			} 
			catch (CMSException& e) 
			{
				break;
			}
		}
	}
	catch (CMSException& e) 
	{
		m_strError = "[consumer] in run, exception occured2, " + e.getMessage();
	}

	//return session object to session pool
	session->close();


	return;
}

#endif
