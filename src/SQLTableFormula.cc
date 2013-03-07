/* Asura
   Copyright (C) 2013 MIRACLE LINUX CORPORATION
 
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SQLTableFormula.h"
#include "SQLProcessorException.h"
#include "AsuraException.h"

// ---------------------------------------------------------------------------
// SQLTableFormula
// ---------------------------------------------------------------------------
SQLTableFormula::~SQLTableFormula()
{
}

int SQLTableFormula::getColumnIndexOffset(const string &tableName)
{
	MLPL_BUG("Not implemented: %s\n", __PRETTY_FUNCTION__);
	return  COLUMN_INDEX_OFFSET_NOT_FOUND;
}

void SQLTableFormula::addTableSizeInfo(const string &tableName,
                                       size_t numColumns)
{
	MLPL_BUG("Not implemented: %s\n", __PRETTY_FUNCTION__);
}

// ---------------------------------------------------------------------------
// SQLTableElement
// ---------------------------------------------------------------------------
SQLTableElement::SQLTableElement(const string &name, const string &varName)
: m_name(name),
  m_varName(varName)
{
}

const string &SQLTableElement::getName(void) const
{
	return m_name;
}

const string &SQLTableElement::getVarName(void) const
{
	return m_varName;
}

void SQLTableElement::setItemTable(ItemTablePtr itemTablePtr)
{
	m_itemTablePtr = itemTablePtr;
}

ItemTablePtr SQLTableElement::getTable(void)
{
	return m_itemTablePtr;
}

// ---------------------------------------------------------------------------
// SQLTableJoin
// ---------------------------------------------------------------------------
SQLTableJoin::SQLTableJoin(SQLJoinType type)
: m_type(type),
  m_leftFormula(NULL),
  m_rightFormula(NULL)
{
}

SQLTableFormula *SQLTableJoin::getLeftFormula(void) const
{
	return m_leftFormula;
}

SQLTableFormula *SQLTableJoin::getRightFormula(void) const
{
	return m_rightFormula;
}

void SQLTableJoin::setLeftFormula(SQLTableFormula *tableFormula)
{
	m_leftFormula = tableFormula;;
}

void SQLTableJoin::setRightFormula(SQLTableFormula *tableFormula)
{
	m_rightFormula = tableFormula;;
}

// ---------------------------------------------------------------------------
// SQLTableCrossJoin
// ---------------------------------------------------------------------------
SQLTableCrossJoin::SQLTableCrossJoin(void)
: SQLTableJoin(SQL_JOIN_TYPE_CROSS)
{
}

ItemTablePtr SQLTableCrossJoin::getTable(void)
{
	SQLTableFormula *leftFormula = getLeftFormula();
	SQLTableFormula *rightFormula = getRightFormula();
	if (!leftFormula || !rightFormula) {
		THROW_SQL_PROCESSOR_EXCEPTION(
		  "leftFormula (%p) or rightFormula (%p) is NULL.\n",
		  leftFormula, rightFormula);
	}
	return crossJoin(leftFormula->getTable(), rightFormula->getTable());
}

// ---------------------------------------------------------------------------
// SQLTableInnerJoin
// ---------------------------------------------------------------------------
SQLTableInnerJoin::SQLTableInnerJoin
  (const string &leftTableName, const string &leftColumnName,
   const string &rightTableName, const string &rightColumnName,
   SQLColumnIndexResoveler *resolver)
: SQLTableJoin(SQL_JOIN_TYPE_INNER),
  m_leftTableName(leftTableName),
  m_leftColumnName(leftColumnName),
  m_rightTableName(rightTableName),
  m_rightColumnName(rightColumnName),
  m_indexLeftJoinColumn(INDEX_NOT_SET),
  m_indexRightJoinColumn(INDEX_NOT_SET),
  m_columnIndexResolver(resolver)
{
}

ItemTablePtr SQLTableInnerJoin::getTable(void)
{
	if (!m_columnIndexResolver)
		THROW_ASURA_EXCEPTION("m_columnIndexResolver: NULL");

	if (m_indexLeftJoinColumn == INDEX_NOT_SET) {
		m_indexLeftJoinColumn =
		  m_columnIndexResolver->getIndex(m_leftTableName,
		                                  m_leftColumnName);
	}
	if (m_indexRightJoinColumn == INDEX_NOT_SET) {
		m_indexRightJoinColumn =
		  m_columnIndexResolver->getIndex(m_rightTableName,
		                                  m_rightColumnName);
	}

	SQLTableFormula *leftFormula = getLeftFormula();
	SQLTableFormula *rightFormula = getRightFormula();
	if (!leftFormula || !rightFormula) {
		THROW_SQL_PROCESSOR_EXCEPTION(
		  "leftFormula (%p) or rightFormula (%p) is NULL.\n",
		  leftFormula, rightFormula);
	}
	return innerJoin(leftFormula->getTable(), rightFormula->getTable(),
	                 m_indexLeftJoinColumn, m_indexRightJoinColumn);
}

const string &SQLTableInnerJoin::getLeftTableName(void) const
{
	return m_leftTableName;
}

const string &SQLTableInnerJoin::getLeftColumnName(void) const
{
	return m_leftColumnName;
}

const string &SQLTableInnerJoin::getRightTableName(void) const
{
	return m_rightTableName;
}

const string &SQLTableInnerJoin::getRightColumnName(void) const
{
	return m_rightColumnName;
}
