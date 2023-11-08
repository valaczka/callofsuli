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
#include "abstractlevelgame.h"
#include "clientcache.h"
#include "application.h"
#include "qjsonobject.h"

Campaign::Campaign(QObject *parent)
	: SelectableObject{parent}
	, m_taskList(new TaskList(this))
{
	connect(this, &Campaign::startedChanged, this, &Campaign::stateChanged);
	connect(this, &Campaign::finishedChanged, this, &Campaign::stateChanged);
	connect(m_taskList.get(), &qolm::QOlmBase::countChanged, this, &Campaign::stateChanged);
}


/**
 * @brief Campaign::~Campaign
 */

Campaign::~Campaign()
{

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

	if (object.contains(QStringLiteral("started")) || allField)
		setStarted(object.value(QStringLiteral("started")).toVariant().toBool());

	if (object.contains(QStringLiteral("starttime")) || allField) {
		setStartTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("starttime")).toInt()));
		if (!object.contains(QStringLiteral("started")))
			setStarted(m_startTime <= QDateTime::currentDateTime());
	}

	if (object.contains(QStringLiteral("endtime")) || allField)
		setEndTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("endtime")).toInt()));


	if (object.contains(QStringLiteral("finished")) || allField)
		setFinished(object.value(QStringLiteral("finished")).toVariant().toBool());

	if (object.contains(QStringLiteral("defaultGrade")) || allField)
		setDefaultGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																								object.value(QStringLiteral("defaultGrade")).toInt())));

	if (object.contains(QStringLiteral("taskList")) || allField) {
		OlmLoader::loadFromJsonArray<Task>(m_taskList.get(), object.value(QStringLiteral("taskList")).toArray(), "id", "taskid", true);
		emit taskListReloaded();
	}

	if (object.contains(QStringLiteral("resultGrade")) || allField)
		setResultGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																							   object.value(QStringLiteral("resultGrade")).toInt())));

	if (object.contains(QStringLiteral("resultXP")) || allField)
		setResultXP(object.value(QStringLiteral("resultXP")).toInt());

	if (object.contains(QStringLiteral("groupid")) || allField)
		setGroupid(object.value(QStringLiteral("groupid")).toInt());
}



/**
 * @brief Campaign::addTaskFromTask
 * @param orig
 * @return
 */

