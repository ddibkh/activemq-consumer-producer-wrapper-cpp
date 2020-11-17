/** 
	@file		DSAMQProducer.h
	@brief		wrapper class for activemq producer message
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/01/30
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

#ifndef _DS_AMQ_PRODUCER_H_
#define _DS_AMQ_PRODUCER_H_

#include <decaf/lang/Runnable.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/cmsutil/PooledSession.h>
#include <string>

class CDSAMQProducer
{
public:
	CDSAMQProducer();
	CDSAMQProducer(const std::string &brokerURI, 
				   const std::string &destURI, 
				   bool useTopic = false, 
				   bool clientAck = true);
	virtual ~CDSAMQProducer();

	void setURIInfo(const std::string &brokerURI, 
					const std::string &destURI,
					bool useTopic = false,
					bool clientAck = true);

	void close();
	bool sendmessage(activemq::cmsutil::PooledSession* session, const std::string &strMessage);
	std::string getlasterror();

	//index position of DSAMQProducerPool 
	void setIndex(const unsigned int &nposition);
	unsigned int getIndex();


//prevent copy class
private:
	CDSAMQProducer(const CDSAMQProducer &push);
	CDSAMQProducer& operator=(const CDSAMQProducer &push);
	void cleanup();
	
private:
	cms::Destination* m_destination;
    bool m_busetopic;
    bool m_bclientack;
    std::string m_strBrokerURI;
    std::string m_strDestURI;
	std::string m_strError;
	unsigned int m_nposition;
};
#endif
