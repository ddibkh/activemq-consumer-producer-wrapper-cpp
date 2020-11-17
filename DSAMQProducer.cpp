/** 
	@file		DSAMQProducer.cpp
	@brief		wrapper class for activemq producer implementation
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/01/30
*/

#include "DSAMQProducer.h"
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>

using namespace activemq;
using namespace activemq::cmsutil;
using namespace cms;
using namespace std;

CDSAMQProducer::CDSAMQProducer() : 
	m_destination(NULL),
	m_busetopic(false),
	m_bclientack(true),
	m_strBrokerURI(""),
	m_strDestURI(""),
	m_strError(""),
	m_nposition(string::npos)
{

}

CDSAMQProducer::CDSAMQProducer(const std::string &brokerURI, 
							   const std::string &destURI, 
							   bool useTopic /*=false*/, 
							   bool clientAck /*=true*/) :
	m_destination(NULL),
	m_busetopic(useTopic),
	m_bclientack(clientAck),
	m_strBrokerURI(brokerURI),
	m_strDestURI(destURI),
	m_strError(""),
	m_nposition(string::npos)
{

}

CDSAMQProducer::~CDSAMQProducer()
{
	cleanup();
}

void CDSAMQProducer::setIndex(const unsigned int &nposition)
{
	m_nposition = nposition;
}

unsigned int CDSAMQProducer::getIndex()
{
	return m_nposition;
}

void CDSAMQProducer::close()
{
	this->cleanup();
}

void CDSAMQProducer::setURIInfo(
					const std::string &brokerURI, 
					const std::string &destURI,
					bool useTopic/*= false*/,
					bool clientAck /*= true*/)
{
	m_strBrokerURI = brokerURI;
	m_strDestURI = destURI;
	m_busetopic = useTopic;
	m_bclientack = clientAck;
}

string CDSAMQProducer::getlasterror()
{
	return m_strError;
}

void CDSAMQProducer::cleanup()
{
	m_strError = "producer cleanup";
	try
	{
		if( m_destination != NULL ) 
			delete m_destination;
	}
	catch ( CMSException& e )
	{
		m_strError += "|";
		m_strError = e.getMessage();
	}
	m_destination = NULL;

	m_strError += "|";
	m_strError += "complete";
}

bool CDSAMQProducer::sendmessage(PooledSession* session, const string &strMessage)
{
	if( session == NULL )
	{
		m_strError = "in sendmessage, session object NULL";
		return false;
	}

	if( strMessage.empty() )
	{
		m_strError = "in sendmessage, message is emtpy";
		return false;
	}

	try 
	{
		if( m_destination == NULL )
		{
			if( m_busetopic )
				m_destination = session->createTopic( m_strDestURI );
			else
				m_destination = session->createQueue( m_strDestURI );
		}

		MessageProducer* producer = NULL;
		// Create a MessageProducer from the Session to the Topic or Queue
		//cachedproducer in managed in session pool
		producer = session->createCachedProducer( m_destination );
		producer->setDeliveryMode( DeliveryMode::PERSISTENT );
		//producer->setTimeToLive(18400000); //expire term in broker queue
		
		// Create a messages
		TextMessage *message = NULL;
		try
		{
			message = session->createTextMessage(strMessage);
		}
		catch( CMSException &e)
		{
			m_strError = "in sendmessage, fail to create message, " + e.getMessage();
			if( message )
				delete message;
			return false;
		}

		try
		{
			producer->send(message);
			if( message )
				delete message;

			printf("[%X] %s\n", reinterpret_cast<unsigned int>(this), strMessage.c_str());
		}
		catch(CMSException &e)
		{
			m_strError = "in sendmessage, fail to send message, " + e.getMessage();
			if( message )
				delete message;
			return false;
		}
	}
	catch ( CMSException& e ) 
	{
		m_strError = "in sendmessage, exception occured, " + e.getMessage();
		return false;
	}

	return true;
}
