/*
 * ---- Call of Suli ----
 *
 * moduletruefalse.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleTruefalse
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
#include "moduletruefalse.h"

ModuleTruefalse::ModuleTruefalse(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleTruefalse::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleTruefalse::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantMap m;
		m["title"] = data.value("question").toString();
		m["details"] = data.value("correct").toBool() ? QObject::tr("igaz") : QObject::tr("hamis");
		m["image"] = "";
		return m;

	} else if (storage->name() == "binding" || storage->name() == "numbers") {
		QStringList answers;

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			answers.append(QString("%1 — %2").arg(left).arg(right));
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
 * @brief ModuleTruefalse::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m = data;

		m.insert("answer", data.value("correct").toBool() ? 1 : 0);

		list.append(m);

		return list;
	}

	if (storage->name() == "binding" || storage->name() == "numbers")
		return generateBinding(data, storageData);


	return QVariantList();
}




/**
 * @brief ModuleTruefalse::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateBinding(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	QString mode = data.value("mode").toString();

	if (mode == "left" || mode == "right") {
		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();


			if (!left.isEmpty()) {
				QVariantMap retMap;
				retMap["question"] = left;
				retMap["answer"] = (mode == "left") ? 1 : 0;
				ret.append(retMap);
			}

			if (!right.isEmpty()) {
				QVariantMap retMap;
				retMap["question"] = right;
				retMap["answer"] = (mode == "right") ? 1 : 0;
				ret.append(retMap);
			}
		}
	} else if (mode == "generateLeft" || mode == "generateRight") {
		QString question = data.value("question").toString();
		bool isBindToRight = mode == "generateRight";

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			bool isCorrect = (QRandomGenerator::global()->generate() % 2 == 1);

			QString questionPart = isBindToRight ? right : left;
			QString answerPart;

			if (isCorrect)
				answerPart = isBindToRight ? left : right;
			else {
				QStringList alist;

				foreach (QVariant v, storageData.value("bindings").toList()) {
					QVariantMap mm = v.toMap();
					QString f1 = mm.value("first").toString();
					QString f2 = mm.value("second").toString();

					if ((isBindToRight && right == f2) || (!isBindToRight && left == f1))
						continue;

					if ((isBindToRight && f1.isEmpty()) || (!isBindToRight && f2.isEmpty()))
						continue;

					alist.append(isBindToRight ? f1 : f2);
				}

				if (alist.size() > 1)
					answerPart = alist.at(QRandomGenerator::global()->bounded(alist.size()));
				else if (alist.size())
					answerPart = alist.at(0);
				else {
					answerPart = isBindToRight ? left : right;
					isCorrect = true;
				}
			}

			QVariantMap retMap;

			if (question.contains("%1") && question.contains("%2"))
				retMap["question"] = question.arg(questionPart).arg(answerPart);
			else if (question.contains("%1"))
				retMap["question"] = question.arg(questionPart)+" "+answerPart;
			else if (question.isEmpty())
				retMap["question"] = questionPart+" "+answerPart;
			else
				retMap["question"] = question+" "+questionPart+" "+answerPart;

			retMap["answer"] = isCorrect ? 1 : 0;

			ret.append(retMap);
		}
	}

	return ret;
}



/**
 * @brief ModuleTruefalse::preview
 * @return
 */

QVariantMap ModuleTruefalse::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		if (m.value("answer").toBool())
			s.append(QString("- [x] **%1**\n").arg(m.value("question").toString()));
		else
			s.append(QString("- [ ] %1\n").arg(m.value("question").toString()));
	}

	m["text"] = s;

	return m;
}



