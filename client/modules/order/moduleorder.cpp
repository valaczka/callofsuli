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

#include "moduleorder.h"
#include <QRandomGenerator>

ModuleOrder::ModuleOrder(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleOrder::testResult
 * @param data
 * @param answer
 * @param success
 * @return
 */

QString ModuleOrder::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &) const
{
	QStringList options;

	const QVariantList &qList = data.value(QStringLiteral("list")).toList();

	for (const QVariant &v : std::as_const(qList)) {
		const QVariantMap &m = v.toMap();
		options.append(m.value(QStringLiteral("text")).toString());
	}

	QString html = QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	const QVariantList &aList = answer.value(QStringLiteral("list")).toList();
	const QVariantList &correctList = data.value(QStringLiteral("answer")).toList();

	for (int i=0; i<qList.size(); ++i) {
		html += QStringLiteral("<p>");
		html += QStringLiteral("%1. ").arg(i+1);

		bool success = false;

		if (i < aList.size()) {
			const QVariantMap &m = aList.at(i).toMap();
			success = m.value(QStringLiteral("success"), false).toBool();
			const auto &v = m.value(QStringLiteral("answer"), -2);
			const int idx = v.isNull() ? -1 : v.toInt();

			if (success)
				html += QStringLiteral("<span class=\"answer\">");
			else
				html += QStringLiteral("<span class=\"answerFail\">");

			if (const auto it = std::find_if(qList.cbegin(), qList.cend(), [&idx](const QVariant &v){
											 return v.toMap().value(QStringLiteral("num")).toInt() == idx;
		}); it != qList.cend()) {
				html += it->toMap().value(QStringLiteral("text")).toString();
			}

			html += QStringLiteral("</span>");
		}

		if (!success && i < correctList.size()) {
			const int idx = correctList.at(i).toInt();

			if (const auto it = std::find_if(qList.cbegin(), qList.cend(), [&idx](const QVariant &v){
											 return v.toMap().value(QStringLiteral("num")).toInt() == idx;
		}); it != qList.cend()) {
				html += QStringLiteral(" <span class=\"answerCorrect\">");
				html += it->toMap().value(QStringLiteral("text")).toString();
				html += QStringLiteral("</span>");
			}
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

QVariantMap ModuleOrder::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	QString mode = data.value(QStringLiteral("mode")).toString();
	QString question;

	if (mode == QStringLiteral("ascending") || mode == QStringLiteral("random"))
		question = data.value(QStringLiteral("questionAsc")).toString();

	if (mode == QStringLiteral("descending") || mode == QStringLiteral("random")) {
		if (!question.isEmpty())
			question += QStringLiteral(" / ");
		question += data.value(QStringLiteral("questionDesc")).toString();
	}

	QStringList list;

	if (!storage)
		list = data.value(QStringLiteral("items")).toStringList();
	else if (storage->name() == QStringLiteral("sequence"))
		list = storageData.value(QStringLiteral("items")).toStringList();
	else if (storage->name() == QStringLiteral("numbers")) {
		foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			list.append(QStringLiteral("%1 — %2").arg(left, right));
		}
	}

	QVariantMap m;
	m[QStringLiteral("title")] = question;
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

QVariantList ModuleOrder::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	QVariantList list;
	QVariantMap m;

	QVariantList slist;
	int cnt = data.value(QStringLiteral("count"), 5).toInt();

	if (!storage)
		slist = generateItems(data.value(QStringLiteral("items")).toStringList(), cnt);
	else if (storage->name() == QStringLiteral("sequence"))
		slist = generateItems(storageData.value(QStringLiteral("items")).toStringList(), cnt);
	else if (storage->name() == QStringLiteral("numbers"))
		slist = generateItems(storageData.value(QStringLiteral("bindings")).toList(), cnt);


	bool isDesc = false;

	QString mode = data.value(QStringLiteral("mode")).toString();

	if (mode == QStringLiteral("descending"))
		isDesc = true;
	else if (mode == QStringLiteral("random"))
		isDesc = (QRandomGenerator::global()->generate() % 2 == 1);


	m[QStringLiteral("mode")] = isDesc ? QStringLiteral("descending") : QStringLiteral("ascending");
	m[QStringLiteral("question")] = isDesc ? data.value(QStringLiteral("questionDesc")).toString() : data.value(QStringLiteral("questionAsc")).toString();
	m[QStringLiteral("placeholderMin")] = data.value(QStringLiteral("placeholderMin")).toString();
	m[QStringLiteral("placeholderMax")] = data.value(QStringLiteral("placeholderMax")).toString();
	m[QStringLiteral("list")] = slist;


	// Get correct index list

	QVector<int> numbers;

	for (int i=0; i<slist.size(); ++i) {
		const int &num = slist.at(i).toMap().value(QStringLiteral("num")).toInt();
		numbers.append(num);
	}


	if (isDesc)
		std::sort(numbers.begin(), numbers.end(), std::greater<>());
	else
		std::sort(numbers.begin(), numbers.end(), std::less<>());

	QVariantList aList;

	for (auto it = numbers.constBegin(); it != numbers.constEnd(); ++it)
		aList.append(*it);

	m[QStringLiteral("answer")] = aList;

	list.append(m);


	return list;
}




/**
 * @brief ModuleOrder::generateItems
 * @param list
 * @return
 */

QVariantList ModuleOrder::generateItems(const QStringList &list, const int &count) const
{
	QVariantList ret;

	QVariantList l;

	for (int i=0; i<list.size(); ++i) {
		l.append(QVariantMap({
								 { QStringLiteral("text"), list.at(i) },
								 { QStringLiteral("num"), i }
							 }));
	}

	while (!l.isEmpty() && ret.size() < count)
		ret.append(l.takeAt(QRandomGenerator::global()->bounded(l.size())));

	return ret;
}


/**
 * @brief ModuleOrder::generateItems
 * @param list
 * @return
 */

QVariantList ModuleOrder::generateItems(const QVariantList &list, const int &count) const
{
	QVariantList ret;

	QVariantList l;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		QString first = m.value(QStringLiteral("first")).toString();

		if (first.isEmpty() || m.value(QStringLiteral("second")).toString().isEmpty())
			continue;

		l.append(QVariantMap({
								 { QStringLiteral("text"), first },
								 { QStringLiteral("num"), m.value(QStringLiteral("second")).toReal() }
							 }));
	}

	while (!l.isEmpty() && ret.size() < count)
		ret.append(l.takeAt(QRandomGenerator::global()->bounded(l.size())));

	return ret;
}


