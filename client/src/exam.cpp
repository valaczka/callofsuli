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


/**
 * @brief Exam::generateRandom
 */

void Exam::generateRandom(TeacherMapHandler *handler, TeacherGroup *group) const
{
	LOG_CTRACE("client") << "Generate random exams" << group << handler;

	if (!handler || !group)
		return;

	std::unique_ptr<GameMap> dstMap;

	for (const TeacherMap *map : *handler->mapList()) {
		if (!map || !map->downloaded())
			continue;

		const auto &ptr = handler->read(map);

		if (!ptr)
			continue;

		dstMap.reset(GameMap::fromBinaryData(ptr.value()));

		if (!dstMap)
			continue;

		LOG_CINFO("client") << "MAP" << map->uuid() << map->name();
	}

	if (!dstMap) {
		LOG_CERROR("client") << "NO MAP";
		return;
	}

	if (dstMap->missions().isEmpty()) {
		LOG_CERROR("client") << "EMPTY MAP";
		return;
	}

	GameMapMission *mission = dstMap->missions().at(0);
	GameMapMissionLevel *ml = mission->level(1);

	QJsonArray data;

	for (const User *u : *group->memberList()) {
		QJsonObject userdata;
		userdata[QStringLiteral("username")] = u->username();
		userdata[QStringLiteral("q")] = ExamGame::generatePaperQuestions(ml);

		data.append(userdata);
	}

	QJsonObject o;
	o[QStringLiteral("list")] = data;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/create").arg(m_examId),
											o)
			;

}



/**
 * @brief Exam::createPdf
 * @param list
 */

void Exam::createPdf(const QJsonArray &list, TeacherGroup *group) const
{
	ExamGame::PdfConfig config;
	config.examId = m_examId;
	config.title = m_description+QStringLiteral(" (")+group->fullName()+QStringLiteral(")");
	//config.pagePerUser = 3;

	ExamGame::generatePdf(list, config, group);
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
