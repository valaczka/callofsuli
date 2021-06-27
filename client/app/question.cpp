/*
 * ---- Call of Suli ----
 *
 * question.cpp
 *
 * Created on: 2021. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Question
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "question.h"
#include "cosclient.h"


Question::Question(const QString &module, const QVariantMap &data)
	: m_module(module)
	, m_data(data)
	, m_storageNum(0)
	, m_objective(nullptr)
{


}




Question::Question(GameMap::Objective *objective, const int &storageNum)
{
	Q_ASSERT(objective);

	m_module = objective->module();
	m_data = objective->data();
	m_storageNum = storageNum;
	m_objective = objective;
}





/**
 * @brief Question::generate
 * @return
 */

QVariantMap Question::generate() const
{
	QVariantMap m;
	if (m_module == "truefalse")
		m = generateTruefalse();
	else if (m_module == "simplechoice")
		m = generateSimplechoice();
	else if (m_module == "calculator")
		m = generateCalculator();

	m["module"] = m_module;
	return m;
}



/**
 * @brief Question::objectiveMap
 * @return
 */

QVariantMap Question::objectivesMap()
{
	QVariantMap m;

	m["simplechoice"] = QVariantMap({
										{ "name", QObject::tr("Egyszerű választás") },
										{ "icon", "image://font/Material Icons/\ue1db" }
									});

	m["truefalse"] = QVariantMap({
									 { "name", QObject::tr("Igaz/hamis") },
									 { "icon", "image://font/School/\uf124" }
								 });

	m["calculator"] = QVariantMap({
									  { "name", QObject::tr("Számolás") },
									  { "icon", "image://font/AcademicI/\uf127" }
								  });

	return m;
}




/**
 * @brief Question::storagesMap
 * @return
 */

QVariantMap Question::storagesMap()
{
	QVariantMap m;

	m["quiz"] = QVariantMap({
								{ "name", QObject::tr("Kvíz") },
								{ "icon", "image://font/Material Icons/\ue163" }
							});

	m["plusminus"] = QVariantMap({
									 { "name", QObject::tr("Összeadás-kivonás") },
									 { "icon", "image://font/Material Icons/\ue163" }
								 });

	return m;
}



/**
 * @brief Question::objectiveInfo
 * @param module
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QVariantMap Question::objectiveInfo(const QString &module, const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	QVariantMap m = objectivesMap().value(module).toMap();

	if (m.isEmpty()) {
		return QVariantMap({
							   { "name", QObject::tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });
	}

	m.insert(objectiveData(module, data, storageModule, storageData));

	return m;
}


/**
 * @brief Question::objectiveInfo
 * @param module
 * @param dataString
 * @param storageModule
 * @param storageDataString
 * @return
 */

