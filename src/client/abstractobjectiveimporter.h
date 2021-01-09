/*
 * ---- Call of Suli ----
 *
 * objectiveimporter.h
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

#ifndef ABSTRACTOBJECTIVEIMPORTER_H
#define ABSTRACTOBJECTIVEIMPORTER_H

#include <QObject>
#include <QString>
#include <QHash>
#include "../common/gamemap.h"
#include "xlsxdocument.h"
#include "xlsxworksheet.h"

#define DEFAULT_COL_COUNT	100
#define DEFAULT_ROW_COUNT	300
#define SEARCH_STEP	5
#define MAX_COL_COUNT	999
#define MAX_ROW_COUNT	99999

class AbstractObjectiveImporter
{
public:
	AbstractObjectiveImporter(QXlsx::Worksheet *worksheet, const QVariantMap &defaultMap = QVariantMap());
	virtual ~AbstractObjectiveImporter();

	bool readObjectives();

	QHash<int, QString> headers() const;
	int rows() const;
	int cols() const;

	QVariantMap defaultMap() const;
	void setDefaultMap(const QVariantMap &defaultMap);
	void defaultMapInsert(const QString &key, const QVariant &variant) { m_defaultMap.insert(key, variant); }

	QVariantList records() const;

protected:
	void normalizeHeaders();
	virtual QString normalizedHeaderString(const QString &header, const int &col) = 0;
	virtual bool readRow(const QMultiMap<QString, QXlsx::Cell *> &cells) = 0;
	void addRecord(const QVariantMap &map);

protected:
	QXlsx::Worksheet *m_worksheet;
	int m_rows;
	int m_cols;
	QHash<int, QString> m_headers;
	QVariantMap m_defaultMap;

private:
	QVariantList m_records;

};

#endif // OBJECTIVEIMPORTER_H
