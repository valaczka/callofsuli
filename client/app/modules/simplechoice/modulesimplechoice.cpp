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
	if (!storage) {
		QVariantMap m;
		QStringList answers = data.value("answers").toStringList();
		m["title"] = data.value("question").toString();
		m["details"] = data.value("correct").toString()+"<br>("+answers.join(", ")+")";
		m["image"] = "";

		return m;
	} else if (storage->name() == "binding" || storage->name() == "numbers") {
		QStringList answers;

		bool isBindToRight = data.value("mode").toString() == "right";

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QString("%1 — %2").arg(isBindToRight ? right : left).arg(isBindToRight ? left : right));
		}

		QVariantMap m;
		m["title"] = data.value("question").toString();
		m["details"] = answers.join(", ");
		m["image"] = "";

		return m;
	}

	return QVariantMap({{"title", ""},
						{"details", ""},
						{"image", ""}
					   });
}




/**
 * @brief ModuleSimplechoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m;

		m["question"] = data.value("question").toString();

		QString correct = data.value("correct").toString();

		if (correct.isEmpty())
			correct = " ";

		QStringList alist = data.value("answers").toStringList();

		m.insert(generateOne(correct, alist));

		list.append(m);

		return list;
	}

	if (storage->name() == "binding" || storage->name() == "numbers")
		return generateBinding(data, storageData);


	return QVariantList();
}




/**
 * @brief ModuleSimplechoice::generateBinding
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBinding(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	bool isBindToRight = data.value("mode").toString() == "right";
	QString question = data.value("question").toString();

	foreach (QVariant v, storageData.value("bindings").toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value("first").toString();
		QString right = m.value("second").toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap["question"] = (isBindToRight ? right : left);
		else if (question.contains("%1"))
			retMap["question"] = question.arg(isBindToRight ? right : left);
		else
			retMap["question"] = question;

		QStringList alist;

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap mm = v.toMap();
			QString f1 = mm.value("first").toString();
			QString f2 = mm.value("second").toString();

			if ((isBindToRight && right == f2) || (!isBindToRight && left == f1))
				continue;

			alist.append(isBindToRight ? f1 : f2);
		}

		retMap.insert(generateOne(isBindToRight ? left : right, alist));

		ret.append(retMap);
	}

	return ret;
}


/**
 * @brief ModuleSimplechoice::generateOne
 * @param correctAnswer
 * @param options
 * @return
 */

QVariantMap ModuleSimplechoice::generateOne(const QString &correctAnswer, QStringList optionsList) const
{
	QVector<QPair<QString, bool>> opts;
	opts.append(qMakePair(correctAnswer, true));

	while (optionsList.size() && opts.size() < 4) {
		QString o = optionsList.takeAt(QRandomGenerator::global()->bounded(optionsList.size()));
		if (!o.isEmpty())
			opts.append(qMakePair(o, false));
	}


	int correctIdx = -1;

	QStringList optList;

	while (opts.size()) {
		QPair<QString, bool> p = opts.takeAt(QRandomGenerator::global()->bounded(opts.size()));
		optList.append(p.first);

		if (p.second)
			correctIdx = optList.size()-1;
	}

	QVariantMap m;
	m["answer"] = correctIdx;
	m["options"] = optList;

	return m;
}


/**
 * @brief ModuleSimplechoice::preview
 * @param generatedList
 * @return
 */

QVariantMap ModuleSimplechoice::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		s.append("**"+m.value("question").toString()+"**\n");

		int correct = m.value("answer", -1).toInt();
		QStringList l = m.value("options").toStringList();
		for (int i=0; i<l.size(); ++i) {
			if (i==correct)
				s.append("- **"+l.at(i)+"**\n");
			else
				s.append("- "+l.at(i)+"\n");
		}
		s.append("---\n");
	}

	m["text"] = s;

	return m;
}
