/*
 * Copyright (C) 2013-2014 Project Hatohol
 *
 * This file is part of Hatohol.
 *
 * Hatohol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License, version 3
 * as published by the Free Software Foundation.
 *
 * Hatohol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Hatohol. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <Logger.h>
#include <Mutex.h>
using namespace mlpl;

#include <sstream>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>

#include "ConfigManager.h"
#include "ArmZabbixAPI.h"
#include "JSONParser.h"
#include "JSONBuilder.h"
#include "DataStoreException.h"
#include "ItemEnum.h"
#include "DBTablesMonitoring.h"
#include "UnifiedDataStore.h"
#include "HatoholDBUtils.h"
#include "HostInfoCache.h"
#include "ThreadLocalDBCache.h"

using namespace std;

static const uint64_t NUMBER_OF_GET_EVENT_PER_ONCE  = 1000;

struct ArmZabbixAPI::Impl
{
	const ServerIdType zabbixServerId;
	HostInfoCache      hostInfoCache;

	// constructors
	Impl(const MonitoringServerInfo &serverInfo)
	: zabbixServerId(serverInfo.id)
	{
	}
};

class connectionException : public HatoholException {};

// ---------------------------------------------------------------------------
// Public methods
// ---------------------------------------------------------------------------
ArmZabbixAPI::ArmZabbixAPI(const MonitoringServerInfo &serverInfo)
: ArmBase("ArmZabbixAPI", serverInfo),
  m_impl(new Impl(serverInfo))
{
	setMonitoringServerInfo(serverInfo);
}

ArmZabbixAPI::~ArmZabbixAPI()
{
	requestExitAndWait();
}

void ArmZabbixAPI::onGotNewEvents(const ItemTablePtr &itemPtr)
{
	// This function is used on a test class.
}

// ---------------------------------------------------------------------------
// Protected methods
// ---------------------------------------------------------------------------
ItemTablePtr ArmZabbixAPI::updateTriggers(void)
{
	UnifiedDataStore *uds = UnifiedDataStore::getInstance();
	SmartTime last = uds->getTimestampOfLastTrigger(m_impl->zabbixServerId);
	const int requestSince = last.getAsTimespec().tv_sec;
	return getTrigger(requestSince);
}

ItemTablePtr ArmZabbixAPI::updateTriggerExpandedDescriptions(void)
{
	UnifiedDataStore *uds = UnifiedDataStore::getInstance();
	SmartTime last = uds->getTimestampOfLastTrigger(m_impl->zabbixServerId);
	const int requestSince = last.getAsTimespec().tv_sec;
	return getTriggerExpandedDescription(requestSince);
}

void ArmZabbixAPI::updateItems(void)
{
	ItemTablePtr items = getItems();
	ItemTablePtr applications = getApplications(items);
	makeHatoholItems(items, applications);
}

void ArmZabbixAPI::updateHosts(void)
{
	ItemTablePtr hostTablePtr, hostsGroupsTablePtr;
	getHosts(hostTablePtr, hostsGroupsTablePtr);
	makeHatoholHosts(hostTablePtr);
	makeHatoholMapHostsHostgroups(hostsGroupsTablePtr);
}

void ArmZabbixAPI::updateEvents(void)
{
	const EventIdType serverLastEventId = getEndEventId(false);
	if (serverLastEventId == EVENT_ID_NOT_FOUND) {
		MLPL_DBG("Last event ID is not found in Zabbix server\n");
		return;
	}

	ThreadLocalDBCache cache;
	const EventIdType dbLastEventId =
	  cache.getMonitoring().getLastEventId(m_impl->zabbixServerId);
	MLPL_DBG("The last event ID in Hatohol DB: %" FMT_EVENT_ID "\n", dbLastEventId);
	EventIdType eventIdFrom = dbLastEventId == EVENT_ID_NOT_FOUND ?
				  (ConfigManager::getInstance()->getLoadOldEvents() ?
				   getEndEventId(true) : serverLastEventId) :
	                          dbLastEventId + 1;

	while (eventIdFrom <= serverLastEventId) {
		const EventIdType eventIdTill =
		  eventIdFrom + NUMBER_OF_GET_EVENT_PER_ONCE - 1;
		ItemTablePtr eventsTablePtr =
		  getEvents(eventIdFrom, eventIdTill);
		makeHatoholEvents(eventsTablePtr);
		onGotNewEvents(eventsTablePtr);

		eventIdFrom = eventIdTill + 1;
	}
}

void ArmZabbixAPI::updateApplications(void)
{
	// getHosts() tries to get all hosts when an empty vector is passed.
	static const vector<uint64_t> appIdVector;
	ItemTablePtr tablePtr = getApplications(appIdVector);
}

void ArmZabbixAPI::updateGroups(void)
{
	ItemTablePtr groupsTablePtr;
	getGroups(groupsTablePtr);
	makeHatoholHostgroups(groupsTablePtr);
}

//
// virtual methods
//
gpointer ArmZabbixAPI::mainThread(HatoholThreadArg *arg)
{
	const MonitoringServerInfo &svInfo = getServerInfo();
	MLPL_INFO("started: ArmZabbixAPI (server: %s)\n",
	          svInfo.hostName.c_str());
	ArmBase::registerAvailableTrigger(COLLECT_NG_PARSER_ERROR,
					  FAILED_PARSER_JSON_DATA_TRIGGER_ID,
					  HTERR_FAILED_TO_PARSE_JSON_DATA);
	ArmBase::registerAvailableTrigger(COLLECT_NG_DISCONNECT_ZABBIX,
					  FAILED_CONNECT_ZABBIX_TRIGGER_ID,
					  HTERR_FAILED_CONNECT_ZABBIX);
	ArmBase::registerAvailableTrigger(COLLECT_NG_INTERNAL_ERROR,
					  FAILED_INTERNAL_ERROR_TRIGGER_ID,
					  HTERR_INTERNAL_ERROR);
	return ArmBase::mainThread(arg);
}

void ArmZabbixAPI::makeHatoholTriggers(ItemTablePtr triggers)
{
	ThreadLocalDBCache cache;
	TriggerInfoList triggerInfoList, expandedTriggerInfoList, mergedTriggerInfoList;
	ItemTablePtr expandedDescription;
	HatoholDBUtils::transformTriggersToHatoholFormat(
	  triggerInfoList, triggers, m_impl->zabbixServerId,
	  m_impl->hostInfoCache);
	expandedDescription = updateTriggerExpandedDescriptions();
	HatoholDBUtils::transformTriggerExpandedDescriptionToHatoholFormat(
	  expandedTriggerInfoList, expandedDescription);
	HatoholDBUtils::mergePlainTriggersListAndExpandedDescriptionList(
	  triggerInfoList, expandedTriggerInfoList, mergedTriggerInfoList);
	cache.getMonitoring().addTriggerInfoList(mergedTriggerInfoList);
}

void ArmZabbixAPI::makeHatoholEvents(ItemTablePtr events)
{
	EventInfoList eventInfoList;
	HatoholDBUtils::transformEventsToHatoholFormat(
	  eventInfoList, events, m_impl->zabbixServerId);

	UnifiedDataStore *dataStore = UnifiedDataStore::getInstance();
	dataStore->addEventList(eventInfoList);
}

void ArmZabbixAPI::makeHatoholItems(
  ItemTablePtr items, ItemTablePtr applications)
{
	ThreadLocalDBCache cache;
	DBTablesMonitoring &dbMonitoring = cache.getMonitoring();
	ItemInfoList itemInfoList;
	MonitoringServerStatus serverStatus;
	serverStatus.serverId = m_impl->zabbixServerId;
	HatoholDBUtils::transformItemsToHatoholFormat(
	  itemInfoList, serverStatus, items, applications);
	dbMonitoring.addItemInfoList(itemInfoList);
	dbMonitoring.addMonitoringServerStatus(&serverStatus);
}

void ArmZabbixAPI::makeHatoholHostgroups(ItemTablePtr groups)
{
	ThreadLocalDBCache cache;
	HostgroupInfoList groupInfoList;
	HatoholDBUtils::transformGroupsToHatoholFormat(groupInfoList, groups,
	                                               m_impl->zabbixServerId);
	cache.getMonitoring().addHostgroupInfoList(groupInfoList);
}

void ArmZabbixAPI::makeHatoholMapHostsHostgroups(ItemTablePtr hostsGroups)
{
	ThreadLocalDBCache cache;
	HostgroupElementList hostgroupElementList;
	HatoholDBUtils::transformHostsGroupsToHatoholFormat(
	  hostgroupElementList, hostsGroups, m_impl->zabbixServerId);
	cache.getMonitoring().addHostgroupElementList(hostgroupElementList);
}

void ArmZabbixAPI::makeHatoholHosts(ItemTablePtr hosts)
{
	ThreadLocalDBCache cache;
	HostInfoList hostInfoList;
	HatoholDBUtils::transformHostsToHatoholFormat(hostInfoList, hosts,
	                                              m_impl->zabbixServerId);
	cache.getMonitoring().updateHosts(hostInfoList, m_impl->zabbixServerId);

	// TODO: consider if DBClientHatohol should have the cache
	HostInfoListConstIterator hostInfoItr = hostInfoList.begin();
	for (; hostInfoItr != hostInfoList.end(); ++hostInfoItr)
		m_impl->hostInfoCache.update(*hostInfoItr);
}

uint64_t ArmZabbixAPI::getMaximumNumberGetEventPerOnce(void)
{
	return NUMBER_OF_GET_EVENT_PER_ONCE;
}

ArmBase::ArmPollingResult ArmZabbixAPI::handleHatoholException(
  const HatoholException &he)
{
	if (he.getErrCode() == HTERR_FAILED_CONNECT_ZABBIX) {
		clearAuthToken();
		MLPL_ERR("Error Connection: %s %d\n",
			 he.what(), he.getErrCode());
		return COLLECT_NG_DISCONNECT_ZABBIX;
	} else if (he.getErrCode() == HTERR_FAILED_TO_PARSE_JSON_DATA) {
		MLPL_ERR("Error Message parse: %s %d\n",
			 he.what(), he.getErrCode());
		return COLLECT_NG_PARSER_ERROR;
	}
	MLPL_ERR("Error on update: %s %d\n", he.what(), he.getErrCode());
	return COLLECT_NG_INTERNAL_ERROR;;
}

//
// This function just shows a warning if there is missing host ID.
//
ArmBase::ArmPollingResult ArmZabbixAPI::mainThreadOneProc(void)
{
	if (!updateAuthTokenIfNeeded())
		return COLLECT_NG_DISCONNECT_ZABBIX;

	try {
		ItemTablePtr triggers = updateTriggers();
		updateHosts();
		updateGroups();
		makeHatoholTriggers(triggers);
		updateEvents();

		if (!getCopyOnDemandEnabled())
			updateItems();
	} catch (const HatoholException &he) {
		return handleHatoholException(he);
	}

	return COLLECT_OK;
}

ArmBase::ArmPollingResult ArmZabbixAPI::mainThreadOneProcFetchItems(void)
{
	if (!updateAuthTokenIfNeeded())
		return COLLECT_NG_DISCONNECT_ZABBIX;

	try {
		updateItems();
	} catch (const HatoholException &he) {
		return handleHatoholException(he);
	}
	return COLLECT_OK;
}

ArmBase::ArmPollingResult ArmZabbixAPI::mainThreadOneProcFetchHistory(
  HistoryInfoVect &historyInfoVect,
  const ItemInfo &itemInfo, const time_t &beginTime, const time_t &endTime)
{
	if (!updateAuthTokenIfNeeded())
		return COLLECT_NG_DISCONNECT_ZABBIX;

	try {
		ItemTablePtr itemTablePtr = getHistory(
		  itemInfo.id,
		  ZabbixAPI::fromItemValueType(itemInfo.valueType),
		  beginTime, endTime);
		HatoholDBUtils::transformHistoryToHatoholFormat(
		  historyInfoVect, itemTablePtr, m_impl->zabbixServerId);
	} catch (const HatoholException &he) {
		return handleHatoholException(he);
	}
	return COLLECT_OK;
}

// ---------------------------------------------------------------------------
// Private methods
// ---------------------------------------------------------------------------
