/*
 * ---- Call of Suli ----
 *
 * exam.cpp
 *
 * Created on: 2023. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Exam
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

#include "exam.h"
#include "Logger.h"
#include "examgame.h"
#include "application.h"
#include "teachergroup.h"

Exam::Exam(QObject *parent)
	: SelectableObject{parent}
{

}

Exam::~Exam()
{

}


/**
 * @brief Exam::loadFromJson
 * @param object
 * @param allField
 */

void Exam::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setExamId(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("mode")) || allField)
		setMode(object.value(QStringLiteral("mode")).toVariant().value<Mode>());

	if (object.contains(QStringLiteral("state")) || allField)
		setState(object.value(QStringLiteral("state")).toVariant().value<State>());

	if (object.contains(QStringLiteral("mapuuid")) || allField)
		setMapUuid(object.value(QStringLiteral("mapuuid")).toString());

	if (object.contains(QStringLiteral("description")) || allField)
		setDescription(object.value(QStringLiteral("description")).toString());

	if (object.contains(QStringLiteral("timestamp")) || allField)
		setTimestamp(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("timestamp")).toInteger()));

	if (object.contains(QStringLiteral("engineData")) || allField)
		setEngineData(object.value(QStringLiteral("engineData")).toObject());

}



int Exam::examId() const
{
	return m_examId;
}

void Exam::setExamId(int newExamId)
{
	if (m_examId == newExamId)
		return;
	m_examId = newExamId;
	emit examIdChanged();
}

Exam::State Exam::state() const
{
	return m_state;
}

void Exam::setState(const State &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();
}

Exam::Mode Exam::mode() const
{
	return m_mode;
}

void Exam::setMode(const Mode &newMode)
{
	if (m_mode == newMode)
		return;
	m_mode = newMode;
	emit modeChanged();
}

QString Exam::mapUuid() const
{
	return m_mapUuid;
}

void Exam::setMapUuid(const QString &newMapUuid)
{
	if (m_mapUuid == newMapUuid)
		return;
	m_mapUuid = newMapUuid;
	emit mapUuidChanged();
}

QString Exam::description() const
{
	return m_description;
}

void Exam::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

QDateTime Exam::timestamp() const
{
	return m_timestamp;
}

void Exam::setTimestamp(const QDateTime &newTimestamp)
{
	if (m_timestamp == newTimestamp)
		return;
	m_timestamp = newTimestamp;
	emit timestampChanged();
}

QJsonObject Exam::engineData() const
{
	return m_engineData;
}

void Exam::setEngineData(const QJsonObject &newEngineData)
{
	if (m_engineData == newEngineData)
		return;
	m_engineData = newEngineData;
	emit engineDataChanged();
}
