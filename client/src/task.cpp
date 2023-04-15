/*
 * ---- Call of Suli ----
 *
 * task.cpp
 *
 * Created on: 2023. 04. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Task
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

#include "task.h"
#include "application.h"

Task::Task(QObject *parent)
	: SelectableObject{parent}
{

}


/**
 * @brief Task::loadFromJson
 * @param object
 * @param allField
 */

void Task::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setTaskid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("criterion")) || allField)
		setCriterion(object.value(QStringLiteral("criterion")).toObject());

	if (object.contains(QStringLiteral("mapuuid")) || allField)
		setMapUuid(object.value(QStringLiteral("mapuuid")).toString());

	if (object.contains(QStringLiteral("mapname")) || allField)
		setMapName(object.value(QStringLiteral("mapname")).toString());

	if (object.contains(QStringLiteral("required")) || allField)
		setRequired(object.value(QStringLiteral("required")).toVariant().toBool());

	if (object.contains(QStringLiteral("xp")) || allField)
		setXp(object.value(QStringLiteral("xp")).toInt());

	if (object.contains(QStringLiteral("gradeid")) || allField)
		setGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																								object.value(QStringLiteral("gradeid")).toInt())));
}



/**
 * @brief Task::taskid
 * @return
 */

int Task::taskid() const
{
	return m_taskid;
}

void Task::setTaskid(int newTaskid)
{
	if (m_taskid == newTaskid)
		return;
	m_taskid = newTaskid;
	emit taskidChanged();
}

const QJsonObject &Task::criterion() const
{
	return m_criterion;
}

void Task::setCriterion(const QJsonObject &newCriterion)
{
	if (m_criterion == newCriterion)
		return;
	m_criterion = newCriterion;
	emit criterionChanged();
}

Grade *Task::grade() const
{
	return m_grade;
}

void Task::setGrade(Grade *newGrade)
{
	if (m_grade == newGrade)
		return;
	m_grade = newGrade;
	emit gradeChanged();
}

const QString &Task::mapUuid() const
{
	return m_mapUuid;
}

void Task::setMapUuid(const QString &newMapUuid)
{
	if (m_mapUuid == newMapUuid)
		return;
	m_mapUuid = newMapUuid;
	emit mapUuidChanged();
}

const QString &Task::mapName() const
{
	return m_mapName;
}

void Task::setMapName(const QString &newMapName)
{
	if (m_mapName == newMapName)
		return;
	m_mapName = newMapName;
	emit mapNameChanged();
}

bool Task::required() const
{
	return m_required;
}

void Task::setRequired(bool newRequired)
{
	if (m_required == newRequired)
		return;
	m_required = newRequired;
	emit requiredChanged();
}

int Task::xp() const
{
	return m_xp;
}

void Task::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	m_xp = newXp;
	emit xpChanged();
}
