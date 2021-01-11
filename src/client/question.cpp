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
{


}




Question::Question(GameMap::Objective *objective)
{
	Q_ASSERT(objective);

	m_module = objective->module();
	m_data = objective->data();
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
										{ "icon", "image://font/Material Icons/\ue163" }
									});

	m["truefalse"] = QVariantMap({
									 { "name", QObject::tr("Igaz/hamis") },
									 { "icon", "image://font/Material Icons/\ue163" }
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

	return m;
}



/**
 * @brief Question::objectiveInfo
 * @param module
 * @return
 */

QVariantMap Question::objectiveInfo(const QString &module)
{
	QVariantMap m = objectivesMap().value(module).toMap();

	if (m.isEmpty()) {
		return QVariantMap({
							   { "name", QObject::tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });
	}

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

QStringList Question::objectiveDataToStringList(const QString &module, const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	if (module == "truefalse")
		return toStringListTruefalse(data, storageModule, storageData);
	else if (module == "simplechoice")
		return toStringListSimplechoice(data, storageModule, storageData);

	QStringList l;
	l.append(QObject::tr("Érvénytelen modul!"));
	l.append(module);

	return l;
}


/**
 * @brief Question::objectiveDataToStringList
 * @param module
 * @param dataString
 * @param storageModule
 * @param storageDataString
 * @return
 */

QStringList Question::objectiveDataToStringList(const QString &module, const QString &dataString, const QString &storageModule, const QString &storageDataString)
{
	QVariantMap data = Client::byteArrayToJsonMap(dataString.toUtf8());
	QVariantMap storageData = storageDataString.isEmpty() ? QVariantMap() : Client::byteArrayToJsonMap(storageDataString.toUtf8());
	return objectiveDataToStringList(module, data, storageModule, storageData);
}




/**
 * @brief Question::dataToStringList
 * @param objective
 * @return
 */

QStringList Question::objectiveDataToStringList(GameMap::Objective *objective, GameMap::Storage *storage)
{
	Q_ASSERT(objective);

	if (storage)
		return objectiveDataToStringList(objective->module(), objective->data(), storage->module(), storage->data());
	else
		return objectiveDataToStringList(objective->module(), objective->data());
}




/**
 * @brief Question::generateTruefalse
 * @return
 */

QVariantMap Question::generateTruefalse() const
{
	return m_data;
}


/**
 * @brief Question::toStringListTruefalse
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QStringList Question::toStringListTruefalse(const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	Q_UNUSED(storageModule)
	Q_UNUSED(storageData)

	QStringList l;
	l.append(data.value("question").toString());
	if (data.value("correct").toBool())
		l.append("igaz");
	else
		l.append("hamis");

	return l;
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

	return m;
}



/**
 * @brief Question::toStringListSimplechoice
 * @param data
 * @param storageModule
 * @param storageData
 * @return
 */

QStringList Question::toStringListSimplechoice(const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	Q_UNUSED(storageModule)
	Q_UNUSED(storageData)

	QStringList l;

	l.append(data.value("question").toString());

	QStringList answers = data.value("answers").toStringList();
	l.append(data.value("correct").toString()+" ("+answers.join(",")+")");

	return l;
}


