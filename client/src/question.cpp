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
#include "gamemap.h"
#include "../modules/interfaces.h"




Question::Question(StorageSeed *seed, GameMapObjective *objective)
	: m_objective (objective)
	, m_seed(seed)
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

	return QString();
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

		if (m_seed) {
			m_seed->setCurrentStorage(m_objective->storage()->id());
			m_seed->setCurrentMap(m_objective->map() ? m_objective->map()->uuid() : QStringLiteral("invalid"));
		}
	}

	QVariantMap q;

	if (m_objective->storage() && m_seed) {
		QVariantList tmp = mi->generateAll(m_objective->data(), st, std, &m_objective->commonData(), m_seed);

		if (tmp.isEmpty())
			return QVariantMap();

		q = tmp.first().toMap();

		m_seed->setData(q, m_objective->storage()->id());
	} else {
		if (m_objective->generatedQuestions().isEmpty())
			m_objective->generatedQuestions() = mi->generateAll(m_objective->data(), st, std, &m_objective->commonData(), nullptr);

		if (m_objective->generatedQuestions().isEmpty())
			return QVariantMap();

		q = m_objective->generatedQuestions().takeFirst().toMap();
	}

	q.insert(QStringLiteral("xpFactor"), mi->xpFactor());
	q.insert(QStringLiteral("examPoint"), m_objective->examPoint());

	return q;
}



QVariantMap Question::commonData() const
{
	return m_objective ? m_objective->commonData() : QVariantMap();
}




/**
 * @brief Question::convertToMonospace
 * @param text
 * @return
 */


QString Question::convertToMonospace(const QString &text)
{
	static const QRegularExpression exp(R"(`([^`]*)`)");

	QString t = text;
	t.replace(exp, QStringLiteral(R"(<font face="Ubuntu Mono">\1</font>)"));

	return t;
}

QString Question::monspaceTagStart()
{
	return QStringLiteral(R"(<font face="Ubuntu Mono">)");
}

QString Question::monspaceTagEnd()
{
	return QStringLiteral(R"(</font>)");
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
							   { QStringLiteral("name"), QString() },
							   { QStringLiteral("icon"), QStringLiteral("qrc:/Qaterial/Icons/alert.svg") },
							   { QStringLiteral("title"), QObject::tr("Érvénytelen modul!") },
							   { QStringLiteral("details"), QString() },
							   { QStringLiteral("image"), QString() }
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
							   { QStringLiteral("name"), QString() },
							   { QStringLiteral("icon"), QStringLiteral("qrc:/Qaterial/Icons/alert.svg") },
							   { QStringLiteral("title"), QObject::tr("Érvénytelen modul!") },
							   { QStringLiteral("details"), QString() },
							   { QStringLiteral("image"), QString() }
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
		return QString();

	QString module = m_objective->module();

	if (!Application::instance()->objectiveModules().contains(module))
		return QString();

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
		return QString();
}














