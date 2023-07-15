/*
 * ---- Call of Suli ----
 *
 * modulepair.cpp
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModulePair
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

#include "modulepair.h"
#include <QRandomGenerator>

ModulePair::ModulePair(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModulePair::testResult
 * @param data
 * @param answer
 * @return
 */

QString ModulePair::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &) const
{
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();

	QString html = QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	const QStringList &qList = data.value(QStringLiteral("list")).toStringList();
	const QVariantList &aList = answer.value(QStringLiteral("list")).toList();

	for (int i=0; i<qList.size(); ++i) {
		html += QStringLiteral("<p>") + qList.at(i) + QStringLiteral(" &ndash; ");

		if (i < aList.size()) {
			const QVariantMap &m = aList.at(i).toMap();

			if (m.value(QStringLiteral("success"), false).toBool())
				html += QStringLiteral("<span class=\"answer\">");
			else
				html += QStringLiteral("<span class=\"answerFail\">");

			html += m.value(QStringLiteral("answer")).toString() + QStringLiteral("</span>");
		}

		html += QStringLiteral("</p>");
	}

	return html;
}


/**
 * @brief ModulePair::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModulePair::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{

	QStringList list;

	QVariantList l;

	if (!storage)
		l = data.value(QStringLiteral("pairs")).toList();
	else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		l = storageData.value(QStringLiteral("bindings")).toList();

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		list.append(QStringLiteral("%1 — %2").arg(m.value(QStringLiteral("first")).toString(),
												  m.value(QStringLiteral("second")).toString()));
	}

	QVariantMap m;
	m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
	m[QStringLiteral("details")] = list.join(QStringLiteral(", "));
	m[QStringLiteral("image")] = QLatin1String("");

	return m;
}



/**
 * @brief ModulePair::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModulePair::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	QVariantList list;
	QVariantMap m;

	m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

	QVariantList alist;

	if (!storage)
		alist = data.value(QStringLiteral("pairs")).toList();
	else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		alist = storageData.value(QStringLiteral("bindings")).toList();

	m.insert(generateOne(data, alist));

	list.append(m);

	return list;


	return QVariantList();
}


/**
 * @brief ModulePair::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModulePair::generateOne(const QVariantMap &data, QVariantList pairList) const
{
	QVariantMap m;

	if (pairList.isEmpty())
		pairList = {
			QVariantMap({{QStringLiteral("first"), QStringLiteral(" ")}, {QStringLiteral("second"), QStringLiteral(" ")}}),
			QVariantMap({{QStringLiteral("first"), QStringLiteral(" ")}, {QStringLiteral("second"), QStringLiteral(" ")}})
		};

	QString mode = data.value(QStringLiteral("mode")).toString();

	Modes modes;

	if (mode == QStringLiteral("first")) {
		modes.setFlag(Mode::First);
	} else if (mode == QStringLiteral("second")) {
		modes.setFlag(Mode::Second);
	} else if (mode == QStringLiteral("both")) {
		if (QRandomGenerator::global()->bounded(2) == 1)
			modes.setFlag(Mode::Second);
		else
			modes.setFlag(Mode::First);
	} else if (mode == QStringLiteral("shuffle")) {
		modes.setFlag(Mode::First);
		modes.setFlag(Mode::Second);
	} else {
		modes.setFlag(Mode::First);
	}

	int maxQuestions = qMax(data.value(QStringLiteral("count"), -1).toInt(), 3);
	int maxOptions = qMax(data.value(QStringLiteral("optionsCount"), -1).toInt(), maxQuestions+1);

	QStringList questions;
	QVariantList answers;
	QStringList options;

	while (pairList.size()) {
		QVariantMap m = pairList.takeAt(QRandomGenerator::global()->bounded(pairList.size())).toMap();
		QString first = m.value(QStringLiteral("first")).toString();
		QString second = m.value(QStringLiteral("second")).toString();

		if (first.isEmpty() && second.isEmpty())
			continue;

		if ((!first.isEmpty() && !second.isEmpty()) && (questions.size() < maxQuestions)) {
			Mode currentMode;
			if (modes.testFlag(Mode::First) && modes.testFlag(Mode::Second)) {
				if (QRandomGenerator::global()->bounded(2) == 1)
					currentMode = Mode::Second;
				else
					currentMode = Mode::First;
			} else if (modes.testFlag(Mode::Second))
				currentMode = Mode::Second;
			else
				currentMode = Mode::First;

			if (currentMode == Mode::First) {
				questions.append(first);
				answers.append(second);
			} else {
				questions.append(second);
				answers.append(first);
			}
		} else if (!first.isEmpty() && modes.testFlag(Mode::Second)) {
			options.append(first);
		} else if (!second.isEmpty() && modes.testFlag(Mode::First)) {
			options.append(second);
		}
	}



	QVariantList mixedList = answers;

	while (options.size() && mixedList.size() < maxOptions)
		mixedList.append(options.takeAt(QRandomGenerator::global()->bounded(options.size())));

	QStringList optList;

	while (mixedList.size())
		optList.append(mixedList.takeAt(QRandomGenerator::global()->bounded(mixedList.size())).toString());

	m[QStringLiteral("list")] = questions;
	m[QStringLiteral("options")] = optList;
	m[QStringLiteral("answer")] = QVariantMap({{ QStringLiteral("list"), answers }});


	return m;
}
