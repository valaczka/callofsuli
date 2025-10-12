/*
 * ---- Call of Suli ----
 *
 * moduleselector.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleSelector
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
#include "moduleselector.h"
#include "qjsonarray.h"
#include "storageseed.h"
#include "../binding/modulebinding.h"
#include "../block/moduleblock.h"


/**
 * @brief ModuleSelector::ModuleSelector
 * @param parent
 */

ModuleSelector::ModuleSelector(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleSelector::testResult
 * @param answer
 * @param success
 * @return
 */

QString ModuleSelector::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	QString html = QStringLiteral("<p>");

	const QStringList optList = data.value(QStringLiteral("options")).toStringList();
	const QString correctAnswer = data.value(QStringLiteral("answer")).toString();

	if (success)
		html += QStringLiteral("<span class=\"answer\">");
	else
		html += QStringLiteral("<span class=\"answerFail\">");


	if (answer.contains(QStringLiteral("index"))) {
		if (success)
			html += QStringLiteral("<span class=\"answer\">") + correctAnswer;
		else {
			html += QStringLiteral("<span class=\"answerFail\">");

			const int idx = answer.value(QStringLiteral("index"), -1).toInt();

			if (idx >= 0 && idx < optList.size())
				html += QStringLiteral(" (")+optList.at(idx)+QStringLiteral(")");
		}

		html += QStringLiteral("</span>");
	}

	if (const int idx = data.value(QStringLiteral("correct"), -1).toInt(); !success && idx > -1) {
		html += QStringLiteral(" <span class=\"answerCorrect\">");
		html += correctAnswer;

		if (idx < optList.size())
			html += QStringLiteral(" (")+optList.at(idx)+QStringLiteral(")");


		html += QStringLiteral("</span>");
	}

	html += QStringLiteral("</p>");

	return html;
}


/**
 * @brief ModuleSelector::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleSelector::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (storage && storage->name() == QStringLiteral("binding")) {
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
	}

	return QVariantMap({{QStringLiteral("title"), QString()},
						{QStringLiteral("details"), QString()},
						{QStringLiteral("image"), QString()}
					   });
}



/**
 * @brief ModuleSelector::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSelector::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
										 QVariantMap *commonDataPtr, StorageSeed *seed) const
{
	if (storage && storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData, commonDataPtr, seed);

	if (storage && storage->name() == QStringLiteral("block"))
		return generateBlockContains(data, storageData, commonDataPtr, seed);

	return QVariantList();
}



/**
 * @brief ModuleSelector::generateOne
 */