QVariantMap Question::objectiveInfo(const QString &module, const QString &dataString, const QString &storageModule, const QString &storageDataString)
{
	QVariantMap m = objectivesMap().value(module).toMap();

	if (m.isEmpty()) {
		return QVariantMap({
							   { "name", QObject::tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });
	}

	m.insert(objectiveData(module, dataString, storageModule, storageDataString));

	return m;
}


/**
 * @brief Question::storageInfo
 * @param module
 * @return
 */

QVariantMap Question::storageInfo(const QString &module)
{
	QVariantMap m = storagesMap().value(module).toMap();

	if (m.isEmpty()) {
		return QVariantMap({
							   { "name", QObject::tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });
	}

	return m;
}




/**
 * @brief Question::dataToStringList
 * @param module
 * @param data
 * @return
 */

QVariantMap Question::objectiveData(const QString &module, const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	QVariantMap m;
	m["title"] = "";
	m["details"] = "";
	m["image"] = "";

	if (module == "truefalse")
		return toMapTruefalse(data, storageModule, storageData);
	else if (module == "simplechoice")
		return toMapSimplechoice(data, storageModule, storageData);
	else if (module == "calculator")
		return toMapCalculator(data, storageModule, storageData);

	return m;
}


/**
 * @brief Question::objectiveDataToStringList
 * @param module
 * @param dataString
 * @param storageModule
 * @param storageDataString
 * @return
 */

QVariantMap Question::objectiveData(const QString &module, const QString &dataString, const QString &storageModule, const QString &storageDataString)
{
	QVariantMap data = Client::byteArrayToJsonMap(dataString.toUtf8());
	QVariantMap storageData = storageDataString.isEmpty() ? QVariantMap() : Client::byteArrayToJsonMap(storageDataString.toUtf8());
	return objectiveData(module, data, storageModule, storageData);
}




/**
 * @brief Question::dataToStringList
 * @param objective
 * @return
 */

QVariantMap Question::objectiveData(GameMap::Objective *objective, GameMap::Storage *storage)
{
	Q_ASSERT(objective);

	if (storage)
		return objectiveData(objective->module(), objective->data(), storage->module(), storage->data());
	else
		return objectiveData(objective->module(), objective->data());
}




/**
 * @brief Question::generateTruefalse
 * @return
 */

QVariantMap Question::generateTruefalse() const
{
	QVariantMap m = m_data;

	m["xpFactor"] = 1.0;

	return m;
}


/**
 * @brief Question::toStringListTruefalse
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QVariantMap Question::toMapTruefalse(const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	Q_UNUSED(storageModule)
	Q_UNUSED(storageData)

	QVariantMap m;
	m["title"] = data.value("question").toString();
	m["details"] = data.value("correct").toBool() ? QObject::tr("igaz") : QObject::tr("hamis");
	m["image"] = "";

	return m;

}




/**
 * @brief Question::generateSimplechoice
 * @return
 */

QVariantMap Question::generateSimplechoice() const
{
	QVariantMap m;

	m["question"] = m_data.value("question").toString();

	QString correct = m_data.value("correct").toString();

	if (correct.isEmpty())
		return m;

	QStringList alist = m_data.value("answers").toStringList();

	QVector<QPair<QString, bool>> options;
	options.append(qMakePair(correct, true));

	while (options.size() < 4 && alist.size()) {
		if (alist.size() == 1) {
			options.append(qMakePair(alist.at(0), false));
			alist.clear();
			break;
		}

		QString o = alist.takeAt(QRandomGenerator::global()->bounded(alist.size()));

		options.append(qMakePair(o, false));
	}

	QVariantList oList;

	while (options.size()) {
		QPair<QString, bool> p;
		if (options.size() > 1) {
			p = options.takeAt(QRandomGenerator::global()->bounded(options.size()));
		} else {
			p = options.at(0);
			options.clear();
		}

		QVariantMap o;
		o["answer"] = p.first;
		o["correct"] = p.second;
		oList.append(o);
	}


	m["options"] = oList;
	m["xpFactor"] = 1.1;

	return m;
}



/**
 * @brief Question::toStringListSimplechoice
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QVariantMap Question::toMapSimplechoice(const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	Q_UNUSED(storageModule)
	Q_UNUSED(storageData)

	QStringList answers = data.value("answers").toStringList();

	QVariantMap m;
	m["title"] = data.value("question").toString();
	m["details"] = data.value("correct").toString()+"<br>("+answers.join(", ")+")";
	m["image"] = "";

	return m;
}




/**
 * @brief Question::generateCalculator
 * @return
 */

QVariantMap Question::generateCalculator() const
{
	QVariantMap m;

	GameMap::Storage *storage = nullptr;

	if (m_objective && m_objective->storage())
		storage = m_objective->storage();

	if (storage && storage->module() == "plusminus")
		return generateCalculatorPlusMinus(storage);


	m["question"] = "0";
	m["suffix"] = "";
	m["twoLine"] = false;
	m["decimalEnabled"] = false;
	m["answer"] = 0;
	m["answer2"] = 0;
	m["xpFactor"] = 0;

	return m;

}



/**
 * @brief Question::toStringListCalculator
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QVariantMap Question::toMapCalculator(const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	Q_UNUSED(storageData)

	QVariantMap m;


	if (storageModule == "plusminus") {
		bool isSubtract = data.value("subtract", false).toBool();
		int canNegative = data.value("canNegative", 0).toInt();
		bool allCanNegative = canNegative > 1;
		int range = data.value("range", 1).toInt();

		if (isSubtract)
			m["title"] = QObject::tr("Kivonás");
		else
			m["title"] = QObject::tr("Összeadás");

		QString details = "";

		if (range >= 3) {
			if (allCanNegative)
				details += QObject::tr("-100 és 100 között");
			else
				details += QObject::tr("0 és 100 között");
		} else if (range == 2) {
			if (allCanNegative)
				details += QObject::tr("-50 és 50 között");
			else
				details += QObject::tr("0 és 50 között");
		} else if (range == 3) {
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
	m["details"] = storageModule;
	m["image"] = "";

	return m;
}



/**
 * @brief Question::generateCalculatorPlusMinus
 * @return
 */

QVariantMap Question::generateCalculatorPlusMinus(GameMap::Storage *storage) const
{
	Q_ASSERT(storage);

	QVariantMap m;

	bool isSubtract = m_data.value("subtract", false).toBool();
	int canNegative = m_data.value("canNegative", 0).toInt();
	bool allCanNegative = canNegative > 1;
	int range = m_data.value("range", 1).toInt();
	float xpFactor = 1.0;

	int floor = 1, ceil = 1;

	if (range >= 3) {
		xpFactor += 0.4;
		ceil = allCanNegative ? 100 : 101;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -100;
	} else if (range == 2) {
		xpFactor += 0.3;
		ceil = allCanNegative ? 50 : 51;
		if (allCanNegative || (isSubtract && canNegative))
			floor = -50;
	} else if (range == 3) {
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
	m["answer"] = answer;
	m["answer2"] = 0;
	m["xpFactor"] = xpFactor;

	return m;
}


