/*
 * ---- Call of Suli ----
 *
 * modulebinary.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleBinary
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
#include "modulebinary.h"

QVector<int> ModuleBinary::m_numbers;
QVector<int> ModuleBinary::m_optionsRange;


/**
 * @brief ModuleBinary::ModuleBinary
 * @param parent
 */

ModuleBinary::ModuleBinary(QObject *parent) : QObject(parent)
{
	if (m_numbers.isEmpty()) {
		QHash<int, QVector<int>> cnt = {
			{ 1, {} },
			{ 2, {} },
			{ 3, {} },
			{ 4, {} },
			{ 5, {} },
			{ 6, {} },
		};

		for (int i=1; i<=0b111111; ++i) {
			const int pop = __builtin_popcount(i);

			auto it = cnt.find(pop);
			if (it == cnt.end())
				continue;

			it->append(i);
		}

		m_optionsRange.clear();

		int full = 0;

		for (int i : std::vector<int>{3,2,4,5,1,6}) {
			const QVector<int> &v = cnt.value(i);
			m_numbers.append(v);
			full += v.size();
			m_optionsRange.append(full);
		}
	}
}


/**
 * @brief ModuleBinary::testResult
 * @param answer
 * @param success
 * @return
 */

QString ModuleBinary::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	QString html = QStringLiteral("<p>");

	if (success)
		html += QStringLiteral("<span class=\"answer\">");
	else
		html += QStringLiteral("<span class=\"answerFail\">");


	if (answer.contains(QStringLiteral("num"))) {
		if (success)
			html += QStringLiteral("<span class=\"answer\">");
		else
			html += QStringLiteral("<span class=\"answerFail\">");

		html += numberToKey(answer.value(QStringLiteral("num")).toInt());

		html += QStringLiteral("</span>");
	}

	if (const int num = data.value(QStringLiteral("answer")).toInt(); !success && num > 0) {
		html += QStringLiteral(" <span class=\"answerCorrect\">")
				+ numberToKey(num) + QStringLiteral("</span>");
	}

	html += QStringLiteral("</p>");

	return html;
}


/**
 * @brief ModuleBinary::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleBinary::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
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
 * @brief ModuleBinary::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleBinary::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
									   QVariantMap *commonDataPtr, StorageSeed */*seed*/) const
{
	if (storage && storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData, commonDataPtr);
	else if (storage && storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData, commonDataPtr);


	return QVariantList();
}




/**
 * @brief ModuleBinary::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleBinary::generateBinding(const QVariantMap &data, const QVariantMap &storageData,
										   QVariantMap *commonDataPtr) const
{
	QVariantList ret;
	QVariantList commonList;

	const QString &question = data.value(QStringLiteral("question")).toString();
	const bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");

	QVariantList bindingData = storageData.value(QStringLiteral("bindings")).toList();

	if (bindingData.size() > m_numbers.size()) {
		qWarning() << "ModuleBinary data size exceeds range" << bindingData.size() << ">" << m_numbers.size();
		return {};
	}

	int s = 0;

	for (const int i : m_optionsRange) {
		if (i > bindingData.size()) {
			s = i;
			break;
		}
	}

	QVector<int> numbers(m_numbers.constBegin(), m_numbers.constBegin()+(s > 0 ? s : bindingData.size()));

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(numbers.begin(), numbers.end(), g);

	auto numberIterator = numbers.cbegin();

	for (const QVariant &v : bindingData) {
		const QVariantMap &m = v.toMap();
		const QString &left = m.value(QStringLiteral("first")).toString();
		const QString &right = m.value(QStringLiteral("second")).toString();

		const QString &questionPart = isBindToRight ? right : left;
		const QString &dataPart  = isBindToRight ? left : right;

		if (dataPart.isEmpty())
			continue;

		if (questionPart.isEmpty()) {
			if (commonDataPtr)
				commonList.append(QVariantMap{
									  { QStringLiteral("answer"), 0 },
									  { QStringLiteral("data"), dataPart },
								  });


			continue;
		}

		int answer = *numberIterator;
		++numberIterator;

		if (commonDataPtr)
			commonList.append(QVariantMap{
								  { QStringLiteral("answer"), answer },
								  { QStringLiteral("data"), dataPart },
							  });

		QVariantMap retMap;

		if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(questionPart);
		else if (question.isEmpty())
			retMap[QStringLiteral("question")] = questionPart;
		else
			retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+questionPart;

		retMap[QStringLiteral("answer")] = answer;

		ret.append(retMap);
	}

	if (commonDataPtr)
		*commonDataPtr = QVariantMap({
										 { QStringLiteral("list"), commonList }
									 });


	std::shuffle(ret.begin(), ret.end(), g);

	return ret;
}




/**
 * @brief ModuleBinary::generateBlock
 * @param data
 * @param storageData
 * @param commonDataPtr
 * @return
 */

