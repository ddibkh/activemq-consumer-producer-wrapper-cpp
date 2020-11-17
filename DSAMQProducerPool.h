/** 
	@file		DSAMQProducerPool.h
	@brief		wrapper class for producer object memory object pool
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#ifndef _DS_AMQ_PRODUCER_POOL_H_
#define _DS_AMP_PRODUCER_POOL_H_

#include "DSAMQProducer.h"
#include <bitset>
#include <limits.h>
#include <string>

#define MAX_BITSET_SIZE		0xFFFF*CHAR_BIT

template <typename CDSAMQProducer>
class CDSAMQProducerPool
{
public:
	CDSAMQProducerPool() : 
		m_nPoolCursor(0), m_nPoolSize(0), m_nUsedPoolItemCnt(0), m_pool(NULL), m_bIsCreated(false), m_nBitSetSize(0)
	{
		m_bits.reset();
	}

	virtual ~CDSAMQProducerPool()
	{
		DestroyPool();
	}

	bool CreatePool(unsigned int nPoolSize, 
				const std::string &strBrokerURI, 
				const std::string &strDestURI)
	{
		return CreatePool(nPoolSize, strBrokerURI, strDestURI, false, true);
	}

	bool CreatePool(unsigned int nPoolSize, 
				const std::string &strBrokerURI, 
				const std::string &strDestURI,
				bool bUseTopic,
				bool bClientAck)
	{
		m_nPoolSize = nPoolSize;
		m_nBitSetSize = ((m_nPoolSize / CHAR_BIT) + 1 ) * 8;

		if ( m_nBitSetSize > MAX_BITSET_SIZE )
		{
			m_strError = "In CreatePool(), max bitset size is over.";
			return false;
		}

		try
		{
			m_pool = new CDSAMQProducer[m_nPoolSize];
		}
		catch ( std::bad_alloc &err )
		{
			m_strError = "In CreatePool(), ";
			m_strError += err.what();
			throw *this;
		}
		
		// Pre initialize pool URI info
		for ( unsigned int i = 0; i < m_nPoolSize; i++ )
			m_pool[i].setURIInfo(strBrokerURI, strDestURI, bUseTopic, bClientAck);

		m_bIsCreated = true;
		return true;
	}

	void DestroyPool()
	{
		if ( m_pool )
			delete [] m_pool;

		m_pool = NULL;
		m_nPoolSize = 0;
		m_nPoolCursor = 0;
		m_nUsedPoolItemCnt = 0;
		m_bits.reset();
	}

	//현재 idle 상태인 pool 객체 리턴
	CDSAMQProducer* CreatePoolItem()
	{
		std::string::size_type nPos = std::string::npos;
		for( std::string::size_type i = 0; i < m_nBitSetSize; ++i )
		{
			if( !m_bits.test(i) )
			{
				nPos = i;
				break;
			}
		}

		if ( nPos >= m_nPoolSize )
		{
			m_strError = "In CreatePoolItem(), Pool Overflow.";
			return NULL;
		}

		m_pool[nPos].setIndex(nPos);
		//bit to on (status used)
		m_bits.set(nPos, true);
		m_nUsedPoolItemCnt++;
		return ( &(m_pool[nPos]) );
	}

	//on 상태인 pool 객체를 idle 상태로 변경
	void FreePoolItem(CDSAMQProducer *producer)
	{
		if( producer == NULL )
			return;
		
		unsigned int nPoolIdx = producer->getIndex();

		if ( nPoolIdx >= m_nPoolSize )
			return;

		producer->setIndex(std::string::npos);

		// 2. bit to off (status idle)
		m_bits.set(nPoolIdx, false);

		if ( m_nUsedPoolItemCnt > 0 )
			m_nUsedPoolItemCnt--;
	}

	void InitPoolItemCursor()
	{
		m_nPoolCursor = 0;
	}

	//현재 사용중인 pool 객체 리턴
	CDSAMQProducer* GetNextPoolItem()
	{
		std::string::size_type nPos = std::string::npos;
		for( std::string::size_type i = m_nPoolCursor; i < m_nBitSetSize; ++i )
		{
			if( m_bits.test(i) )
			{
				nPos = i;
				break;
			}
		}

		if ( nPos >= m_nPoolSize )
		{
			m_nPoolCursor = 0;
			return NULL;
		}

		m_nPoolCursor = nPos+1;
		return &(m_pool[nPos]);
	}

	unsigned int GetUsedPoolItemCount()
	{
		return m_nUsedPoolItemCnt;
	}

	unsigned int GetPoolSize()
	{
		return m_nPoolSize;
	}

	std::string GetLastError()
	{
		return m_strError;
	}

	bool IsCreated()
	{
		return m_bIsCreated;
	}
protected:
	unsigned int m_nPoolCursor;
	unsigned int m_nPoolSize;
	unsigned int m_nUsedPoolItemCnt;
	CDSAMQProducer *m_pool;
	bool m_bIsCreated;
	std::string::size_type m_nBitSetSize;
	std::bitset<MAX_BITSET_SIZE> m_bits;
	
private:
	std::string m_strError;
};
#endif
