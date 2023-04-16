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
#include <selectableobject.h>
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Campaign;
using CampaignList = qolm::QOlm<Campaign>;
Q_DECLARE_METATYPE(CampaignList*)


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

	Q_INVOKABLE QStringList usedMapUuids() const;

signals:
	void campaignidChanged();
	void startTimeChanged();
	void endTimeChanged();
	void descriptionChanged();
	void startedChanged();
	void finishedChanged();
	void defaultGradeChanged();
	void stateChanged();
	void readableNameChanged();

private:
	int m_campaignid = 0;
	QDateTime m_startTime;
	QDateTime m_endTime;
	QString m_description;
	bool m_started = false;
	bool m_finished = false;
	Grade *m_defaultGrade = nullptr;
	TaskList *const m_taskList;
};

#endif // CAMPAIGN_H
