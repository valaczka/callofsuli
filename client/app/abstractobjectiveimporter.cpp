/*
 * ---- Call of Suli ----
 *
 * objectiveimporter.cpp
 *
 * Created on: 2021. 01. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectiveImporter
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "abstractobjectiveimporter.h"

AbstractObjectiveImporter::AbstractObjectiveImporter(QXlsx::Worksheet *worksheet, const QVariantMap &defaultMap)
	: m_worksheet(worksheet)
	, m_rows(0)
	, m_cols(0)
	, m_headers()
	, m_defaultMap(defaultMap)
	, m_records()
{
	Q_ASSERT(worksheet);
}




/**
 * @brief AbstractObjectiveImporter::~AbstractObjectiveImporter
 */

AbstractObjectiveImporter::~AbstractObjectiveImporter()
{

}


/**
 * @brief AbstractObjectiveImporter::readObjectives
 * @return
 */

bool AbstractObjectiveImporter::readObjectives()
{
	if (m_headers.isEmpty())
		normalizeHeaders();

	bool ret = true;

	int readRows = DEFAULT_ROW_COUNT;

	for (int i=2; i<readRows; i++) {
		QMultiMap<QString, QXlsx::Cell *> cells;

		for (int j=1; j<m_cols; j++) {
			QXlsx::Cell *cell = m_worksheet->cellAt(i, j);

			if (i >= readRows-SEARCH_STEP && cell) {
				readRows = i+SEARCH_STEP+1;
				if (readRows > MAX_ROW_COUNT)
					readRows = MAX_ROW_COUNT;
			}

			if (!cell)
				continue;

			if (m_headers.contains(j)) {
				cells.insert(m_headers.value(j), cell);
			}
		}

		if (cells.isEmpty())
			continue;

		if (!readRow(cells))
			ret = false;
	}

	m_rows = readRows;

	return ret;
}





/**
 * @brief AbstractObjectiveImporter::normalizeHeaders
 */

void AbstractObjectiveImporter::normalizeHeaders()
{
	int searchCols = DEFAULT_COL_COUNT;

	for (int i=1; i<searchCols; i++) {
		QXlsx::Cell *cell = m_worksheet->cellAt(1, i);

		if (i >= searchCols-SEARCH_STEP && cell) {
			searchCols = i+SEARCH_STEP+1;
			if (searchCols > MAX_COL_COUNT)
				searchCols = MAX_COL_COUNT;
		}

		QString header = cell ? cell->value().toString() : "";

		QString norm = normalizedHeaderString(header, i);

		if (!norm.isEmpty())
			m_headers[i] = norm;
	}

	m_cols = searchCols;
}





/**
 * @brief AbstractObjectiveImporter::addRecord
 * @param map
 */

void AbstractObjectiveImporter::addRecord(const QVariantMap &map)
{
	QVariantMap m = m_defaultMap;
	m.insert(map);
	m_records.append(m);
}


/**
 * @brief AbstractObjectiveImporter::records
 * @return
 */

QVariantList AbstractObjectiveImporter::records() const
{
	return m_records;
}

/**
 * @brief AbstractObjectiveImporter::defaultMap
 * @return
 */

QVariantMap AbstractObjectiveImporter::defaultMap() const
{
	return m_defaultMap;
}

/**
 * @brief AbstractObjectiveImporter::setDefaultMap
 * @param defaultMap
 */

void AbstractObjectiveImporter::setDefaultMap(const QVariantMap &defaultMap)
{
	m_defaultMap = defaultMap;
}




/**
 * @brief AbstractObjectiveImporter::cols
 * @return
 */

int AbstractObjectiveImporter::cols() const
{
	return m_cols;
}


/**
 * @brief AbstractObjectiveImporter::rows
 * @return
 */

int AbstractObjectiveImporter::rows() const
{
	return m_rows;
}


/**
 * @brief AbstractObjectiveImporter::headers
 * @return
 */

QHash<int, QString> AbstractObjectiveImporter::headers() const
{
	return m_headers;
}

