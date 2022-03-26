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
		m["title"] = data.value("question").toString();
		m["details"] = QString("%1 %2").arg(data.value("answer").toString()).arg(data.value("suffix").toString());
		m["image"] = "";

		return m;
	} else if (storage->name() == "plusminus") {
		bool isSubtract = data.value("subtract", false).toBool();
		int canNegative = data.value("canNegative", 0).toInt();
		bool allCanNegative = canNegative > 1;
		int range = data.value("range", 1).toInt();

		if (isSubtract)
			m["title"] = QObject::tr("Kivonás");
		else
			m["title"] = QObject::tr("Összeadás");

		QString details = "";

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
			details += "<br>"+QObject::tr("nem lehet negatív eredmény");

		m["details"] = details;
		m["image"] = "";
		return m;
	} else if (storage->name() == "numbers") {
		QStringList answers;

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QString("%1 — %2").arg(left).arg(right));
		}

		QVariantMap m;
		m["title"] = data.value("question").toString();
		m["details"] = answers.join(", ");
		m["image"] = "";

		return m;

	}


	m["title"] = "Calculator";
	m["details"] = storage ? storage->name() : "";
	m["image"] = "";

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

		bool decimals = data.value("decimals", false).toBool();

		m["question"] = data.value("question").toString();
		m["suffix"] = data.value("suffix").toString();
		m["twoLine"] = false;
		m["decimalEnabled"] = decimals;
		m["answer"] = QVariantMap({{"first", decimals ? data.value("answer").toReal() : data.value("answer").toInt()}, {"second", 0}});

		list.append(m);

		return list;
	}

	if (storage->name() == "plusminus") {
		QVariantList list;

		for (int i=0; i<20; ++i)
			list.append(generatePlusminus(data));

		return list;
	}


	if (storage->name() == "numbers")
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

	bool isSubtract = data.value("subtract", false).toBool();
	int canNegative = data.value("canNegative", 0).toInt();
	bool allCanNegative = canNegative > 1;
	int range = data.value("range", 1).toInt();

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
		m["question"] = QString("%1 %2 (%3) =")
						.arg(number1)
						.arg(isSubtract ? "-" : "+")
						.arg(number2);
	else
		m["question"] = QString("%1 %2 %3 =")
						.arg(number1)
						.arg(isSubtract ? "-" : "+")
						.arg(number2);

	m["suffix"] = "";
	m["twoLine"] = false;
	m["decimalEnabled"] = false;
	m["answer"] = QVariantMap({{"first", answer}, {"second", 0}});


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

	QString question = data.value("question").toString();

	foreach (QVariant v, storageData.value("bindings").toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value("first").toString();
		QString right = m.value("second").toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap["question"] = left;
		else if (question.contains("%1"))
			retMap["question"] = question.arg(left);
		else
			retMap["question"] = question;

		qreal r = m.value("second").toReal();
		/*qreal ri = floor(r);
		bool decimal = !(r-ri == 0.0);*/


		retMap["suffix"] = data.value("suffix").toString();
		retMap["twoLine"] = false;
		retMap["decimalEnabled"] = true;
		retMap["answer"] = QVariantMap({{"first", r}, {"second", 0}});

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

		s.append(QString("- %1 <b>%2 %3</b>\n").arg(m.value("question").toString())
				 .arg(m.value("answer").toMap().value("first").toReal())
				 .arg(m.value("suffix").toString()));
	}

	m["text"] = s;

	return m;
}
