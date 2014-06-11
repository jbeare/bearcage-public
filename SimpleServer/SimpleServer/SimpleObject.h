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
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#define SIMPLE_CLASS_HEADER(CLASS_NAME); \
static boost::shared_ptr<CLASS_NAME> Create() { \
	return boost::shared_ptr<CLASS_NAME>(new CLASS_NAME()); \
} \
boost::shared_ptr<CLASS_NAME> GetSharedPointer() { \
	return boost::dynamic_pointer_cast<CLASS_NAME>(shared_from_this()); \
}

class SimpleObject : public boost::enable_shared_from_this<SimpleObject> {
public:
	SIMPLE_CLASS_HEADER(SimpleObject);

	int WhoAmI() {
		return 0;
	}

	virtual ~SimpleObject() {};

protected:
	SimpleObject() {};
};

class AdvancedObject : public SimpleObject {
public:
	SIMPLE_CLASS_HEADER(AdvancedObject);

	int WhoAmI() {
		return 1;
	}

	virtual ~AdvancedObject() {};

protected:
	AdvancedObject() {};
};