Task *Campaign::appendTask()
{
	Task *t = new Task(m_taskList.get());
	m_taskList->append(t);
	return t;
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
	emit readableNameChanged();
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
	return m_taskList.get();
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


/**
 * @brief Campaign::readableName
 * @return
 */

QString Campaign::readableName() const
{
	if (m_description.isEmpty())
		return tr("Kihívás •%1").arg(m_campaignid);
	else
		return m_description;

}



/**
 * @brief Campaign::usedMapUuids
 * @return
 */

QStringList Campaign::usedMapUuids() const
{
	QStringList list;

	for (const Task *t : *m_taskList) {
		if (!t->mapUuid().isEmpty() && !list.contains(t->mapUuid()))
			list.append(t->mapUuid());
	}

	return list;
}


/**
 * @brief Campaign::readableResult
 * @param grade
 * @param xp
 * @return
 */

QString Campaign::readableResult(Grade *grade, int xp)
{
	if (!grade && xp <= 0 && !m_finished)
		return Task::readableGradeOrXp(m_defaultGrade, -1);
	else
		return Task::readableGradeOrXp(grade, xp);
}


/**
 * @brief Campaign::readableShortResult
 * @param grade
 * @param xp
 * @return
 */

QString Campaign::readableShortResult(Grade *grade, int xp)
{
	if (!grade && xp <= 0 && !m_finished)
		return Task::readableGradeOrXpShort(m_defaultGrade, -1);
	else
		return Task::readableGradeOrXpShort(grade, xp);
}


/**
 * @brief Campaign::hasRequiredTask
 * @return
 */

bool Campaign::hasRequiredTask() const
{
	for (Task *t : *m_taskList) {
		if (t && t->required())
			return true;
	}

	return false;
}



/**
 * @brief Campaign::getOrderedTaskList
 * @return
 */

QList<TaskOrSection> Campaign::getOrderedTaskList() const
{
	QVector<Task*> tlist;

	for (Task *t : *m_taskList)
		tlist.append(t);

	std::sort(tlist.begin(), tlist.end(), [](Task *left, Task *right) {
		if (!left || !right)
			return true;

		if (left->grade()) {
			if (!right->grade())
				return true;

			if (left->gradeValue() < right->gradeValue())
				return true;

			if (left->gradeValue() > right->gradeValue())
				return false;
		} else {
			if (right->grade())
				return false;
		}

		if (left->xp() < right->xp())
			return true;

		if (left->xp() > right->xp())
			return false;

		return (left->taskid() < right->taskid());
	});


	// Add sections

	QList<TaskOrSection> list;

	QVector<Task*>::iterator prev;

	for (auto it = tlist.begin(); it != tlist.end(); ++it) {
		if (it == tlist.begin()) {
			const QString &s = *it ? (*it)->readableGradeOrXp() : QStringLiteral("");
			list.append(TaskOrSection(nullptr, s));
			list.append(TaskOrSection(*it));
			prev = it;
			continue;
		}

		const QString &prevS = *prev ? (*prev)->readableGradeOrXp() : QStringLiteral("");
		const QString &s = *it ? (*it)->readableGradeOrXp() : QStringLiteral("");

		if (prevS != s)
			list.append(TaskOrSection(nullptr, s));

		list.append(TaskOrSection(*it));

		prev = it;
	}

	return list;
}



/**
 * @brief Campaign::getOrderedTaskListAsVariant
 * @return
 */

QVariantList Campaign::getOrderedTaskListModel() const
{
	QVariantList list;

	foreach (const TaskOrSection &t, getOrderedTaskList()) {
		list.append(QVariantMap{
						{ QStringLiteral("section"), t.section() },
						{ QStringLiteral("task"), QVariant::fromValue(t.task()) }
					});
	}

	return list;
}

int Campaign::groupid() const
{
	return m_groupid;
}

void Campaign::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();
}




Grade *Campaign::resultGrade() const
{
	return m_resultGrade;
}

void Campaign::setResultGrade(Grade *newResultGrade)
{
	if (m_resultGrade == newResultGrade)
		return;
	m_resultGrade = newResultGrade;
	emit resultGradeChanged();
}

int Campaign::resultXP() const
{
	return m_resultXP;
}

void Campaign::setResultXP(int newResultXP)
{
	if (m_resultXP == newResultXP)
		return;
	m_resultXP = newResultXP;
	emit resultXPChanged();
}





/**
 * @brief StudentCampaignOffsetModel::StudentCampaignOffsetModel
 * @param parent
 */


StudentCampaignOffsetModel::StudentCampaignOffsetModel(QObject *parent)
	: OffsetModel(parent)
{
	setFields({
				  QStringLiteral("timestamp"),
				  QStringLiteral("mapid"),
				  QStringLiteral("missionid"),
				  QStringLiteral("level"),
				  QStringLiteral("mode"),
				  QStringLiteral("deathmatch"),
				  QStringLiteral("success"),
				  QStringLiteral("duration"),
				  QStringLiteral("xp"),
				  QStringLiteral("readableMap"),
				  QStringLiteral("readableMission"),
				  QStringLiteral("medal"),
				  QStringLiteral("username"),
				  QStringLiteral("givenName"),
				  QStringLiteral("familyName"),
			  });
}



/**
 * @brief StudentCampaignOffsetModel::getListFromJson
 * @param obj
 * @return
 */

