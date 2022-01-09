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




Question::Question(GameMapObjective *objective)
{
	m_objective = objective;
}



/**
 * @brief Question::isValid
 * @return
 */

bool Question::isValid() const
{
	if (m_objective)
		return true;

	return false;
}


/**
 * @brief Question::module
 * @return
 */

QString Question::module() const
{
	if (m_objective)
		return m_objective->module();

	return "";
}





/**
 * @brief Question::generate
 * @return
 */

bool Question::generate()
{
	if (!m_objective)
		return false;

	QString module = m_objective->module();

	QVariantMap m;
	m["module"] = module;

	if (!Client::moduleObjectiveList().contains(module)) {
		m["error"] = "invalid module";
		return false;
	}

	ModuleInterface *mi = Client::moduleObjectiveList().value(module);
	ModuleInterface *st = nullptr;
	QVariantMap std;

	if (m_objective->storage()) {
		st = Client::moduleStorageList().value(m_objective->storage()->module(), nullptr);
		std = m_objective->storage()->data();
	}

	m_question = mi->generate(m_objective->data(), st, std, &m_answer);

	return true;
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
	if (!Client::moduleObjectiveList().contains(module)) {
		return QVariantMap({
							   { "name", "" },
							   { "icon", "image://font/Material Icons/\ue002" },
							   { "title", QObject::tr("Érvénytelen modul!") },
							   { "details", "" },
							   { "image", "" }
						   });
	}

	QVariantMap data = Client::byteArrayToJsonMap(dataString.toUtf8());
	QVariantMap storageData = storageDataString.isEmpty() ? QVariantMap() : Client::byteArrayToJsonMap(storageDataString.toUtf8());

	ModuleInterface *mi = Client::moduleObjectiveList().value(module);

	QVariantMap m;
	m["name"] = mi->readableName();
	m["icon"] = mi->icon();
	m.insert(mi->details(data, Client::moduleStorageList().value(storageModule, nullptr), storageData));

	return m;
}


/**
 * @brief Question::storageInfo
 * @param module
 * @param dataString
 * @return
 */

QVariantMap Question::storageInfo(const QString &module, const QString &dataString)
{
	if (!Client::moduleStorageList().contains(module)) {
		return QVariantMap({
							   { "name", "" },
							   { "icon", "image://font/Material Icons/\ue002" },
							   { "title", QObject::tr("Érvénytelen modul!") },
							   { "details", "" },
							   { "image", "" }
						   });
	}

	QVariantMap data = Client::byteArrayToJsonMap(dataString.toUtf8());

	ModuleInterface *mi = Client::moduleStorageList().value(module);

	QVariantMap m;
	m["name"] = mi->readableName();
	m["icon"] = mi->icon();
	m.insert(mi->details(data, nullptr, QVariantMap()));

	return m;
}



/**
 * @brief Question::qml
 * @return
 */

QString Question::qml() const
{
	if (!m_objective)
		return "";

	QString module = m_objective->module();

	if (!Client::moduleObjectiveList().contains(module))
		return "";

	ModuleInterface *mi = Client::moduleObjectiveList().value(module);

	return mi->qmlQuestion();
}














