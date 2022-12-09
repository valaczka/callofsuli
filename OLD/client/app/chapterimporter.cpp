/*
 * ---- Call of Suli ----
 *
 * chapterimporter.cpp
 *
 * Created on: 2021. 01. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ChapterImporter
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

#include "chapterimporter.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"
#include "xlsxworksheet.h"
#include "abstractobjectiveimporter.h"
#include "modules/interfaces.h"
#include "cosclient.h"
#include <QObject>
#include <QFile>
#include <QDebug>

ChapterImporter::ChapterImporter(const QString &filename)
	: m_filename(filename)
	, m_chapterId(-1)
	, m_records()
{

}


/**
 * @brief ChapterImporter::import
 */

bool ChapterImporter::import()
{
	if (m_filename.isEmpty() || !QFile::exists(m_filename))
		return false;

	QXlsx::Document doc(m_filename);

	QStringList sheets = doc.sheetNames();

	foreach (QString s, sheets) {
		QXlsx::AbstractSheet *sheet = doc.sheet(s);
		if (!sheet) {
			qWarning() << "Invalid sheet";
			return false;
		}

		QString name = sheet->sheetName();

		if (sheet->sheetType() != QXlsx::AbstractSheet::ST_WorkSheet) {
			qInfo() << "Incompatible sheet" << name;
			continue;
		}

		QXlsx::Worksheet *ws = static_cast<QXlsx::Worksheet *>(sheet);

		if (!ws) {
			qWarning() << "Invalid worksheet" << name;
			return false;
		}

		if (!Client::moduleObjectiveList().contains(name)) {
			qInfo() << "Invalid worksheet module" << name;
			continue;
		}

		ModuleInterface *mi = Client::moduleObjectiveList().value(name);

		if (!mi->canImport()) {
			qInfo() << "Invalid worksheet module importer" << name;
			continue;
		}

		AbstractObjectiveImporter *importer = mi->newImporter(ws);

		if (!importer) {
			qWarning() << "Invalid importer" << name;
			continue;
		}

		qDebug() << "Read sheet" << name;

		if (m_chapterId != -1)
			importer->defaultMapInsert("chapter", m_chapterId);

		if (!importer->readObjectives()) {
			qWarning() << "Error read objectives" << name;
			delete importer;
			return false;
		}

		if (!importer->records().isEmpty())
			m_records.append(importer->records());

		delete importer;
	}

	return true;
}



int ChapterImporter::chapterId() const
{
	return m_chapterId;
}

void ChapterImporter::setChapterId(int chapterId)
{
	m_chapterId = chapterId;
}

QVariantList ChapterImporter::records() const
{
	return m_records;
}
