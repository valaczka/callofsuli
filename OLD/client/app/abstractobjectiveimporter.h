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

#ifndef ABSTRACTOBJECTIVEIMPORTER_H
#define ABSTRACTOBJECTIVEIMPORTER_H

#include <QObject>
#include <QString>
#include <QHash>
#include "../QtXlsxWriter/xlsxdocument.h"
#include "../QtXlsxWriter/xlsxworksheet.h"

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
