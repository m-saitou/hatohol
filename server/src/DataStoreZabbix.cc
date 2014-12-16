/*
 * Copyright (C) 2013 Project Hatohol
 *
 * This file is part of Hatohol.
 *
 * Hatohol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3
 * as published by the Free Software Foundation.
 *
 * Hatohol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hatohol. If not, see <http://www.gnu.org/licenses/>.
 *
 * Additional permission under GNU GPL version 3 section 7
 *
 * If you modify this program, or any covered work, by linking or
 * combining it with the OpenSSL project's OpenSSL library (or a
 * modified version of that library), containing parts covered by the
 * terms of the OpenSSL or SSLeay licenses, Project Hatohol
 * grants you additional permission to convey the resulting work.
 * Corresponding Source for a non-source form of such a combination
 * shall include the source code for the parts of OpenSSL used as well
 * as that of the covered work.
 */

#include <cstdio>
#include "DataStoreZabbix.h"
using namespace std;

// ---------------------------------------------------------------------------
// Public methods
// ---------------------------------------------------------------------------
DataStoreZabbix::DataStoreZabbix(
  const MonitoringServerInfo &serverInfo, const bool &autoStart)
: m_armApi(serverInfo)
{
	if (autoStart)
		m_armApi.start();
}

DataStoreZabbix::~DataStoreZabbix(void)
{
}

const MonitoringServerInfo &DataStoreZabbix::getMonitoringServerInfo(void)
  const
{
	return m_armApi.getServerInfo();
}

const ArmStatus &DataStoreZabbix::getArmStatus(void) const
{
	return m_armApi.getArmStatus();
}

void DataStoreZabbix::setCopyOnDemandEnable(bool enable)
{
	m_armApi.setCopyOnDemandEnabled(enable);
}

bool DataStoreZabbix::isFetchItemsSupported(void)
{
	return m_armApi.isFetchItemsSupported();
}

void DataStoreZabbix::startOnDemandFetchItem(Closure0 *closure)
{
	m_armApi.fetchItems(closure);
}

void DataStoreZabbix::startOnDemandFetchHistory(
  const ItemInfo &itemInfo, const time_t &beginTime, const time_t &endTime,
  Closure1<HistoryInfoVect> *closure)
{
	m_armApi.fetchHistory(itemInfo, beginTime, endTime, closure);
}

// ---------------------------------------------------------------------------
// Protected methods
// ---------------------------------------------------------------------------
