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


	if (storage && storage->name() == "plusminus") {
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


	}


	m["title"] = "Calculator";
	m["details"] = storage ? storage->name() : "";
	m["image"] = "";

	return m;
}






/**
 * @brief ModuleCalculator::generate
 * @param data
 * @param storage
 * @return
 */

QVariantMap ModuleCalculator::generate(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answerPtr) const
{
	QVariantMap m;

	if (storage && storage->name() == "plusminus")
		return generatePlusminus(data, storage, storageData, answerPtr);


	m["question"] = "0";
	m["suffix"] = "";
	m["twoLine"] = false;
	m["decimalEnabled"] = false;
	m["xpFactor"] = 0;

	if (answerPtr) {
		(*answerPtr)["first"] = 0;
		(*answerPtr)["second"] = 0;
	}

	return m;
}



/**
 * @brief ModuleCalculator::generatePlusminus
 * @param data
 * @param storage
 * @return
 */

QVariantMap ModuleCalculator::generatePlusminus(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answerPtr) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantMap m;

	bool isSubtract = data.value("subtract", false).toBool();
	int canNegative = data.value("canNegative", 0).toInt();
	bool allCanNegative = canNegative > 1;
	int range = data.value("range", 1).toInt();
	float xpFactor = 1.0;

	int floor = 1, ceil = 1;

	if (range >= 4) {
		xpFactor += 0.4;
		ceil = allCanNegative ? 100 : 101;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -100;
	} else if (range == 3) {
		xpFactor += 0.3;
		ceil = allCanNegative ? 50 : 51;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -50;
	} else if (range == 2) {
		xpFactor += 0.2;
		ceil = allCanNegative ? 20 : 21;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -20;
	} else {
		xpFactor += 0.1;
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



	if (allCanNegative)
		xpFactor = 1.0+(xpFactor-1.0)*2;
	else if (canNegative)
		xpFactor += 0.1;


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
	m["xpFactor"] = xpFactor;

	if (answerPtr) {
		(*answerPtr)["first"] = answer;
		(*answerPtr)["second"] = 0;
	}

	return m;
}
