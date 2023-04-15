/*
 * ---- Call of Suli ----
 *
 * campaign.cpp
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

#include "campaign.h"
#include "clientcache.h"
#include "application.h"
#include "qjsonobject.h"

Campaign::Campaign(QObject *parent)
	: SelectableObject{parent}
	, m_taskList(new TaskList(this))
{
	connect(this, &Campaign::startedChanged, this, &Campaign::stateChanged);
	connect(this, &Campaign::finishedChanged, this, &Campaign::stateChanged);
	connect(m_taskList, &qolm::QOlmBase::countChanged, this, &Campaign::stateChanged);
}


/**
 * @brief Campaign::~Campaign
 */

Campaign::~Campaign()
{
	delete m_taskList;
}



/**
 * @brief Campaign::loadFromJson
 * @param object
 * @param allField
 */

void Campaign::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setCampaignid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("description")) || allField)
		setDescription(object.value(QStringLiteral("description")).toString());

	if (object.contains(QStringLiteral("starttime")) || allField)
		setStartTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("starttime")).toInt()));

	if (object.contains(QStringLiteral("endtime")) || allField)
		setEndTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("endtime")).toInt()));

	if (object.contains(QStringLiteral("started")) || allField)
		setStarted(object.value(QStringLiteral("started")).toVariant().toBool());

	if (object.contains(QStringLiteral("finished")) || allField)
		setFinished(object.value(QStringLiteral("finished")).toVariant().toBool());

	if (object.contains(QStringLiteral("defaultGrade")) || allField)
		setDefaultGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																								object.value(QStringLiteral("defaultGrade")).toInt())));

	if (object.contains(QStringLiteral("taskList")) || allField)
		OlmLoader::loadFromJsonArray<Task>(m_taskList, object.value(QStringLiteral("taskList")).toArray(), "id", "taskid", true);
}



/**
 * @brief Campaign::campaignid
 * @return
 */

int Campaign::campaignid() const
{
	return m_campaignid;
}

void Campaign::setCampaignid(int newCampaignid)
{
	if (m_campaignid == newCampaignid)
		return;
	m_campaignid = newCampaignid;
	emit campaignidChanged();
}

const QDateTime &Campaign::startTime() const
{
	return m_startTime;
}

void Campaign::setStartTime(const QDateTime &newStartTime)
{
	if (m_startTime == newStartTime)
		return;
	m_startTime = newStartTime;
	emit startTimeChanged();
}

const QDateTime &Campaign::endTime() const
{
	return m_endTime;
}

void Campaign::setEndTime(const QDateTime &newEndTime)
{
	if (m_endTime == newEndTime)
		return;
	m_endTime = newEndTime;
	emit endTimeChanged();
}

const QString &Campaign::description() const
{
	return m_description;
}

void Campaign::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

bool Campaign::started() const
{
	return m_started;
}

void Campaign::setStarted(bool newStarted)
{
	if (m_started == newStarted)
		return;
	m_started = newStarted;
	emit startedChanged();
}

bool Campaign::finished() const
{
	return m_finished;
}

void Campaign::setFinished(bool newFinished)
{
	if (m_finished == newFinished)
		return;
	m_finished = newFinished;
	emit finishedChanged();
}

Grade *Campaign::defaultGrade() const
{
	return m_defaultGrade;
}

void Campaign::setDefaultGrade(Grade *newDefaultGrade)
{
	if (m_defaultGrade == newDefaultGrade)
		return;
	m_defaultGrade = newDefaultGrade;
	emit defaultGradeChanged();
}

TaskList *Campaign::taskList() const
{
	return m_taskList;
}


/**
 * @brief Campaign::state
 * @return
 */

Campaign::State Campaign::state() const
{
	if (m_finished)
		return Finished;

	if (m_started)
		return Running;

	if (m_taskList->size())
		return Prepared;

	return Invalid;
}


