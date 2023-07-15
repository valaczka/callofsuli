/*
 * ---- Call of Suli ----
 *
 * campaign.h
 *
 * Created on: 2023. 04. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Campaign
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

#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "grade.h"
#include "qdatetime.h"
#include "task.h"
#include <QPointer>
#include <selectableobject.h>
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "offsetmodel.h"

class Campaign;
using CampaignList = qolm::QOlm<Campaign>;
Q_DECLARE_METATYPE(CampaignList*)

class TaskOrSection;

/**
 * @brief The Campaign class
 */

class Campaign : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int campaignid READ campaignid WRITE setCampaignid NOTIFY campaignidChanged)
	Q_PROPERTY(QDateTime startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
	Q_PROPERTY(QDateTime endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(bool started READ started WRITE setStarted NOTIFY startedChanged)
	Q_PROPERTY(bool finished READ finished WRITE setFinished NOTIFY finishedChanged)
	Q_PROPERTY(Grade *defaultGrade READ defaultGrade WRITE setDefaultGrade NOTIFY defaultGradeChanged)
	Q_PROPERTY(TaskList *taskList READ taskList CONSTANT)
	Q_PROPERTY(State state READ state NOTIFY stateChanged)
	Q_PROPERTY(QString readableName READ readableName NOTIFY readableNameChanged)
	Q_PROPERTY(Grade *resultGrade READ resultGrade WRITE setResultGrade NOTIFY resultGradeChanged)
	Q_PROPERTY(int resultXP READ resultXP WRITE setResultXP NOTIFY resultXPChanged)
	Q_PROPERTY(int groupid READ groupid WRITE setGroupid NOTIFY groupidChanged)

public:
	explicit Campaign(QObject *parent = nullptr);
	virtual ~Campaign();

	enum State {
		Invalid = 0,
		Prepared,
		Running,
		Finished
	};

	Q_ENUM(State);

	Q_INVOKABLE void loadFromJson(const QJsonObject &object, const bool &allField = true);
	Q_INVOKABLE Task *appendTask();

	int campaignid() const;
	void setCampaignid(int newCampaignid);

	const QDateTime &startTime() const;
	void setStartTime(const QDateTime &newStartTime);

	const QDateTime &endTime() const;
	void setEndTime(const QDateTime &newEndTime);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	bool started() const;
	void setStarted(bool newStarted);

	bool finished() const;
	void setFinished(bool newFinished);

	Grade *defaultGrade() const;
	void setDefaultGrade(Grade *newDefaultGrade);

	TaskList *taskList() const;

	State state() const;

	QString readableName() const;

	Grade *resultGrade() const;
	void setResultGrade(Grade *newResultGrade);

	int resultXP() const;
	void setResultXP(int newResultXP);

	Q_INVOKABLE QStringList usedMapUuids() const;
	Q_INVOKABLE QString readableResult(Grade *grade, int xp);
	Q_INVOKABLE QString readableShortResult(Grade *grade, int xp);

	Q_INVOKABLE bool hasRequiredTask() const;

	QList<TaskOrSection> getOrderedTaskList() const;
	Q_INVOKABLE QVariantList getOrderedTaskListModel() const;

	int groupid() const;
	void setGroupid(int newGroupid);

signals:
	void taskListReloaded();
	void campaignidChanged();
	void startTimeChanged();
	void endTimeChanged();
	void descriptionChanged();
	void startedChanged();
	void finishedChanged();
	void defaultGradeChanged();
	void stateChanged();
	void readableNameChanged();
	void resultGradeChanged();
	void resultXPChanged();

	void groupidChanged();

private:
	int m_campaignid = 0;
	QDateTime m_startTime;
	QDateTime m_endTime;
	QString m_description;
	bool m_started = false;
	bool m_finished = false;
	Grade *m_defaultGrade = nullptr;
	Grade *m_resultGrade = nullptr;
	int m_resultXP = -1;
	TaskList *const m_taskList;
	int m_groupid = -1;
};




/**
 * @brief The TaskOrSection class
 */

class TaskOrSection
{
public:
	TaskOrSection(Task *task = nullptr, const QString &section = QString()) : m_task(task), m_section(section) {}
	~TaskOrSection() {}

	Task *task() const { return m_task; }
	const QString &section() const { return m_section; }

private:
	QPointer<Task> m_task;
	QString m_section;

};



/**
 * @brief The StudentCampaignOffsetModel class
 */


class StudentCampaignOffsetModel : public OffsetModel
{
	Q_OBJECT

	Q_PROPERTY(Campaign *campaign READ campaign WRITE setCampaign NOTIFY campaignChanged)
	Q_PROPERTY(BaseMapList *mapList READ mapList WRITE setMapList NOTIFY mapListChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(int groupid READ groupid WRITE setGroupid NOTIFY groupidChanged)

public:
	explicit StudentCampaignOffsetModel(QObject *parent = nullptr);

	Campaign *campaign() const;
	void setCampaign(Campaign *newCampaign);

	BaseMapList *mapList() const;
	void setMapList(BaseMapList *newMapList);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	int groupid() const;
	void setGroupid(int newGroupid);

signals:
	void campaignChanged();
	void mapListChanged();
	void usernameChanged();
	void groupidChanged();

protected:
	virtual QVariantList getListFromJson(const QJsonObject &obj) override;

private:
	void _setApi();

private:
	Campaign *m_campaign = nullptr;
	BaseMapList *m_mapList = nullptr;
	QString m_username;
	int m_groupid = -1;
};

#endif // CAMPAIGN_H
