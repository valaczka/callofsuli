/*
 * ---- Call of Suli ----
 *
 * task.h
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

#ifndef TASK_H
#define TASK_H

#include "basemap.h"
#include "grade.h"
#include "qjsonobject.h"
#include <selectableobject.h>
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Task;
using TaskList = qolm::QOlm<Task>;
Q_DECLARE_METATYPE(TaskList*)

class Campaign;

/**
 * @brief The Task class
 */

class Task : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int taskid READ taskid WRITE setTaskid NOTIFY taskidChanged)
	Q_PROPERTY(QJsonObject criterion READ criterion WRITE setCriterion NOTIFY criterionChanged)
	Q_PROPERTY(Grade *grade READ grade WRITE setGrade NOTIFY gradeChanged)
	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged)
	Q_PROPERTY(QString mapName READ mapName WRITE setMapName NOTIFY mapNameChanged)
	Q_PROPERTY(bool required READ required WRITE setRequired NOTIFY requiredChanged)
	Q_PROPERTY(int xp READ xp WRITE setXp NOTIFY xpChanged)
	Q_PROPERTY(QString readableGradeOrXp READ readableGradeOrXp NOTIFY readableGradeOrXpChanged)
	Q_PROPERTY(int gradeValue READ gradeValue NOTIFY gradeValueChanged)
	Q_PROPERTY(bool success READ success WRITE setSuccess NOTIFY successChanged)

public:
	explicit Task(QObject *parent = nullptr);
	virtual ~Task() {}

	void loadFromJson(const QJsonObject &object, const bool &allField = true);
	void loadFromTask(Task *task);

	int taskid() const;
	void setTaskid(int newTaskid);

	const QJsonObject &criterion() const;
	void setCriterion(const QJsonObject &newCriterion);

	Grade *grade() const;
	void setGrade(Grade *newGrade);

	const QString &mapUuid() const;
	void setMapUuid(const QString &newMapUuid);

	const QString &mapName() const;
	void setMapName(const QString &newMapName);

	bool required() const;
	void setRequired(bool newRequired);

	int xp() const;
	void setXp(int newXp);

	QString readableGradeOrXp() const;

	static QString readableGradeOrXp(Grade *grade, int xp);
	static QString readableGradeOrXpShort(Grade *grade, int xp);

	int gradeValue() const;

	Q_INVOKABLE QString readableCriterion(BaseMapList *mapList, Campaign *campaign = nullptr) const;
	Q_INVOKABLE QString readableShortCriterion(BaseMapList *mapList, Campaign *campaign = nullptr) const;

	bool success() const;
	void setSuccess(bool newSuccess);

signals:
	void taskidChanged();
	void criterionChanged();
	void gradeChanged();
	void mapUuidChanged();
	void mapNameChanged();
	void requiredChanged();
	void xpChanged();
	void readableGradeOrXpChanged();
	void gradeValueChanged();
	void successChanged();

private:
	int m_taskid = 0;
	QJsonObject m_criterion;
	Grade *m_grade = nullptr;
	QString m_mapUuid;
	QString m_mapName;
	bool m_required = false;
	bool m_success = false;
	int m_xp = -1;
};

#endif // TASK_H
