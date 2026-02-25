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
#include "storageseed.h"
#include "../binding/modulebinding.h"
#include "../block/moduleblock.h"

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
		m[QStringLiteral("image")] = QString();
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
		m[QStringLiteral("image")] = QString();

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
		m[QStringLiteral("image")] = QString();

		return m;
	} else if (storage->name() == QStringLiteral("mergebinding")) {
		const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();
		const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

		QStringList answers;

		for (int i=0; i<sections.size(); ++i) {
			QVariantMap m = sections.at(i).toMap();
			const QString &key = m.value(QStringLiteral("key")).toString();

			if (!usedSections.contains(key))
				continue;

			answers.append(m.value(QStringLiteral("name")).toString());
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QString();

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QString()},
						{QStringLiteral("details"), QString()},
						{QStringLiteral("image"), QString()}
					   });
}



/**
 * @brief ModuleTruefalse::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
										  QVariantMap */*commonDataPtr*/, StorageSeed *seed) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m = data;

		m.insert(QStringLiteral("answer"), data.value(QStringLiteral("correct")).toBool() ? 1 : 0);

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		return generateBinding(data, storageData, seed);
	else if (storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData, seed);
	else if (storage->name() == QStringLiteral("mergebinding"))
		return generateMergeBinding(data, storageData, seed);


	return QVariantList();
}




/**
 * @brief ModuleTruefalse::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();

	SeedDuplexHelper helper(seed, SEED_BINDING_LEFT, SEED_BINDING_RIGHT);

	const QVariantList &list = storageData.value(QStringLiteral("bindings")).toList();
	const QString &question = data.value(QStringLiteral("question")).toString();

	if (mode == QStringLiteral("left") || mode == QStringLiteral("right")) {
		for (int i=0; i<list.size(); ++i) {
			const QVariantMap &m = list.at(i).toMap();
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

				helper.append(retMap, i+1, -1);
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

				helper.append(retMap, -1, i+1);
			}

		}
	} else if (mode == QStringLiteral("generateLeft") || mode == QStringLiteral("generateRight")) {
		bool isBindToRight = mode == QStringLiteral("generateRight");

		for (int i=0; i<list.size(); ++i) {
			const QVariantMap &m = list.at(i).toMap();
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

			helper.append(retMap,
						  isBindToRight ? -1 : i+1,
						  isBindToRight ? i+1 : -1);
		}
	}

	return helper.getVariantList(true);
}




/**
 * @brief ModuleTruefalse::generateBlock
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateBlock(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &question = data.value(QStringLiteral("question")).toString();
	const QVariantList &list = storageData.value(QStringLiteral("blocks")).toList();

	SeedHelper helper(seed, SEED_BLOCK_LEFT);

	QVector<QString> bNames;
	QVector<QStringList> bItems;
	QVector<int> realIndices;
	QVector<int> idxList;

	for (int i=0; i<list.size(); ++i) {
		const QVariantMap &m = list.at(i).toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() && right.isEmpty())
			continue;

		idxList.append(i);

		bNames.append(left);
		bItems.append(right);

		if (!left.isEmpty())
			realIndices.append(bNames.size()-1);
	}


	for (int i=0; i<bItems.size(); ++i) {
		const QStringList &list = bItems.at(i);
		const QString &realname = bNames.at(i);

		const int &idx = idxList.at(i);

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

			helper.append(retMap, idx+1);
		}
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleTruefalse::generateMergeBinding
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleTruefalse::generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();

	SeedDuplexHelper helper(seed, SEED_BINDING_LEFT, SEED_BINDING_RIGHT);

	////const QVariantList &list = storageData.value(QStringLiteral("bindings")).toList();
	const QString &question = data.value(QStringLiteral("question")).toString();


	const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();

	const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

	struct Data {
		QString left;
		QString right;
		int idx = 0;
	};

	QList<Data> list;

	for (int i=0; i<sections.size(); ++i) {
		QVariantMap m = sections.at(i).toMap();
		const QString &key = m.value(QStringLiteral("key")).toString();

		if (!usedSections.contains(key))
			continue;

		const QVariantList &l = m.value(QStringLiteral("bindings")).toList();

		for (int j = 0; j<l.size(); ++j) {
			QVariantMap m = l.at(j).toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			list.append(Data{.left = left, .right = right, .idx = (i+1)*1000+j+1});
		}
	}



	if (mode == QStringLiteral("left") || mode == QStringLiteral("right")) {
		for (const Data &d : list) {
			if (!d.left.isEmpty()) {
				QVariantMap retMap;

				if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(d.left);
				else if (question.isEmpty())
					retMap[QStringLiteral("question")] = d.left;
				else
					retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+d.left;

				retMap[QStringLiteral("answer")] = (mode == QStringLiteral("left")) ? 1 : 0;

				helper.append(retMap, d.idx, -1);
			}

			if (!d.right.isEmpty()) {
				QVariantMap retMap;

				if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(d.right);
				else if (question.isEmpty())
					retMap[QStringLiteral("question")] = d.right;
				else
					retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+d.right;

				retMap[QStringLiteral("answer")] = (mode == QStringLiteral("right")) ? 1 : 0;

				helper.append(retMap, -1, d.idx);
			}

		}
	} else if (mode == QStringLiteral("generateLeft") || mode == QStringLiteral("generateRight")) {
		bool isBindToRight = mode == QStringLiteral("generateRight");

		for (const Data &d : list) {
			if (d.left.isEmpty() || d.right.isEmpty())
				continue;

			bool isCorrect = (QRandomGenerator::global()->generate() % 2 == 1);

			const QString &questionPart = isBindToRight ? d.right : d.left;
			QString answerPart;

			if (isCorrect)
				answerPart = isBindToRight ? d.left : d.right;
			else {
				QStringList alist;

				for (const Data &v : list) {
					if (d.idx == v.idx)
						continue;

					if ((isBindToRight && v.left.isEmpty()) || (!isBindToRight && v.right.isEmpty()))
						continue;

					alist.append(isBindToRight ? v.left : v.right);
				}

				if (alist.size() > 1)
					answerPart = alist.at(QRandomGenerator::global()->bounded(alist.size()));
				else if (alist.size())
					answerPart = alist.at(0);
				else {
					answerPart = isBindToRight ? d.left : d.right;
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

			helper.append(retMap,
						  isBindToRight ? -1 : d.idx,
						  isBindToRight ? d.idx : -1);
		}
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleTruefalse::preview
 * @return
 */

QVariantMap ModuleTruefalse::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	Q_UNUSED(commonData);

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



