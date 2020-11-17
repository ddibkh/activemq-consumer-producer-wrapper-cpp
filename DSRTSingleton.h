/** 
	@file		DSRTSingleton.h
	@brief		singleton object
	@author		ddibkh
	@warning	.
	@version	v1.0
	@date		2020/02/05
*/

#ifndef _DS_RT_SINGLETON_H_
#define _DS_RT_SINGLETON_H_

template < class T >
class CDSRTSingleton
{
protected:
	CDSRTSingleton() { };
	virtual ~CDSRTSingleton() { };

public:
	static T* instance()
	{
		if( m_pInstance == NULL )
			m_pInstance = new T;

		return m_pInstance;
	}

	static void release()
	{
		if( m_pInstance )
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

private:
	static T* m_pInstance;
	CDSRTSingleton(const CDSRTSingleton &);
	CDSRTSingleton& operator=(const CDSRTSingleton &);
};

template< class T >
T* CDSRTSingleton<T>::m_pInstance = 0;
#endif
