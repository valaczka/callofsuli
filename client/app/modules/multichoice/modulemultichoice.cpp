/*
 * ---- Call of Suli ----
 *
 * modulemultichoice.cpp
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleMultichoice
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

#include "modulemultichoice.h"
#include <QRandomGenerator>

ModuleMultichoice::ModuleMultichoice(QObject *parent) : QObject(parent)
{

}

/**
 * @brief ModuleMultichoice::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleMultichoice::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantMap m;
	m["title"] = data.value("question").toString();
	m["details"] = data.value("corrects").toStringList().join(", ")+"<br>("+data.value("answers").toStringList().join(", ")+")";
	m["image"] = "";

	return m;
}



/**
 * @brief ModuleMultichoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleMultichoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storageData)

	if (!storage) {
		QVariantList list;

		for (int i=0; i<5; ++i)
			list.append(generateOne(data));

		return list;
	}


	return QVariantList();
}


/**
 * @brief ModuleMultichoice::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModuleMultichoice::generateOne(const QVariantMap &data) const
{
	QVariantMap m;

	m["question"] = data.value("question").toString();

	QStringList clist = data.value("corrects").toStringList();

	if (clist.isEmpty())
		clist = QStringList({" "});

	QStringList alist = data.value("answers").toStringList();

	int minCorrect = qMax(data.value("correctMin", -1).toInt(), 2);
	int maxOptions = qMax(data.value("count", -1).toInt(), 3);
	int maxCorrect = qMax(data.value("correctMax", -1).toInt(), maxOptions-1);

	int correctCount = minCorrect;

	if (maxCorrect>minCorrect)
		correctCount = QRandomGenerator::global()->bounded(minCorrect, maxCorrect+1);

	QVector<QPair<QString, bool>> options;

	while (clist.size() && options.size() < correctCount) {
		options.append(qMakePair(clist.takeAt(QRandomGenerator::global()->bounded(clist.size())), true));
	}

	while (alist.size() && options.size() < maxOptions) {
		options.append(qMakePair(alist.takeAt(QRandomGenerator::global()->bounded(alist.size())), false));
	}



	QVariantList correctIdx;

	QStringList optList;

	while (options.size()) {
		QPair<QString, bool> p = options.takeAt(QRandomGenerator::global()->bounded(options.size()));
		optList.append(p.first);

		if (p.second)
			correctIdx.append(optList.size()-1);
	}


	m["options"] = optList;
	m["answer"] = QVariantMap({{"indices", correctIdx}});

	return m;
}
