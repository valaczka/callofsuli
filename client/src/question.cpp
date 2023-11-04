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
#include "application.h"
#include "../modules/interfaces.h"




Question::Question(GameMapObjective *objective)
	: m_objective (objective)
{
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

	return QStringLiteral("");
}





/**
 * @brief Question::generate
 * @return
 */

QVariantMap Question::generate() const
{
	if (!m_objective)
		return QVariantMap();

	QString module = m_objective->module();

	if (!Application::instance()->objectiveModules().contains(module)) {
		return QVariantMap();
	}

	ModuleInterface *mi = Application::instance()->objectiveModules().value(module);
	ModuleInterface *st = nullptr;
	QVariantMap std;

	if (m_objective->storage()) {
		st = Application::instance()->storageModules().value(m_objective->storage()->module(), nullptr);
		std = m_objective->storage()->data();
	}

	if (m_objective->generatedQuestions().isEmpty())
		m_objective->generatedQuestions() = mi->generateAll(m_objective->data(), st, std);

	QVariantMap q = m_objective->generatedQuestions().takeAt(QRandomGenerator::global()->bounded(m_objective->generatedQuestions().size())).toMap();

	q.insert(QStringLiteral("xpFactor"), mi->xpFactor());

	return q;
}




/**
 * @brief Question::objectiveInfo
 * @param module
 * @param dataString
 * @param storageModule
 * @param storageDataString
 * @return
 */

QVariantMap Question::objectiveInfo(const QString &module, const QVariantMap &data, const QString &storageModule, const QVariantMap &storageData)
{
	if (!Application::instance()->objectiveModules().contains(module)) {
		return QVariantMap({
							   { QStringLiteral("name"), QStringLiteral("") },
							   { QStringLiteral("icon"), QStringLiteral("image://font/Material Icons/\ue002") },
							   { QStringLiteral("title"), QObject::tr("Érvénytelen modul!") },
							   { QStringLiteral("details"), QStringLiteral("") },
							   { QStringLiteral("image"), QStringLiteral("") }
						   });
	}

	ModuleInterface *mi = Application::instance()->objectiveModules().value(module);

	QVariantMap m;
	m[QStringLiteral("name")] = mi->readableName();
	m[QStringLiteral("icon")] = mi->icon();
	m.insert(mi->details(data, Application::instance()->storageModules().value(storageModule, nullptr), storageData));

	return m;
}


/**
 * @brief Question::storageInfo
 * @param module
 * @param dataString
 * @return
 */

QVariantMap Question::storageInfo(const QString &module, const QVariantMap &data)
{
	if (!Application::instance()->storageModules().contains(module)) {
		return QVariantMap({
							   { QStringLiteral("name"), QStringLiteral("") },
							   { QStringLiteral("icon"), QStringLiteral("image://font/Material Icons/\ue002") },
							   { QStringLiteral("title"), QObject::tr("Érvénytelen modul!") },
							   { QStringLiteral("details"), QStringLiteral("") },
							   { QStringLiteral("image"), QStringLiteral("") }
						   });
	}

	ModuleInterface *mi = Application::instance()->storageModules().value(module);

	QVariantMap m;
	m[QStringLiteral("name")] = mi->readableName();
	m[QStringLiteral("icon")] = mi->icon();
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
		return QStringLiteral("");

	QString module = m_objective->module();

	if (!Application::instance()->objectiveModules().contains(module))
		return QStringLiteral("");

	ModuleInterface *mi = Application::instance()->objectiveModules().value(module);

	return mi->qmlQuestion();
}


/**
 * @brief Question::uuid
 * @return
 */

QString Question::uuid() const
{
	if (m_objective)
		return m_objective->uuid();
	else
		return QStringLiteral("");
}














