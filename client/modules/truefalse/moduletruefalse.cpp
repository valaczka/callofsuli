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
 * @brief ModuleTruefalse::testResult
 * @param answer
 * @param success
 * @return
 */

QString ModuleTruefalse::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	QString html = QStringLiteral("<p>");
	html += tr("Igaz vagy hamis? ");

	if (answer.contains(QStringLiteral("index"))) {
		const int &idx = answer.value(QStringLiteral("index"), -1).toInt();

		if (success)
			html += QStringLiteral("<span class=\"answer\">");
		else
			html += QStringLiteral("<span class=\"answerFail\">");

		if (idx == 1)
			html += tr("IGAZ");
		else if (idx == 0)
			html += tr("HAMIS");

		html += QStringLiteral("</span>");
	}

	if (const int cIdx = data.value(QStringLiteral("answer")).toInt(); !success && cIdx>=0 && cIdx<2) {
		static const QStringList list = { tr("Hamis"), tr("igaz")};
		html += QStringLiteral(" <span class=\"answerCorrect\">")
				+ list.at(cIdx) + QStringLiteral("</span>");
	}

	html += QStringLiteral("</p>");

	return html;
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
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = data.value(QStringLiteral("correct")).toBool() ? QObject::tr("igaz") : QObject::tr("hamis");
		m[QStringLiteral("image")] = QStringLiteral("");
		return m;

	} else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers")) {
		QStringList answers;

		foreach (const QVariant &v, storageData.value(QStringLiteral("bindings")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toString();

			answers.append(QStringLiteral("%1 — %2").arg(left, right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	} else if (storage->name() == QStringLiteral("block")) {
		QStringList answers;

		foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toStringList().join(QStringLiteral(", "));

			answers.append(QStringLiteral("%1 [%2]").arg(left, right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QStringLiteral("")},
						{QStringLiteral("details"), QStringLiteral("")},
						{QStringLiteral("image"), QStringLiteral("")}
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

		m.insert(QStringLiteral("answer"), data.value(QStringLiteral("correct")).toBool() ? 1 : 0);

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		return generateBinding(data, storageData);
	else if (storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData);


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

	const QString &mode = data.value(QStringLiteral("mode")).toString();

	if (mode == QStringLiteral("left") || mode == QStringLiteral("right")) {
		const QString &question = data.value(QStringLiteral("question")).toString();

		foreach (const QVariant &v, storageData.value(QStringLiteral("bindings")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toString();


			if (!left.isEmpty()) {
				QVariantMap retMap;
				if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(left);
				else if (question.isEmpty())
					retMap[QStringLiteral("question")] = left;
				else
					retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+left;

				retMap[QStringLiteral("answer")] = (mode == QStringLiteral("left")) ? 1 : 0;
				ret.append(retMap);
			}

			if (!right.isEmpty()) {
				QVariantMap retMap;
				if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(right);
				else if (question.isEmpty())
					retMap[QStringLiteral("question")] = right;
				else
					retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+right;
				retMap[QStringLiteral("answer")] = (mode == QStringLiteral("right")) ? 1 : 0;
				ret.append(retMap);
			}
		}
	} else if (mode == QStringLiteral("generateLeft") || mode == QStringLiteral("generateRight")) {
		const QString &question = data.value(QStringLiteral("question")).toString();
		bool isBindToRight = mode == QStringLiteral("generateRight");

		foreach (const QVariant &v, storageData.value(QStringLiteral("bindings")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			bool isCorrect = (QRandomGenerator::global()->generate() % 2 == 1);

			const QString &questionPart = isBindToRight ? right : left;
			QString answerPart;

			if (isCorrect)
				answerPart = isBindToRight ? left : right;
			else {
				QStringList alist;

				foreach (const QVariant &v, storageData.value(QStringLiteral("bindings")).toList()) {
					const QVariantMap &mm = v.toMap();
					const QString &f1 = mm.value(QStringLiteral("first")).toString();
					const QString &f2 = mm.value(QStringLiteral("second")).toString();

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

			if (question.contains(QStringLiteral("%1")) && question.contains(QStringLiteral("%2")))
				retMap[QStringLiteral("question")] = question.arg(questionPart, answerPart);
			else if (question.contains(QStringLiteral("%1")))
				retMap[QStringLiteral("question")] = question.arg(questionPart)+QStringLiteral(" ")+answerPart;
			else if (question.isEmpty())
				retMap[QStringLiteral("question")] = questionPart+QStringLiteral(" ")+answerPart;
			else
				retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+questionPart+QStringLiteral(" ")+answerPart;

			retMap[QStringLiteral("answer")] = isCorrect ? 1 : 0;

			ret.append(retMap);
		}
	}

	return ret;
}




/**
 * @brief ModuleTruefalse::generateBlock
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateBlock(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;
	const QString &question = data.value(QStringLiteral("question")).toString();

	QVector<QString> bNames;
	QVector<QStringList> bItems;
	QVector<int> realIndices;

	foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
		const QVariantMap &m = v.toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() && right.isEmpty())
			continue;

		bNames.append(left);
		bItems.append(right);

		if (!left.isEmpty())
			realIndices.append(bNames.size()-1);
	}


	for (int i=0; i<bItems.size(); ++i) {
		const QStringList &list = bItems.at(i);
		const QString &realname = bNames.at(i);

		foreach (QString s, list) {
			s = s.simplified();
			if (s.isEmpty())
				continue;

			bool isCorrect = (QRandomGenerator::global()->generate() % 2 == 1);

			QString qName;

			if (realname.isEmpty())
				isCorrect = false;

			if (isCorrect)
				qName = realname;
			else {
				QVector<int> indices = realIndices;
				indices.removeAll(i);

				if (indices.isEmpty())
					isCorrect = true;
				else {
					qName = bNames.at(indices.at(QRandomGenerator::global()->bounded(indices.size()))).simplified();
				}
			}

			QVariantMap retMap;

			if (question.contains(QStringLiteral("%1")) && question.contains(QStringLiteral("%2")))
				retMap[QStringLiteral("question")] = question.arg(s, qName);
			else if (question.contains(QStringLiteral("%1")))
				retMap[QStringLiteral("question")] = question.arg(s)+QStringLiteral(" ")+qName;
			else if (question.isEmpty())
				retMap[QStringLiteral("question")] = s+QStringLiteral(" ")+qName;
			else
				retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+s+QStringLiteral(" ")+qName;

			retMap[QStringLiteral("answer")] = isCorrect ? 1 : 0;

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

	foreach (const QVariant &v, generatedList) {
		const QVariantMap &m = v.toMap();

		if (m.value(QStringLiteral("answer")).toBool())
			s.append(QStringLiteral("- [x] **%1**\n").arg(m.value(QStringLiteral("question")).toString()));
		else
			s.append(QStringLiteral("- [ ] %1\n").arg(m.value(QStringLiteral("question")).toString()));
	}

	m[QStringLiteral("text")] = s;

	return m;
}



