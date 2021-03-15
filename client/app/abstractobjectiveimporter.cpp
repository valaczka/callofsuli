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
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

