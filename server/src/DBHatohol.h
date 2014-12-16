/*
 * Copyright (C) 2014 Project Hatohol
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

#ifndef DBHatohol_h
#define DBHatohol_h

#include "DB.h"

class DBTablesConfig;
class DBTablesHost;
class DBTablesUser;
class DBTablesAction;
class DBTablesMonitoring;

class DBHatohol : public DB {
public:
	static void reset(void);

	/**
	 * Set default DB parameters
	 *
	 * Each of the following parameters can be NULL. In that case,
	 * the paramter is  not updated (the previous value is used)
	 *
	 * @param dbName   A database name.
	 * @param user     A user name used to connect with a DB server.
	 * @param password A password used to connect with a DB server.
	 */
	static void setDefaultDBParams(const char *dbName,
	                               const char *user = NULL,
	                               const char *password = NULL);

	DBHatohol(void);
	virtual ~DBHatohol();
	DBTablesConfig  &getDBTablesConfig(void);
	DBTablesHost    &getDBTablesHost(void);
	DBTablesUser    &getDBTablesUser(void);
	DBTablesAction  &getDBTablesAction(void);
	DBTablesMonitoring &getDBTablesMonitoring(void);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};

#endif // DBHatohol_h
