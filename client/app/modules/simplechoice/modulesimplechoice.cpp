/*
 * ---- Call of Suli ----
 *
 * modulesimplechoice.cpp
 *
 * Created on: 2021. 08. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleSimplechoice
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

#include <QRandomGenerator>
#include "modulesimplechoice.h"

ModuleSimplechoice::ModuleSimplechoice(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleSimplechoice::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleSimplechoice::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QStringList answers = data.value("answers").toStringList();

	QVariantMap m;
	m["title"] = data.value("question").toString();
	m["details"] = data.value("correct").toString()+"<br>("+answers.join(", ")+")";
	m["image"] = "";

	return m;
}



/**
 * @brief ModuleSimplechoice::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModuleSimplechoice::generate(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answer) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantMap m;

	m["question"] = data.value("question").toString();

	QString correct = data.value("correct").toString();

	if (correct.isEmpty())
		return m;

	QStringList alist = data.value("answers").toStringList();

	QVector<QPair<QString, bool>> options;
	options.append(qMakePair(correct, true));

	while (options.size() < 4 && alist.size()) {
		if (alist.size() == 1) {
			options.append(qMakePair(alist.at(0), false));
			alist.clear();
			break;
		}

		QString o = alist.takeAt(QRandomGenerator::global()->bounded(alist.size()));

		options.append(qMakePair(o, false));
	}

	QStringList oList;

	int correctIdx = -1;

	while (options.size()) {
		QPair<QString, bool> p;
		if (options.size() > 1) {
			p = options.takeAt(QRandomGenerator::global()->bounded(options.size()));
		} else {
			p = options.at(0);
			options.clear();
		}

		oList.append(p.first);

		if (p.second)
			correctIdx = oList.size()-1;
	}


	m["options"] = oList;
	m["xpFactor"] = 1.1;

	if (answer) {
		(*answer)["index"] = correctIdx;
	}

	return m;
}