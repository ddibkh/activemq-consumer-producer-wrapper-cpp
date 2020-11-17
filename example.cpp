/**
	@file		example.cpp
	@brief		activemq producer/consumer 테스트 샘플 코드.
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#include <stdio.h>
#include <activemq/library/ActiveMQCPP.h>
#include "DSAMQProducer.h"
#include "DSAMQProducerPoolMgr.h"
#include "DSAMQConsumerPool.h"
#include "DSAMQConsumer.h"
#include <string>
#include <string.h>

using namespace activemq;
using namespace decaf::util::concurrent;
using namespace std;

class ProducerThread : public Runnable
{
public:
	ProducerThread(int nStart) : m_nStart(nStart)
	{

	}
	virtual ~ProducerThread() {};
	void run()
	{
		int nMessageCnt = 10;
		char message[1024] = {0};
		for( int i = 0; i < nMessageCnt; i++ )
		{
			memset(message, 0, sizeof(message));
			sprintf(message, "send message #%d", m_nStart + i);
			string strMessage(message);
			if( !CDSAMQProducerPoolMgr::instance()->sendmessage(strMessage) )
				printf("fail to send message, %s, %s\n", strMessage.c_str(), CDSAMQProducerPoolMgr::instance()->getlasterror().c_str());
		}
	}

private:
	int m_nStart;
};

//반드시 DSAMQTask 를 상속 받아야 한다.
class CDSPrintTask : public CDSAMQTask
{
public:
	CDSPrintTask() : CDSAMQTask()
	{

	}

	virtual ~CDSPrintTask() {};

	ACK_RESULT process(const string &strMessage)
	{
		printf("taskid : [%d], [%X] reecived message : %s\n", m_nTaskID, reinterpret_cast<unsigned int>(this), strMessage.c_str());
		return QUEUE_REMOVE;
	}
};

int main( int argc, char**argv )
{
	if( argc != 3 )
	{
		printf("Usage : %s producer|consumer threadcount\n", argv[0]);
		printf("producer send threadcount * 10 messages\n");
		printf("cousumer received message in threadcount threads\n");
		return 0;
	}

	activemq::library::ActiveMQCPP::initializeLibrary();

	const int nThreadCnt = atoi(argv[2]);

	//producer
	if( memcmp(argv[1], "producer", strlen(argv[1])) == 0 )
	{
		string strBrokerURI = "failover://(tcp://127.0.0.1:61616)?startupMaxReconnectAttempts=0&timeout=3000&connection.sendTimeout=5000";
		string strDestURI = "TEST.FOO";

		if( !CDSAMQProducerPoolMgr::instance()->CreatePool(nThreadCnt, strBrokerURI, strDestURI) )
		{
			printf("%s\n", CDSAMQProducerPoolMgr::instance()->getlasterror().c_str());
			return -1;
		}

		if( !CDSAMQProducerPoolMgr::instance()->CreateSessionPool() )
			printf("fail to create session pool, %s\n", CDSAMQProducerPoolMgr::instance()->getlasterror().c_str());
		else
			printf("success to create session pool\n");
		int offset = 0;
		int i = 0;

		
		vector< ProducerThread > vProducer;
		vector< shared_ptr<Thread> > vThread;
		for( ; i < nThreadCnt; i++ )
		{
			ProducerThread p(offset);
			vProducer.push_back(p);
			offset += 10;
		}

		for( i = 0; i < nThreadCnt; i++ )
		{
			shared_ptr<Thread> th(new Thread(&vProducer[i]));
			vThread.push_back(th);
		}

		for( i = 0; i < nThreadCnt; i++ )
		{
			vThread[i]->start();
		}

		for( i = 0; i < nThreadCnt; i++ )
		{
			vThread[i]->join();
		}
		
		CDSAMQProducerPoolMgr::instance()->ReleaseResource();
	}
	//consumer
	else if( memcmp(argv[1], "consumer", strlen(argv[1])) == 0 )
	{
		string rstrError = "";
		string strBrokerURI = "failover:(tcp://127.0.0.1:61616?connection.dispatchAsync=false&jms.redeliveryPolicy.initialRedeliveryDelay=0&jms.redeliveryPolicy.maximumRedeliveries=-1&jms.redeliveryPolicy.redeliveryDelay=5000)";
		string strDestURI = "TEST.FOO";
		if( !CDSAMQConsumerPool< CDSPrintTask >::StartProcess(strBrokerURI, strDestURI, rstrError, false, true, 3000, nThreadCnt) )
		{
			printf("fail to start consumer, %s\n", rstrError.c_str());
		}
		else
		{
			printf("success to start consumer\n");
		}

		std::cout << "Press 'q' to quit" << std::endl;
    	while( std::cin.get() != 'q') {}
		CDSAMQConsumerPool< CDSPrintTask >::StopProcess();
	}
	else
	{
		printf("Usage : %s producer|consumer", argv[0]);
	}

	activemq::library::ActiveMQCPP::shutdownLibrary();
    std::cout << "end process " << string(argv[1]) << std::endl;

	return 0;
}
