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

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "Utility.h"

class SimpleEvent : public boost::enable_shared_from_this<SimpleEvent> {
public:
	enum EventType{
		Connected,
		Disconnected,
		Read_Completed,
		Write_Completed
	};

	static boost::shared_ptr<SimpleEvent> Create(EventType Type) {
		return boost::shared_ptr<SimpleEvent>(new SimpleEvent(Type));
	}

	boost::shared_ptr<SimpleEvent> GetShared() {
		return boost::dynamic_pointer_cast<SimpleEvent>(shared_from_this());
	}

	EventType GetEventType() {
		return m_eventType;
	}

	virtual ~SimpleEvent() {
		UT_STAT_INCREMENT("SimpleEvent");
	};

protected:
	SimpleEvent(EventType Type) : m_eventType(Type) {
		UT_STAT_DECREMENT("SimpleEvent");
	};

private:
	SimpleEvent& operator=(SimpleEvent const &) = delete;
	SimpleEvent(SimpleEvent const &) = delete;

	EventType m_eventType;
};

class SimpleObject : public boost::enable_shared_from_this<SimpleObject> {
public:
	static boost::shared_ptr<SimpleObject> Create(boost::shared_ptr<SimpleObject> const &Parent) {
		return boost::shared_ptr<SimpleObject>(new SimpleObject(Parent));
	}

	boost::shared_ptr<SimpleObject> GetShared() {
		return boost::dynamic_pointer_cast<SimpleObject>(shared_from_this());
	}

	boost::shared_ptr<SimpleObject> GetParent() {
		return m_parent;
	}

	void SetParent(boost::shared_ptr<SimpleObject> const &Parent) {
		m_parent = Parent;
	}

	virtual ~SimpleObject() {
		UT_STAT_INCREMENT("SimpleObject");
	};

protected:
	SimpleObject(boost::shared_ptr<SimpleObject> const &Parent) : m_parent(Parent) {
		UT_STAT_DECREMENT("SimpleObject");
	};

	virtual void HandleEvent(boost::shared_ptr<SimpleEvent> const &Event) {
		if(m_parent) {
			m_parent->HandleEvent(Event);
		}
	};

private:
	SimpleObject& operator=(SimpleObject const &) = delete;
	SimpleObject(SimpleObject const &) = delete;

	boost::shared_ptr<SimpleObject> m_parent;
};
