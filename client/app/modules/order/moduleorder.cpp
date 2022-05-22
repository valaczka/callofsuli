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
 * @brief ModulePair::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleOrder::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	QString mode = data.value("mode").toString();
	QString question;

	if (mode == "ascending" || mode == "random")
		question = data.value("questionAsc").toString();

	if (mode == "descending" || mode == "random") {
		if (!question.isEmpty())
			question += " / ";
		question += data.value("questionDesc").toString();
	}

	QStringList list;

	if (!storage)
		list = data.value("items").toStringList();
	else if (storage->name() == "sequence")
		list = storageData.value("items").toStringList();
	else if (storage->name() == "numbers") {
		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			list.append(QString("%1 — %2").arg(left).arg(right));
		}
	}

	QVariantMap m;
	m["title"] = question;
	m["details"] = list.join(", ");
	m["image"] = "";

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
	int cnt = data.value("count", 5).toInt();

	if (!storage)
		slist = generateItems(data.value("items").toStringList(), cnt);
	else if (storage->name() == "sequence")
		slist = generateItems(storageData.value("items").toStringList(), cnt);
	else if (storage->name() == "numbers")
		slist = generateItems(storageData.value("bindings").toList(), cnt);


	bool isDesc = false;

	QString mode = data.value("mode").toString();

	if (mode == "descending")
		isDesc = true;
	else if (mode == "random")
		isDesc = (QRandomGenerator::global()->generate() % 2 == 1);


	m["mode"] = isDesc ? "descending" : "ascending";
	m["question"] = isDesc ? data.value("questionDesc").toString() : data.value("questionAsc").toString();
	m["placeholderMin"] = data.value("placeholderMin").toString();
	m["placeholderMax"] = data.value("placeholderMax").toString();
	m["list"] = slist;

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
								 { "text", list.at(i) },
								 { "num", i }
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

		QString first = m.value("first").toString();

		if (first.isEmpty() || m.value("second").toString().isEmpty())
			continue;

		l.append(QVariantMap({
								 { "text", first },
								 { "num", m.value("second").toReal() }
							 }));
	}

	while (!l.isEmpty() && ret.size() < count)
		ret.append(l.takeAt(QRandomGenerator::global()->bounded(l.size())));

	return ret;
}


