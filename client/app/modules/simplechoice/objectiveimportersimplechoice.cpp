/*
 * ---- Call of Suli ----
 *
 * objectiveimportersimplechoice.cpp
 *
 * Created on: 2021. 01. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectiveImporterSimplechoice
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

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>


#include "objectiveimportersimplechoice.h"

ObjectiveImporterSimplechoice::ObjectiveImporterSimplechoice(QXlsx::Worksheet *worksheet)
	: AbstractObjectiveImporter(worksheet, { { "module", "simplechoice" } })
{

}


/**
 * @brief ObjectiveImporterSimplechoice::normalizedHeaderString
 * @param header
 * @return
 */

QString ObjectiveImporterSimplechoice::normalizedHeaderString(const QString &header, const int &)
{
	QString h = header.toLower();

	if (h == "question")
		return "question";
	else if (h == "correct")
		return "correct";

	return "answer";
}


/**
 * @brief ObjectiveImporterSimplechoice::readRow
 * @param cells
 * @return
 */

bool ObjectiveImporterSimplechoice::readRow(const QMultiMap<QString, QXlsx::Cell *> &cells)
{
	QJsonObject m;

	QXlsx::Cell *question = cells.value("question", nullptr);
	QXlsx::Cell *correct = cells.value("correct", nullptr);

	if (question)
		m["question"] = question->value().toString();
	else
		m["question"] = "";

	if (correct)
		m["correct"] = correct->value().toString();
	else
		m["correct"] = "";

	QJsonArray l;
	QList<QXlsx::Cell *> answers = cells.values("answer");
	foreach (QXlsx::Cell *c, answers) {
		if (c) {
			l.append(c->value().toString());
		}
	}
	m["answers"] = l;

	QJsonDocument doc(m);

	QVariantMap r;
	r["data"] = QString(doc.toJson(QJsonDocument::Compact));

	addRecord(r);
	return true;
}
