/*
 * ---- Call of Suli ----
 *
 * modulecalculator.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleCalculator
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

#include "modulecalculator.h"

ModuleCalculator::ModuleCalculator(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleCalculator::testResult
 * @param success
 * @return
 */

QString ModuleCalculator::testResult(const QVariantMap &, const QVariantMap &answer, const bool &success) const
{
	QString html;

	if (success)
		html = QStringLiteral("<p class=\"answer\">");
	else
		html = QStringLiteral("<p class=\"answerFail\">");

	if (answer.contains(QStringLiteral("first"))) {
		const qreal &fReal = answer.value(QStringLiteral("first"), 0).toReal();
		const int &fInt = answer.value(QStringLiteral("first"), 0).toInt();

		if (fReal == (qreal) fInt)
			html += QString::number(fInt);
		else
			html += QString::number(fReal);
	}

	if (answer.contains(QStringLiteral("second"))) {
		const qreal &fReal = answer.value(QStringLiteral("second"), 0).toReal();
		const int &fInt = answer.value(QStringLiteral("second"), 0).toInt();

		html += QStringLiteral("/");

		if (fReal == (qreal) fInt)
			html += QString::number(fInt);
		else
			html += QString::number(fReal);
	}

	html += QStringLiteral("</p>");

	return html;
}




/**
 * @brief ModuleCalculator::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleCalculator::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storageData)

	QVariantMap m;

	if (!storage) {
		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = QStringLiteral("%1 %2").arg(
					data.value(QStringLiteral("answer")).toString(),
					data.value(QStringLiteral("suffix")).toString());
		m[QStringLiteral("image")] = QLatin1String("");

		return m;
	} else if (storage->name() == QStringLiteral("plusminus")) {
		bool isSubtract = data.value(QStringLiteral("subtract"), false).toBool();
		int canNegative = data.value(QStringLiteral("canNegative"), 0).toInt();
		bool allCanNegative = canNegative > 1;
		int range = data.value(QStringLiteral("range"), 1).toInt();

		if (isSubtract)
			m[QStringLiteral("title")] = QObject::tr("Kivonás");
		else
			m[QStringLiteral("title")] = QObject::tr("Összeadás");

		QString details = QLatin1String("");

		if (range >= 4) {
			if (allCanNegative)
				details += QObject::tr("-100 és 100 között");
			else
				details += QObject::tr("0 és 100 között");
		} else if (range == 3) {
			if (allCanNegative)
				details += QObject::tr("-50 és 50 között");
			else
				details += QObject::tr("0 és 50 között");
		} else if (range == 2) {
			if (allCanNegative)
				details += QObject::tr("-20 és 20 között");
			else
				details += QObject::tr("0 és 20 között");
		} else {
			if (allCanNegative)
				details += QObject::tr("-10 és 10 között");
			else
				details += QObject::tr("0 és 10 között");
		}


		if (!canNegative)
			details += QStringLiteral("<br>")+QObject::tr("nem lehet negatív eredmény");

		m[QStringLiteral("details")] = details;
		m[QStringLiteral("image")] = QLatin1String("");
		return m;
	} else if (storage->name() == QStringLiteral("numbers")) {
		QStringList answers;

		foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QStringLiteral("%1 — %2").arg(left, right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QLatin1String("");

		return m;

	}


	m[QStringLiteral("title")] = QStringLiteral("Calculator");
	m[QStringLiteral("details")] = storage ? storage->name() : QLatin1String("");
	m[QStringLiteral("image")] = QLatin1String("");

	return m;
}



/**
 * @brief ModuleCalculator::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleCalculator::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m;

		bool decimals = data.value(QStringLiteral("decimals"), false).toBool();

		m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("suffix")] = data.value(QStringLiteral("suffix")).toString();
		m[QStringLiteral("twoLine")] = false;
		m[QStringLiteral("decimalEnabled")] = decimals;
		m[QStringLiteral("answer")] = QVariantMap({{QStringLiteral("first"), decimals ? data.value(QStringLiteral("answer")).toReal() : data.value(QStringLiteral("answer")).toInt()}, {QStringLiteral("second"), 0}});

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("plusminus")) {
		QVariantList list;

		for (int i=0; i<20; ++i)
			list.append(generatePlusminus(data));

		return list;
	}


	if (storage->name() == QStringLiteral("numbers"))
		return generateNumbers(data, storageData);

	return QVariantList();
}






/**
 * @brief ModuleCalculator::generatePlusminus
 * @param data
 * @param storage
 * @return
 */

