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

#include "chapterimporter.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"
#include "xlsxworksheet.h"
#include "abstractobjectiveimporter.h"
#include "objectiveimportersimplechoice.h"
#include "objectiveimportertruefalse.h"
#include <QObject>
#include <QFile>

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


		AbstractObjectiveImporter *importer = nullptr;

		if (name == "simplechoice")
			importer = new ObjectiveImporterSimplechoice(ws);
		else if (name == "truefalse")
			importer = new ObjectiveImporterTruefalse(ws);
		else {
			qInfo() << "Invalid worksheet name" << name;
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
