/** 
	@file		DSAMQConsumerPool.h
	@brief		wrapper class for simple management consumer thread, inherited ThreadPoolExecutor
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#ifndef _DS_AMQ_CONSUMER_POOL_H_
#define _DS_AMQ_CONSUMER_POOL_H_

#include <decaf/util/concurrent/ThreadPoolExecutor.h>
#include <decaf/util/concurrent/TimeUnit.h>
#include <decaf/util/concurrent/LinkedBlockingQueue.h>
#include <decaf/util/concurrent/RejectedExecutionException.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <cms/CMSException.h>
#include <activemq/cmsutil/SessionPool.h>
#include <activemq/cmsutil/ResourceLifecycleManager.h>

#include "DSAMQStopper.h"
#include "DSAMQConsumer.h"
#include "DSAMQTask.h"

#include <set>
#include <memory>

#define TM       template< class CDSAMQTask >
#define MGR      CDSAMQConsumerPool< CDSAMQTask >

using namespace cms;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace decaf::lang::exceptions;
using namespace activemq::cmsutil;
using namespace std;


TM class CDSAMQConsumerPool : public decaf::util::concurrent::ThreadPoolExecutor
{
public:
	//namespace 를 지정했지만 어느 위치에 해당 Class 가 존재하는지 보기 위해서 namespace 는 full로 기재함.
	CDSAMQConsumerPool(int corePoolSize, 
					int maxPoolSize, 
					long long keepAliveTime,
					const decaf::util::concurrent::TimeUnit& unit,
					decaf::util::concurrent::BlockingQueue<decaf::lang::Runnable*>* workQueue);

	CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const decaf::util::concurrent::TimeUnit& unit,
                    decaf::util::concurrent::BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    decaf::util::concurrent::RejectedExecutionHandler* handler);

	CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const decaf::util::concurrent::TimeUnit& unit,
                    decaf::util::concurrent::BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    decaf::util::concurrent::ThreadFactory* threadFactory);

	CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const decaf::util::concurrent::TimeUnit& unit,
                    decaf::util::concurrent::BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    decaf::util::concurrent::ThreadFactory* threadFactory,
                    decaf::util::concurrent::RejectedExecutionHandler* handler);

	virtual ~CDSAMQConsumerPool();

	//minimum thread pool size 10
	static bool StartProcess(const std::string& brokerURI,
					const std::string &destURI,
					std::string &rstrError,
					bool useTopic = false,
					bool clientAck = true,
					const int &nTimeout = 3000,
					const int &nPoolSize = 10);

	static void StopProcess();
	
protected:
	//override ThreadPoolExecutor afterExecute
	/* this function is called when thread task job is completed */
	virtual void afterExecute(decaf::lang::Runnable* task, decaf::lang::Throwable* error);

private:
	CDSAMQConsumerPool(const CDSAMQConsumerPool &);
	CDSAMQConsumerPool& operator=(const CDSAMQConsumerPool &);
	static std::set< CLS* > m_sTask;
	static MGR* m_threadPool;
	static std::string m_strBrokerURI;
	static std::string m_strDestURI;
	static bool m_bUseTopic;
	static bool m_bClientAck;
	static int m_nTimeout;
	/* 리소스 해제는 ResourceLifecycleManager 에서 해준다 */
	static activemq::cmsutil::ResourceLifecycleManager* m_resourcemgr;
	static std::shared_ptr<activemq::cmsutil::SessionPool> m_sessionpool;
	static cms::Connection* m_connection;
	static int m_next_taskid;
};

TM set< CLS* > MGR::m_sTask;
TM MGR* MGR::m_threadPool = NULL;
TM string MGR::m_strBrokerURI = "";		//failover:(tcp://127.0.0.1:61616?connection.dispatchAsync=false)
TM string MGR::m_strDestURI = "";		//queuename?consumer.prefetchSize=1
TM bool MGR::m_bUseTopic = false;
TM bool MGR::m_bClientAck = true;
TM int MGR::m_nTimeout = 3000;
TM ResourceLifecycleManager* MGR::m_resourcemgr = NULL;
TM shared_ptr<activemq::cmsutil::SessionPool> MGR::m_sessionpool;
TM Connection* MGR::m_connection = NULL;
TM int MGR::m_next_taskid = 0;