QVariantList StudentCampaignOffsetModel::getListFromJson(const QJsonObject &obj)
{
	QVariantList list;

	for (const QJsonValue &v : obj.value(listField()).toArray()) {
		QJsonObject obj = v.toObject();

		if (m_mapList) {
			QObject *o = OlmLoader::find(m_mapList, "uuid", obj.value(QStringLiteral("mapid")).toString());
			BaseMap *m = qobject_cast<BaseMap*>(o);

			QString mission, medal;

			if (m) {
				obj[QStringLiteral("readableMap")] = m->name();
				const QJsonArray &list = m->cache().value(QStringLiteral("missions")).toArray();
				for (const QJsonValue &v : std::as_const(list)) {
					const QJsonObject &o = v.toObject();
					if (o.value(QStringLiteral("uuid")).toString() == obj.value(QStringLiteral("missionid")).toString()) {
						mission = o.value(QStringLiteral("name")).toString();
						medal = o.value(QStringLiteral("medal")).toString();
						break;
					}
				}
			} else {
				obj[QStringLiteral("readableMap")] = tr("???");
			}

			obj[QStringLiteral("readableMission")] = mission.isEmpty() ? tr("???") : mission;
			obj[QStringLiteral("medal")] = medal.isEmpty() ? QStringLiteral("") : AbstractLevelGame::medalImagePath(medal);
		}

		QVariantMap m = obj.toVariantMap();

		m[QStringLiteral("timestamp")] = QDateTime::fromSecsSinceEpoch(obj.value(QStringLiteral("timestamp")).toInt());

		list.append(m);
	}

	return list;
}




/**
 * @brief StudentCampaignOffsetModel::setApi
 */

void StudentCampaignOffsetModel::_setApi()
{
	if (m_groupid > -1 && !m_username.isEmpty()) {
		setApi(HttpConnection::ApiTeacher);
		setPath(QStringLiteral("group/%1/result/%2").arg(m_groupid).arg(m_username));
	} else if (m_groupid > -1 && m_username.isEmpty()) {
		setApi(HttpConnection::ApiTeacher);
		setPath(QStringLiteral("group/%1/log").arg(m_groupid));
	} else if (m_username.isEmpty()) {
		setApi(m_campaign ? HttpConnection::ApiUser : HttpConnection::ApiInvalid);
		setPath(m_campaign ? QStringLiteral("campaign/%1/result").arg(m_campaign->campaignid()) : QStringLiteral(""));
	} else {
		setApi(m_campaign ? HttpConnection::ApiTeacher : HttpConnection::ApiInvalid);
		setPath(m_campaign ? QStringLiteral("campaign/%1/result/%2").arg(m_campaign->campaignid()).arg(m_username) : QStringLiteral(""));
	}
}


/**
 * @brief StudentCampaignOffsetModel::groupid
 * @return
 */

int StudentCampaignOffsetModel::groupid() const
{
	return m_groupid;
}

void StudentCampaignOffsetModel::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();

	_setApi();
}



/**
 * @brief StudentCampaignOffsetModel::username
 * @return
 */

const QString &StudentCampaignOffsetModel::username() const
{
	return m_username;
}

void StudentCampaignOffsetModel::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();

	_setApi();
}



/**
 * @brief StudentCampaignOffsetModel::mapList
 * @return
 */

BaseMapList *StudentCampaignOffsetModel::mapList() const
{
	return m_mapList;
}

void StudentCampaignOffsetModel::setMapList(BaseMapList *newMapList)
{
	if (m_mapList == newMapList)
		return;
	m_mapList = newMapList;
	emit mapListChanged();
}



/**
 * @brief StudentCampaignOffsetModel::campaign
 * @return
 */

Campaign *StudentCampaignOffsetModel::campaign() const
{
	return m_campaign;
}


/**
 * @brief StudentCampaignOffsetModel::setCampaign
 * @param newCampaign
 */

void StudentCampaignOffsetModel::setCampaign(Campaign *newCampaign)
{
	if (m_campaign == newCampaign)
		return;
	m_campaign = newCampaign;
	emit campaignChanged();

	_setApi();
}
