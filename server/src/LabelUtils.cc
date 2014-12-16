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

#include "LabelUtils.h"
#include "StringUtils.h"
using namespace std;
using namespace mlpl;

// ---------------------------------------------------------------------------
// Public methods
// ---------------------------------------------------------------------------
string LabelUtils::getEventTypeLabel(EventType eventType)
{
	switch (eventType) {
	case EVENT_TYPE_GOOD:
		return "GOOD";
	case EVENT_TYPE_BAD:
		return "BAD";
	case EVENT_TYPE_UNKNOWN:
		return "Unknown";
	default:
		return StringUtils::sprintf("Invalid: %d", eventType);
	}
}

string LabelUtils::getTriggerStatusLabel(TriggerStatusType status)
{
	switch (status) {
	case TRIGGER_STATUS_OK:
		return "OK";
	case TRIGGER_STATUS_PROBLEM:
		return "PROBLEM";
	default:
		return StringUtils::sprintf("Invalid: %d", status);
	}
}

string LabelUtils::getTriggerSeverityLabel(TriggerSeverityType severity)
{
	switch (severity) {
	case TRIGGER_SEVERITY_UNKNOWN:
		return "UNKNWON";
	case TRIGGER_SEVERITY_INFO:
		return "INFO";
	case TRIGGER_SEVERITY_WARNING:
		return "WARNING";
	case TRIGGER_SEVERITY_ERROR:
		return "ERROR";
	case TRIGGER_SEVERITY_CRITICAL:
		return "CRITICAL";
	case TRIGGER_SEVERITY_EMERGENCY:
		return "EMERGENCY";
	default:
		return StringUtils::sprintf("Invalid: %d", severity);
	}
}
