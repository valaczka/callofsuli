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
 * @brief ModuleMultichoice::testResult
 * @param data
 * @param answer
 * @return
 */

QString ModuleMultichoice::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();

	QString html = QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	if (answer.contains(QStringLiteral("list"))) {
		const QVariantList &list = answer.value(QStringLiteral("list")).toList();

		QStringList a;

		foreach (const QVariant &v, list) {
			bool ok = false;
			const int &idx = v.toInt(&ok);

			if (ok && idx >= 0 && idx < options.size())
				a.append(options.at(idx));
			else
				a.append(QStringLiteral("???"));
		}

		if (success)
			html += QStringLiteral("<p class=\"answer\">");
		else
			html += QStringLiteral("<p class=\"answerFail\">");

		html += a.join(QStringLiteral(", ")) + QStringLiteral("</p>");
	}

	return html;
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
	m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
	m[QStringLiteral("details")] = data.value(QStringLiteral("corrects")).toStringList().join(QStringLiteral(", "))+
			QStringLiteral("<br>(")+data.value(QStringLiteral("answers")).toStringList().join(QStringLiteral(", "))+
			QStringLiteral(")");
	m[QStringLiteral("image")] = QLatin1String("");

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
 * @brief ModuleMultichoice::preview
 * @return
 */

QVariantMap ModuleMultichoice::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		s.append(QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));

		QVector<int> idxList;

		foreach (const QVariant &v, m.value(QStringLiteral("answer")).toList())
			idxList.append(v.toInt());

		const QStringList &l = m.value(QStringLiteral("options")).toStringList();

		for (int i=0; i<l.size(); ++i) {
			if (idxList.contains(i))
				s.append(QStringLiteral("- **")+l.at(i)+QStringLiteral("**\n"));
			else
				s.append(QStringLiteral("- ")+l.at(i)+QStringLiteral("\n"));
		}

		s.append(QStringLiteral("---\n"));
	}

	m[QStringLiteral("text")] = s;

	return m;
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

	m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

	QStringList clist = data.value(QStringLiteral("corrects")).toStringList();

	if (clist.isEmpty())
		clist = QStringList({QStringLiteral(" ")});

	QStringList alist = data.value(QStringLiteral("answers")).toStringList();

	int minCorrect = qMax(data.value(QStringLiteral("correctMin"), -1).toInt(), 2);
	int maxOptions = qMax(data.value(QStringLiteral("count"), -1).toInt(), 3);
	int maxCorrect = qMin(data.value(QStringLiteral("correctMax"), -1).toInt(), maxOptions-1);

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


	m[QStringLiteral("options")] = optList;
	m[QStringLiteral("answer")] = correctIdx;

	return m;
}
