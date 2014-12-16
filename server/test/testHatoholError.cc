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

#include <cppcutter.h>
#include "HatoholError.h"
using namespace std;

namespace testHatoholError {

void cut_setup(void)
{
}

void cut_teardown(void)
{
}

// ---------------------------------------------------------------------------
// Test cases
// ---------------------------------------------------------------------------
void test_defaultValue(void)
{
	HatoholError err;
	cppcut_assert_equal(HTERR_UNINITIALIZED, err.getCode());
	cppcut_assert_equal(true, err.getOptionMessage().empty());
}

void test_getOptMessage(void)
{
	string optMsg = "Option message.";
	HatoholError err(HTERR_OK, optMsg);
	cppcut_assert_equal(optMsg, err.getOptionMessage());
}

void test_operatorEqual(void)
{
	HatoholError err(HTERR_OK);
	cppcut_assert_equal(true, err == HTERR_OK);
}

void test_operatorNotEqual(void)
{
	HatoholError err;
	cppcut_assert_equal(true, err != HTERR_OK);
}

void test_operatorSubst(void)
{
	HatoholError err(HTERR_UNKNOWN_REASON, "Option message.");
	err =  HTERR_OK;
	cppcut_assert_equal(HTERR_OK, err.getCode());
	cppcut_assert_equal(true, err.getOptionMessage().empty());
}

} // namespace testHatoholError
