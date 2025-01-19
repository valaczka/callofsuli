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
	const QVariantList &correctList = data.value(QStringLiteral("answer")).toMap().value(QStringLiteral("list")).toList();

	for (int i=0; i<qList.size(); ++i) {
		html += QStringLiteral("<p>") + qList.at(i) + QStringLiteral(" &ndash; ");

		bool success = false;

		if (i < aList.size()) {
			const QVariantMap &m = aList.at(i).toMap();
			success = m.value(QStringLiteral("success"), false).toBool();

			if (success)
				html += QStringLiteral("<span class=\"answer\">");
			else
				html += QStringLiteral("<span class=\"answerFail\">");

			html += m.value(QStringLiteral("answer")).toString() + QStringLiteral("</span>");
		}

		if (!success && i < correctList.size()) {
			html += QStringLiteral(" <span class=\"answerCorrect\">")
					+ correctList.at(i).toString() + QStringLiteral("</span>");
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

	if (!storage) {
		l = data.value(QStringLiteral("pairs")).toList();

		foreach (QVariant v, l) {
			QVariantMap m = v.toMap();
			list.append(QStringLiteral("%1 — %2").arg(m.value(QStringLiteral("first")).toString(),
													  m.value(QStringLiteral("second")).toString()));
		}
	} else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers")) {
		l = storageData.value(QStringLiteral("bindings")).toList();

		foreach (QVariant v, l) {
			QVariantMap m = v.toMap();
			list.append(QStringLiteral("%1 — %2").arg(m.value(QStringLiteral("first")).toString(),
													  m.value(QStringLiteral("second")).toString()));
		}
	} else if (storage->name() == QStringLiteral("block")) {
		foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toStringList().join(QStringLiteral(", "));

			list.append(QStringLiteral("%1 [%2]").arg(left, right));
		}
	}



	QVariantMap m;
	m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
	m[QStringLiteral("details")] = list.join(QStringLiteral(", "));
	m[QStringLiteral("image")] = QStringLiteral("");

	return m;
}



/**
 * @brief ModulePair::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModulePair::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
									 QVariantMap *commonDataPtr) const
{
	Q_UNUSED(commonDataPtr);

	QVariantList list;
	QVariantMap m;

	m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

	QVariantList alist;

	if (!storage)
		alist = data.value(QStringLiteral("pairs")).toList();
	else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		alist = storageData.value(QStringLiteral("bindings")).toList();
	else if (storage->name() == QStringLiteral("block"))
		alist = generateBlock(storageData);

	m.insert(generateOne(data, alist));

	list.append(m);

	return list;


	return QVariantList();
}



/**
 * @brief ModulePair::generateBlock
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModulePair::generateBlock(const QVariantMap &storageData) const
{
	QVariantList ret;

	foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
		const QVariantMap &m = v.toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		QStringList realList;

		foreach (QString s, right) {
			s = s.simplified();
			if (s.isEmpty())
				continue;

			realList.append(s);
		}

		if (left.isEmpty() && realList.isEmpty())
			continue;

		QString second;

		if (!realList.isEmpty())
			second = realList.at(QRandomGenerator::global()->bounded(realList.size()));

		ret.append(QVariantMap {
					   { QStringLiteral("first"), left },
					   { QStringLiteral("second"), second }
				   }
				   );
	}

	return ret;
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
	int maxOptions = qMax(data.value(QStringLiteral("optionsCount"), -1).toInt(), maxQuestions);

	QStringList questions;
	QVariantList answers;
	QStringList options;

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(pairList.begin(), pairList.end(), g);

	for (const QVariant &v : pairList) {
		const QVariantMap m = v.toMap();
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


	std::shuffle(options.begin(), options.end(), g);

	QVariantList mixedList = answers;

	for (const QString &o : options) {
		if (mixedList.size() >= maxOptions)
			break;
		mixedList.append(o);
	}

	std::shuffle(mixedList.begin(), mixedList.end(), g);

	m[QStringLiteral("list")] = questions;
	m[QStringLiteral("options")] = mixedList;
	m[QStringLiteral("answer")] = QVariantMap({{ QStringLiteral("list"), answers }});


	return m;
}
