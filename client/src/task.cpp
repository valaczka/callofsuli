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

	if (object.contains(QStringLiteral("success")) || allField)
		setSuccess(object.value(QStringLiteral("success")).toVariant().toBool());
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
	emit readableGradeOrXpChanged();
	emit gradeValueChanged();
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
	emit readableGradeOrXpChanged();
}


/**
 * @brief Task::readableGradeOrXp
 * @return
 */

QString Task::readableGradeOrXp() const
{
	return readableGradeOrXp(m_grade, m_xp);
}



/**
 * @brief Task::readableGradeOrXp
 * @param grade
 * @param xp
 * @return
 */

QString Task::readableGradeOrXp(Grade *grade, int xp)
{
	if (grade && xp > 0)
		return tr("%1 (%2), %3 XP").arg(grade->longname()).arg(grade->shortname()).arg(xp);
	else if (grade)
		return QStringLiteral("%1 (%2)").arg(grade->longname(), grade->shortname());
	else if (xp > 0)
		return tr("%1 XP").arg(xp);

	return QLatin1String("");
}


/**
 * @brief Task::readableGradeOrXpShort
 * @param grade
 * @param xp
 * @return
 */

QString Task::readableGradeOrXpShort(Grade *grade, int xp)
{
	if (grade && xp > 0)
		return tr("%2 XP / %1").arg(grade->shortname()).arg(xp);
	else if (grade)
		return grade->shortname();
	else if (xp > 0)
		return tr("%1 XP").arg(xp);

	return QLatin1String("");
}



/**
 * @brief Task::gradeValue
 * @return
 */

int Task::gradeValue() const
{
	return m_grade ? m_grade->value() : -1;
}




/**
 * @brief Task::readableCriterion
 * @param mapList
 * @return
 */

QString Task::readableCriterion(BaseMapList *mapList) const
{
	const QString &module = m_criterion.value(QStringLiteral("module")).toString();
	const int &num = m_criterion.value(QStringLiteral("num")).toInt();

	if (module == QLatin1String("xp")) {
		return tr("Gyűjts össze %1 XP-t").arg(num);
	} else if (module == QLatin1String("mission")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();
		const int &level = m_criterion.value(QStringLiteral("level")).toInt(1);
		const bool &deathmatch = m_criterion.value(QStringLiteral("deathmatch")).toVariant().toBool();

		const QString &mapName = m_mapName;
		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				foreach (const QJsonValue &v, list) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		return tr("Teljesítsd a %1 pálya %2 küldetést LEVEL %3%4 szinten").arg(mapName, missionName)
				.arg(level)
				.arg(deathmatch ? tr(" SUDDEN DEATH") : QLatin1String(""));
	}

	return tr("-- Érvénytelen modul: %1 --").arg(module);
}


/**
 * @brief Task::readableShortCriterion
 * @param mapList
 * @return
 */

QString Task::readableShortCriterion(BaseMapList *mapList) const
{
	const QString &module = m_criterion.value(QStringLiteral("module")).toString();
	const int &num = m_criterion.value(QStringLiteral("num")).toInt();

	if (module == QLatin1String("xp")) {
		return tr("%1 XP").arg(num);
	} else if (module == QLatin1String("mission")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();
		const int &level = m_criterion.value(QStringLiteral("level")).toInt(1);
		const bool &deathmatch = m_criterion.value(QStringLiteral("deathmatch")).toVariant().toBool();

		const QString &mapName = m_mapName;
		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				foreach (const QJsonValue &v, list) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		return tr("%1 / %2 level %3%4").arg(mapName, missionName)
				.arg(level)
				.arg(deathmatch ? tr(" S D") : QLatin1String(""));
	}

	return tr("-- Érvénytelen --");
}



bool Task::success() const
{
	return m_success;
}

void Task::setSuccess(bool newSuccess)
{
	if (m_success == newSuccess)
		return;
	m_success = newSuccess;
	emit successChanged();
}