QVariantList ModuleBinary::generateBlock(const QVariantMap &data, const QVariantMap &storageData, QVariantMap *commonDataPtr) const
{
	QVariantList ret;
	QVariantList commonList;

	const QString &question = data.value(QStringLiteral("question")).toString();
	const QVariantList &list = storageData.value(QStringLiteral("blocks")).toList();

	if (list.size() > m_numbers.size()) {
		qWarning() << "ModuleBinary data size exceeds range" << list.size() << ">" << m_numbers.size();
		return {};
	}

	int s = 0;

	for (const int i : m_optionsRange) {
		if (i > list.size()) {
			s = i;
			break;
		}
	}

	QVector<int> numbers(m_numbers.constBegin(), m_numbers.constBegin()+(s > 0 ? s : list.size()));

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(numbers.begin(), numbers.end(), g);

	auto numberIterator = numbers.cbegin();

	for (const QVariant &v : list) {
		const QVariantMap &m = v.toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty())
			continue;

		if (right.isEmpty()) {
			if (commonDataPtr)
				commonList.append(QVariantMap{
									  { QStringLiteral("answer"), 0 },
									  { QStringLiteral("data"), left },
								  });


			continue;
		}

		int answer = *numberIterator;
		++numberIterator;

		if (commonDataPtr)
			commonList.append(QVariantMap{
								  { QStringLiteral("answer"), answer },
								  { QStringLiteral("data"), left },
							  });

		QVariantMap retMap;

		const QString questionPart = right.at(QRandomGenerator::global()->bounded(right.size()));

		if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(questionPart);
		else if (question.isEmpty())
			retMap[QStringLiteral("question")] = questionPart;
		else
			retMap[QStringLiteral("question")] = question+QStringLiteral(" ")+questionPart;

		retMap[QStringLiteral("answer")] = answer;

		ret.append(retMap);
	}

	if (commonDataPtr)
		*commonDataPtr = QVariantMap({
										 { QStringLiteral("list"), commonList }
									 });


	std::shuffle(ret.begin(), ret.end(), g);

	return ret;
}



/**
 * @brief ModuleBinary::preview
 * @return
 */

QVariantMap ModuleBinary::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	QVariantMap m;
	QString s;

	for (const QVariant &v : generatedList) {
		const QVariantMap &m = v.toMap();

		s.append(QStringLiteral("- %1 - **%2**\n")
				 .arg(m.value(QStringLiteral("question")).toString())
				 .arg(numberToKey(m.value(QStringLiteral("answer")).toInt()))
				 );
	}

	if (!commonData.isEmpty()) {
		s.append(QStringLiteral("---\n"));
		const QVariantList list = commonData.value(QStringLiteral("list")).toList();
		for (const QVariant &v : list) {
			const QVariantMap &m = v.toMap();

			const int answer = m.value(QStringLiteral("answer")).toInt();

			if (answer > 0)
				s.append(QStringLiteral("[%1] %2\n\n")
						 .arg(numberToKey(answer))
						 .arg(m.value(QStringLiteral("data")).toString())
						 );
			else
				s.append(m.value(QStringLiteral("data")).toString()).append(QStringLiteral("\n\n"));
		}
	}

	m[QStringLiteral("text")] = s;

	return m;
}



/**
 * @brief ModuleBinary::numberToKey
 * @param number
 * @return
 */

QString ModuleBinary::numberToKey(const int &number)
{
	static const char *letters = "ABCDEF";

	QString s;

	for (int i=0; i<(int) strlen(letters); ++i) {
		const int bit = (1 << i);

		if (number & bit)
			s.append(letters[i]);
	}

	return s;
}





/**
 * @brief ModuleBinary::keyToNumber
 * @param key
 * @return
 */

int ModuleBinary::keyToNumber(const QString &key)
{
	static const char *letters = "ABCDEF";

	int num = 0;

	for (int i=0; i<(int) strlen(letters); ++i) {
		if (key.contains(letters[i]))
			num += (1 << i);
	}

	return num;
}