QVariantMap ModuleCalculator::generatePlusminus(const QVariantMap &data) const
{
	QVariantMap m;

	bool isSubtract = data.value(QStringLiteral("subtract"), false).toBool();
	int canNegative = data.value(QStringLiteral("canNegative"), 0).toInt();
	bool allCanNegative = canNegative > 1;
	int range = data.value(QStringLiteral("range"), 1).toInt();

	int floor = 1, ceil = 1;

	if (range >= 4) {
		ceil = allCanNegative ? 100 : 101;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -100;
	} else if (range == 3) {
		ceil = allCanNegative ? 50 : 51;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -50;
	} else if (range == 2) {
		ceil = allCanNegative ? 20 : 21;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -20;
	} else {
		ceil = allCanNegative ? 10 : 11;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -10;
	}


	int answer = 0, number1 = 0, number2 = 0;

	if (!isSubtract) {						// Összeadás
		answer = QRandomGenerator::global()->bounded(floor+1, ceil);
		number2 = QRandomGenerator::global()->bounded(floor, allCanNegative ? ceil : answer);
		number1 = answer-number2;
	} else {								// Kivonás
		answer = QRandomGenerator::global()->bounded(floor, ceil-1);
		if (!allCanNegative && answer < 0) {
			number2 = qAbs(answer) < ceil-1 ?
						  QRandomGenerator::global()->bounded(qAbs(answer)+1, ceil) :
						  ceil-1;
			number1 = answer+number2;  //
		} else {
			number1 = answer < ceil-1 ?
						  QRandomGenerator::global()->bounded(answer+1, ceil) :
						  ceil-1;
			number2 = number1-answer;
		}
	}


	if (number2 < 0)
		m[QStringLiteral("question")] = QStringLiteral("%1 %2 (%3) =")
						.arg(number1)
						.arg(isSubtract ? QStringLiteral("-") : QStringLiteral("+"))
						.arg(number2);
	else
		m[QStringLiteral("question")] = QStringLiteral("%1 %2 %3 =")
						.arg(number1)
						.arg(isSubtract ? QStringLiteral("-") : QStringLiteral("+"))
						.arg(number2);

	m[QStringLiteral("suffix")] = QLatin1String("");
	m[QStringLiteral("twoLine")] = false;
	m[QStringLiteral("decimalEnabled")] = false;
	m[QStringLiteral("answer")] = QVariantMap({{QStringLiteral("first"), answer}, {QStringLiteral("second"), 0}});


	return m;
}



/**
 * @brief ModuleCalculator::generateNumbers
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleCalculator::generateNumbers(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	QString question = data.value(QStringLiteral("question")).toString();

	foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value(QStringLiteral("first")).toString();
		QString right = m.value(QStringLiteral("second")).toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = left;
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(left);
		else
			retMap[QStringLiteral("question")] = question;

		qreal r = m.value(QStringLiteral("second")).toReal();
		/*qreal ri = floor(r);
		bool decimal = !(r-ri == 0.0);*/


		retMap[QStringLiteral("suffix")] = data.value(QStringLiteral("suffix")).toString();
		retMap[QStringLiteral("twoLine")] = false;
		retMap[QStringLiteral("decimalEnabled")] = true;
		retMap[QStringLiteral("answer")] = QVariantMap({{QStringLiteral("first"), r}, {QStringLiteral("second"), 0}});

		ret.append(retMap);
	}

	return ret;
}





/**
 * @brief ModuleCalculator::preview
 * @param generatedList
 * @return
 */

QVariantMap ModuleCalculator::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		s.append(QStringLiteral("- %1 <b>%2 %3</b>\n").arg(m.value(QStringLiteral("question")).toString())
				 .arg(m.value(QStringLiteral("answer")).toMap().value(QStringLiteral("first")).toReal())
				 .arg(m.value(QStringLiteral("suffix")).toString()));
	}

	m[QStringLiteral("text")] = s;

	return m;
}