TM MGR::CDSAMQConsumerPool(int corePoolSize, 
                    int maxPoolSize, 
                    long long keepAliveTime,
                    const TimeUnit& unit,
                    BlockingQueue<decaf::lang::Runnable*>* workQueue) :
                    ThreadPoolExecutor(corePoolSize, maxPoolSize,
                    keepAliveTime, unit, workQueue)
{

}

TM MGR::CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const TimeUnit& unit,
                    BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    RejectedExecutionHandler* handler) :
                    ThreadPoolExecutor(corePoolSize, maxPoolSize, 
                    keepAliveTime, unit, workQueue, handler)
{
	
}

TM MGR::CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const TimeUnit& unit,
                    BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    ThreadFactory* threadFactory) :
                    ThreadPoolExecutor(corePoolSize, maxPoolSize, keepAliveTime,
                    unit, workQueue, threadFactory)
{
	
}

TM MGR::CDSAMQConsumerPool(int corePoolSize, int maxPoolSize,
                    long long keepAliveTime, const TimeUnit& unit,
                    BlockingQueue<decaf::lang::Runnable*>* workQueue,
                    ThreadFactory* threadFactory,
                    RejectedExecutionHandler* handler) :
                    ThreadPoolExecutor(corePoolSize, maxPoolSize, keepAliveTime,
                    unit, workQueue, threadFactory, handler)
{
	
}

TM MGR::~CDSAMQConsumerPool()
{
	
}

TM bool MGR::StartProcess(const std::string &brokerURI,
						const std::string &destURI,
						std::string &rstrError,
						bool useTopic/*=false*/,
						bool clientAck/*=true*/,
						const int &nTimeout/*=3000*/,
						const int &nPoolSize/*=10*/)
{
	//alerady started
	if( !m_sTask.empty() )
        return true;

	if( brokerURI.empty() || destURI.empty() )
	{
		rstrError = "fail to StartProcess, broker URI or destination URI empty";
		return false;
	}
	m_strBrokerURI = brokerURI;
	m_strDestURI = destURI;

	m_bUseTopic = useTopic;
	m_bClientAck = clientAck;
	m_nTimeout = nTimeout;

	int nsize = nPoolSize;
	if( nsize > 50 )
		nsize = 50;

	try
	{
		shared_ptr<ActiveMQConnectionFactory> factory(new ActiveMQConnectionFactory(m_strBrokerURI));
		m_connection = factory->createConnection();

		//setting resend count unlimited
		ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>(m_connection);
		if( amqConnection )
		{
			RedeliveryPolicy* rp = amqConnection->getRedeliveryPolicy();
			if( !rp )
			{
				rstrError = "fail to StartProcess, get redelivery policy object null";
				return false;
			}

			//setting resend count unlimited
			rp->setMaximumRedeliveries(-1);
			//setting resend time delay 3 seconds
			rp->setRedeliveryDelay(3000);
			//setting initial resent time delay 3 seconds
			rp->setInitialRedeliveryDelay(3000);

		}
		//broker 에 연결이 될때까지 대기.
		m_connection->start();

		m_resourcemgr = new ResourceLifecycleManager();
		if( !m_resourcemgr )
		{
			rstrError = "fail to StartProcess, create resourcemanager";
			return false;
		}

		m_resourcemgr->addConnection(m_connection);

		cms::Session::AcknowledgeMode mode = cms::Session::CLIENT_ACKNOWLEDGE;
		if( !m_bClientAck )
			mode = cms::Session::AUTO_ACKNOWLEDGE;
		
		m_sessionpool.reset(new SessionPool(m_connection, mode, m_resourcemgr));
		m_threadPool = new CDSAMQConsumerPool(nsize, nsize, 5, TimeUnit::SECONDS, new LinkedBlockingQueue<Runnable*>());

		for( int i = 0; i < nsize; i++ )
		{
			CLS *task = new CLS(m_sessionpool, m_strDestURI, i, m_bUseTopic, m_bClientAck, m_nTimeout);
			m_sTask.insert(task);
			/*
				execute 두번째 인자 값 (ownership)
				false 로 지정하게 되면 task 작업이 완료되는 시점에 자동으로 
				task 의 메모리가 해제되지 않는다. 즉 이 클래스에서 완료 시점에 
				메모리를 해제해야 한다.
				true(default) 로 지정하게 되면 task 작업이 완료되는 시점에 자동으로
				task 객체의 메모리가 해제된다. 즉 이 클래스에서 완료 시점에 메모리를
				해제하려고 하면 안된다. (메모리 참조 오류)
			*/
			//m_threadPool->execute(task, false);
			m_threadPool->execute(task);
		}

		m_next_taskid = nsize;
	}
	catch( IllegalArgumentException &e )
	{
		rstrError = "Illegalargument exception, " + e.getMessage();
		return false;
	}
	catch( NullPointerException &e )
	{
		rstrError = "NullPointerException exception, " + e.getMessage();
		return false;
	}
	catch( RejectedExecutionException &e )
	{
		rstrError = "RejectedExecutionException exception, " + e.getMessage();
		return false;
	}
	catch( CMSException &e )
	{
		rstrError = "exception, " + e.getMessage();
		return false;
	}

	return true;
}

