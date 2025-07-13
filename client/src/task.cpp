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
#include "campaign.h"

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

	if (object.contains(QStringLiteral("mapname")) || allField)
		setMapName(object.value(QStringLiteral("mapname")).toString());

	if (object.contains(QStringLiteral("criterion")) || allField)
		setCriterion(object.value(QStringLiteral("criterion")).toObject());

	if (object.contains(QStringLiteral("mapuuid")) || allField)
		setMapUuid(object.value(QStringLiteral("mapuuid")).toString());

	if (object.contains(QStringLiteral("required")) || allField)
		setRequired(object.value(QStringLiteral("required")).toVariant().toBool());

	if (object.contains(QStringLiteral("xp")) || allField)
		setXp(object.value(QStringLiteral("xp")).toInt());

	if (object.contains(QStringLiteral("gradeid")) || allField)
		setGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																						 object.value(QStringLiteral("gradeid")).toInt())));

	if (object.contains(QStringLiteral("success")) || allField)
		setSuccess(object.value(QStringLiteral("success")).toVariant().toBool());

	if (object.contains(QStringLiteral("result")) || allField)
		setResult(object.value(QStringLiteral("result")).toVariant().toFloat());
}




/**
 * @brief Task::loadFromTask
 * @param task
 */

void Task::loadFromTask(Task *task)
{
	if (!task)
		return;

	setTaskid(task->taskid());
	setMapName(task->mapName());
	setCriterion(task->criterion());
	setMapUuid(task->mapUuid());
	setRequired(task->required());
	setXp(task->xp());
	setGrade(task->grade());
	setSuccess(task->success());
	setResult(task->result());
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
	const int pts = m_criterion.value(QStringLiteral("pts")).toInt(-1);
	return readableGradeOrXp(m_grade, m_xp, pts);
}



/**
 * @brief Task::readableGradeOrXp
 * @param grade
 * @param xp
 * @return
 */

QString Task::readableGradeOrXp(Grade *grade, const int xp, const int pts)
{
	QStringList t;

	if (pts >= 0)
		t.append(tr("%1 pont").arg(pts));

	if (grade)
		t.append(QStringLiteral("%1 (%2)").arg(grade->longname(), grade->shortname()));

	if (xp > 0)
		t.append(tr("%1 XP").arg(xp));

	return t.join(QStringLiteral(", "));
}


/**
 * @brief Task::readableGradeOrXpShort
 * @param grade
 * @param xp
 * @return
 */

QString Task::readableGradeOrXpShort(Grade *grade, const int xp, const int pts)
{
	QStringList t;

	if (pts >= 0)
		t.append(tr("%1 p").arg(pts));

	if (xp > 0)
		t.append(tr("%1 XP").arg(xp));

	if (grade)
		t.append(grade->shortname());

	return t.join(QStringLiteral(" / "));
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

QString Task::readableCriterion(BaseMapList *mapList, Campaign *campaign) const
{
	const QString &module = m_criterion.value(QStringLiteral("module")).toString();
	const int &num = m_criterion.value(QStringLiteral("num")).toInt();
	//const int &pts = m_criterion.value(QStringLiteral("pts")).toInt(0);

	if (module == QLatin1String("xp")) {
		return tr("Gyűjts össze %1 XP-t").arg(num);
	} else if (module == QLatin1String("mapmission")) {
		return tr("Teljesíts %1 különböző küldetést a %2 pályán").arg(num).arg(m_mapName);
	} else if (module == QLatin1String("mission")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();
		const int &level = m_criterion.value(QStringLiteral("level")).toInt(1);
		const bool &deathmatch = m_criterion.value(QStringLiteral("deathmatch")).toVariant().toBool();

		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				for (const QJsonValue &v : std::as_const(list)) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		if (campaign && campaign->usedMapUuids().size() < 2)
			return tr("Teljesítsd a %1 küldetést LEVEL %2%3 szinten").arg(missionName)
					.arg(level)
					.arg(deathmatch ? tr(" SUDDEN DEATH") : QString());
		else
			return tr("Teljesítsd a %1 pálya %2 küldetést LEVEL %3%4 szinten").arg(m_mapName, missionName)
					.arg(level)
					.arg(deathmatch ? tr(" SUDDEN DEATH") : QString());
	} else if (module == QLatin1String("levels")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();

		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				for (const QJsonValue &v : std::as_const(list)) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		if (campaign && campaign->usedMapUuids().size() < 2)
			return tr("Teljesíts %1 szintet a %2 küldetésben")
					.arg(num)
					.arg(missionName);
		else
			return tr("Teljesíts %1 szintet a %2 pálya %3 küldetésben")
					.arg(num)
					.arg(m_mapName)
					.arg(missionName);
	}

	return tr("-- Érvénytelen modul: %1 --").arg(module);
}


/**
 * @brief Task::readableShortCriterion
 * @param mapList
 * @return
 */

QString Task::readableShortCriterion(BaseMapList *mapList, Campaign *campaign) const
{
	const QString &module = m_criterion.value(QStringLiteral("module")).toString();
	const int &num = m_criterion.value(QStringLiteral("num")).toInt();

	if (module == QLatin1String("xp")) {
		return tr("%1 XP").arg(num);
	} else if (module == QLatin1String("mapmission")) {
		return tr("%1 (%2 db)").arg(m_mapName).arg(num);
	} else if (module == QLatin1String("mission")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();
		const int &level = m_criterion.value(QStringLiteral("level")).toInt(1);

		const QString &mapName = m_mapName;
		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				for (const QJsonValue &v : std::as_const(list)) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		if (campaign && campaign->usedMapUuids().size() < 2)
			return tr("%1 level %2").arg(missionName)
					.arg(level);
		else
			return tr("%1 / %2 level %3").arg(mapName, missionName)
					.arg(level);

	} else if (module == QLatin1String("levels")) {
		const QString &mission = m_criterion.value(QStringLiteral("mission")).toString();

		const QString &mapName = m_mapName;
		QString missionName = tr("???");

		if (mapList) {
			QObject *o = OlmLoader::find(mapList, "uuid", m_mapUuid);
			BaseMap *m = qobject_cast<BaseMap*>(o);

			if (m) {
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				for (const QJsonValue &v : std::as_const(list)) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == mission) {
						missionName = o.value(QStringLiteral("name")).toString();
						break;
					}
				}
			}
		}

		if (campaign && campaign->usedMapUuids().size() < 2)
			return tr("%1 levels: %2").arg(missionName)
					.arg(num);
		else
			return tr("%1 / %2 levels: %3").arg(mapName)
					.arg(missionName)
					.arg(num);
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

qreal Task::result() const
{
	return m_result;
}

void Task::setResult(qreal newResult)
{
	if (qFuzzyCompare(m_result, newResult))
		return;
	m_result = newResult;
	emit resultChanged();
}
