/******************************************************************************
**                                                                           **
** Jonathan Beare (2014) - (2014)                                            **
** All Rights Reserved.                                                      **
**                                                                           **
** NOTICE:  All information contained herein is, and remains the property of **
** Jonathan Beare and his suppliers, if any.  The intellectual and technical **
** concepts contained herein are proprietary to Jonathan Beare and his       **
** suppliers and may be covered by U.S. and Foreign Patents, patents in      **
** process, and are protected by trade secret or copyright law.              **
** Dissemination of this information or reproduction of this material is     **
** strictly forbidden unless prior written permission is obtained from       **
** Jonathan Beare.                                                           **
**                                                                           **
******************************************************************************/

#pragma once

#include <map>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>

class ConfigurationManager {
public:
	int Get(std::string Key) {
		boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
		return m_configurationMap[Key];
	}

	void Set(std::string Key, int Value) {
		boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
		m_configurationMap[Key] = Value;
	}

	void Clear() {
		boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
		m_configurationMap.clear();
	}

	void Lock() {
		m_mutex.lock();
	}

	void Unlock() {
		m_mutex.unlock();
	}

private:
	std::map<std::string, int> m_configurationMap;
	boost::recursive_mutex m_mutex;
};

extern ConfigurationManager G_CONFIG;

#ifndef UNIT_TESTS_ENABLED

#define UT_STAT_INCREMENT(_NAME_)
#define UT_STAT_DECREMENT(_NAME_)
#define UT_STAT_ADD(_NAME_, _INC_)
#define UT_STAT_SUB(_NAME_, _INC_)
#define UT_STAT_COUNT(_NAME_)
#define UT_STAT_RESET(_NAME_)
#define UT_STAT_RESET_ALL()

#else

#define UT_STAT_INCREMENT(_NAME_) {G_CONFIG.Lock(); G_CONFIG.Set(_NAME_, G_CONFIG.Get(_NAME_) + 1); G_CONFIG.Unlock();}
#define UT_STAT_DECREMENT(_NAME_) {G_CONFIG.Lock(); G_CONFIG.Set(_NAME_, G_CONFIG.Get(_NAME_) - 1); G_CONFIG.Unlock();}
#define UT_STAT_ADD(_NAME_, _INC_) {G_CONFIG.Lock(); G_CONFIG.Set(_NAME_, G_CONFIG.Get(_NAME_) + _INC_); G_CONFIG.Unlock();}
#define UT_STAT_SUB(_NAME_, _INC_) {G_CONFIG.Lock(); G_CONFIG.Set(_NAME_, G_CONFIG.Get(_NAME_) - _INC_); G_CONFIG.Unlock();}
#define UT_STAT_COUNT(_NAME_) G_CONFIG.Get(_NAME_)
#define UT_STAT_RESET(_NAME_) G_CONFIG.Set(_NAME_, 0)
#define UT_STAT_RESET_ALL() G_CONFIG.Clear()

#endif