QVariantMap ModuleSelector::generateOne(const QString &answer, const int &maxOptions) const
{
	std::vector<int> indices;
	indices.reserve(answer.length());

	// Az első karaktert soha nem kérdezzük (hogy ne legyen könnyű)

	for (int i=1; i<answer.size(); ++i) {
		if (!answer.at(i).isSpace() && !answer.at(i).isPunct())
			indices.emplace_back(i);
	}

	if (indices.empty())
		return {};

	static const QString &upperLetters = QStringLiteral("AÁBCDEÉFGHIÍJKLMNOÖŐPQRSTUÚÜŰVWXYZ");
	static const QString &lowerLetters = QStringLiteral("aábcdeéfghiíjklmnoöőpqrstuúüűvwxyz");

	const int idx = indices.at(QRandomGenerator::global()->bounded((int) indices.size()));
	const QChar ch = answer.at(idx);


	QVector<QPair<QChar, bool>> opts;

	const QString *ptr = ch.isUpper() ? &upperLetters : &lowerLetters;

	for (auto it = ptr->cbegin(); it != ptr->cend(); ++it) {
		if (*it != ch)
			opts.append(qMakePair(*it, false));
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(opts.begin(), opts.end(), g);


	QVector<QPair<QChar, bool>> options(opts.cbegin(), opts.cbegin() + (std::min((int) opts.size(),
																				 (maxOptions > 1 ? maxOptions-1 : 3)
																				 )));

	options.append(qMakePair(ch, true));

	std::shuffle(options.begin(), options.end(), g);

	QStringList optList;
	int correctIdx = -1;

	for (const auto &p : options) {
		optList.append(p.first);

		if (p.second)
			correctIdx = optList.size()-1;
	}

	QVariantMap m;
	m[QStringLiteral("index")] = idx;
	m[QStringLiteral("correct")] = correctIdx;
	m[QStringLiteral("options")] = optList;

	return m;
}




/**
 * @brief ModuleSelector::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSelector::generateBinding(const QVariantMap &data, const QVariantMap &storageData,
											 QVariantMap *commonDataPtr, StorageSeed *seed) const
{
	Q_UNUSED(commonDataPtr);

	SeedDuplexHelper helper(seed, SEED_BINDING_LEFT, SEED_BINDING_RIGHT);

	const QString &question = data.value(QStringLiteral("question")).toString();
	const bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();


	const QVariantList &list = storageData.value(QStringLiteral("bindings")).toList();

	for (int i=0; i<list.size(); ++i) {
		const QVariantMap m = list.at(i).toMap();
		const QString left = m.value(QStringLiteral("first")).toString();
		const QString right = m.value(QStringLiteral("second")).toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = (isBindToRight ? right : left);
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(isBindToRight ? right : left);
		else
			retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+(isBindToRight ? right : left);

		retMap[QStringLiteral("answer")] = isBindToRight ? left : right;

		retMap.insert(generateOne(isBindToRight ? left : right, maxOptions));

		helper.append(retMap, i+1, i+1);
	}


	return helper.getVariantList(true);
}





/**
 * @brief ModuleSelector::generateBlockContains
 * @param data
 * @param storageData
 * @param commonDataPtr
 * @param seed
 * @return
 */

QVariantList ModuleSelector::generateBlockContains(const QVariantMap &data, const QVariantMap &storageData,
												   QVariantMap *commonDataPtr, StorageSeed *seed) const
{
	Q_UNUSED(commonDataPtr);

	SeedDuplexHelper helper(seed, SEED_BLOCK_RIGHT, SEED_BLOCK_LEFT);

	const QString &question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();

	const QVariantList &list = storageData.value(QStringLiteral("blocks")).toList();

	for (int idx = 0; idx < list.size(); ++idx) {
		const QVariantMap &m = list.at(idx).toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		for (int i=0; i<right.size(); ++i) {
			const QString &s = right.at(i).simplified();
			if (s.isEmpty())
				continue;

			if (question.isEmpty())
				retMap[QStringLiteral("question")] = s;
			else if (question.contains(QStringLiteral("%1")))
				retMap[QStringLiteral("question")] = question.arg(s);
			else
				retMap[QStringLiteral("question")] = question;

			retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

			retMap[QStringLiteral("answer")] = left;

			retMap.insert(generateOne(left, maxOptions));

			// Seed main: 2
			// Seed sub: (block index+1) * 1000 + (answer index + 1)

			const int sub = (idx+1)*1000 + i+1;

			helper.append(retMap, sub, idx+1);
		}

	}

	return helper.getVariantList(true);
}







/**
 * @brief ModuleSelector::preview
 * @return
 */

QVariantMap ModuleSelector::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	Q_UNUSED(commonData);

	QVariantMap m;
	QString s;

	for (const QVariant &v : generatedList) {
		const QVariantMap &m = v.toMap();

		s += QStringLiteral("- **%1** - ").arg(m.value(QStringLiteral("question")).toString());

		const QJsonArray &options = m.value(QStringLiteral("options")).toJsonArray();
		const QString &answer = m.value(QStringLiteral("answer")).toString();
		const int &index = m.value(QStringLiteral("index")).toInt();


		for (int i=0; i<answer.size(); ++i) {
			if (i==index)
				s += QStringLiteral("**")+answer.at(i)+QStringLiteral("**");
			else
				s += answer.at(i);
		}

		s += QStringLiteral(" (");

		for (int i=0; i<options.size(); ++i) {
			s += options.at(i).toString();
			if (i<options.size()-1)
				s += QStringLiteral(",");
		}

		s += QStringLiteral(")\n");
	}

	m[QStringLiteral("text")] = s;

	return m;
}