TM void MGR::StopProcess()
{
	//all tasks stop
	CDSAMQStopper::instance()->stop();

	//shutdown TharedPoolExecutor
	if( m_threadPool )
	{
		m_threadPool->shutdown();

		//all tasks stop
		CDSAMQStopper::instance()->stop();
		try
		{
			if( !m_threadPool->awaitTermination(60, TimeUnit::SECONDS) )
			{
				printf("in stopprocess, not yet termination running tasks... re wait 60 seconds\n");
				m_threadPool->shutdownNow();
				if( !m_threadPool->awaitTermination(60, TimeUnit::SECONDS) )
					printf("in stopprocess, not yet termination running tasks...\n");
				else
					printf("in stopprocess, complete termination running tasks\n");
			}
			else
				printf("in stopprocess, complete termination running tasks\n");
		}
		catch( InterruptedException &e )
		{
			printf("in stopprocess, exception occured, %s\n", e.getMessage().c_str());
		}

		delete m_threadPool;
		m_threadPool = NULL;
	}

	m_sTask.clear();

	if( m_resourcemgr )
		delete m_resourcemgr;

	m_sessionpool.reset();
}

TM void MGR::afterExecute(Runnable* task, Throwable* error)
{
	//서비스 전체 종료가 아닌 개별 task(CDSAMQConsumer) 스레드가 종료된 경우 처리.
	//set 에서 task 객체를 제거 후 새롭게 task 를 생성하여 threadpool 에 추가한다.
	//m_threadPool 객체의 shutdown() 이 호출되면 isShutdown() 은 true 를 리턴한다.
	if( m_threadPool && !m_threadPool->isShutdown() )
	{
		/*
		ThreadPoolExecutor 의 execute 호출시 두번째 파라미터인 ownership 값을
		false 로 지정한 경우 파라미터로 전달되는 task 는 CDSAMQConsumer* 가 아닌
		UnownedTaskWrapper* 이기 때문에 dynamic_cast 를 수행하면 NULL 이 리턴된다.
		즉, 이 클래스에서 사용하는 ThreadPoolExecutor::execute 호출의 ownership 값은
		false 로 지정하면 안된다.
		execute 호출의 ownership 이 true 인 경우 ThreadPoolExecutor 에서 완료된 task
		스레드 객체는 자동으로 delete 처리를 하기 때문에 여기서는 set 에서 제거만 해준다.
		*/
		CLS* ac = dynamic_cast<CLS*>(task);
		if( ac )
		{
			//exception occured in task
			if( error != NULL )
			{
				;
			}
			
			typename set< CLS* >::iterator iter;
			iter = m_sTask.find(ac);

			if( iter != m_sTask.end() )
			{
				//remove task thread object in set<> memory
				m_sTask.erase(iter);

				//create new task thread object and add threadpool
				CLS *th = new CLS(m_sessionpool, m_strDestURI, m_next_taskid++, m_bUseTopic, m_bClientAck, m_nTimeout);
				m_sTask.insert(th);

				try
				{
					m_threadPool->execute(th);
				}
				catch( NullPointerException &e )
				{
					//printf("nullpointer exception, %s\n", e.getMessage().c_str());
					; //write log
				}
				catch( RejectedExecutionException &e )
				{
					//printf("rejectexecution exception, %s\n", e.getMessage().c_str());
					; //write log
				}
				catch( CMSException &e )
				{
					//printf("exception, %s\n", e.getMessage().c_str());
					; //write log
				}
			}
		}
	}
}

#endif
