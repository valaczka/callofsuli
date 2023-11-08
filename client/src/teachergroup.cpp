/*
 * ---- Call of Suli ----
 *
 * teachergroup.cpp
 *
 * Created on: 2023. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroup
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

#include "teachergroup.h"
#include "Logger.h"
#include "clientcache.h"
#include "application.h"
#include "utils_.h"


TeacherGroup::TeacherGroup(QObject *parent)
	: QObject{parent}
	, m_userList(new UserList())
	, m_memberList(new UserList())
	, m_classList(new ClassList())
	, m_campaignList(new CampaignList())
{
	LOG_CTRACE("client") << "TeacherGroup created" << this;

}


/**
 * @brief TeacherGroup::~TeacherGroup
 */

TeacherGroup::~TeacherGroup()
{
	LOG_CTRACE("client") << "TeacherGroup destroyed" << this;
}


/**
 * @brief TeacherGroup::loadFromJson
 * @param object
 * @param allField
 */

void TeacherGroup::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setGroupid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("name")) || allField)
		setName(object.value(QStringLiteral("name")).toString());

	if (object.contains(QStringLiteral("active")) || allField)
		setActive(object.value(QStringLiteral("active")).toInt());

	if (object.contains(QStringLiteral("classList")) || allField) {
		OlmLoader::loadFromJsonArray<ClassObject>(m_classList.get(), object.value(QStringLiteral("classList")).toArray(), "id", "id", true);
		emit fullNameChanged();
	}

	if (object.contains(QStringLiteral("userList")) || allField)
		OlmLoader::loadFromJsonArray<User>(m_userList.get(), object.value(QStringLiteral("userList")).toArray(), "username", "username", true);

	if (object.contains(QStringLiteral("memberList")) || allField) {
		OlmLoader::loadFromJsonArray<User>(m_memberList.get(), object.value(QStringLiteral("memberList")).toArray(), "username", "username", true);
		emit memberListReloaded();
	}

	if (object.contains(QStringLiteral("campaignList")) || allField) {
		OlmLoader::loadFromJsonArray<Campaign>(m_campaignList.get(), object.value(QStringLiteral("campaignList")).toArray(), "id", "campaignid", true);
		emit campaignListReloaded();
	}
}


/**
 * @brief TeacherGroup::reload
 */

void TeacherGroup::reload()
{
	if (m_groupid <= 0)
		return;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("group/%1").arg(m_groupid))
			->fail(this, [](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Letöltés sikertelen"));
	})
			->done(this, std::bind(&TeacherGroup::loadFromJson, this, std::placeholders::_1, true));
}


/**
 * @brief TeacherGroup::reloadAndCall
 * @param v
 */

void TeacherGroup::reloadAndCall(QObject *inst, QJSValue v)
{
	if (m_groupid <= 0)
		return;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("group/%1").arg(m_groupid))
			->fail(inst, [](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Letöltés sikertelen"));
	})
			->done(inst, [this, v](const QJsonObject &o) mutable {
		loadFromJson(o, true);
		if (v.isCallable())
			v.call();
	});
}



/**
 * @brief TeacherGroup::groupid
 * @return
 */

int TeacherGroup::groupid() const
{
	return m_groupid;
}

void TeacherGroup::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();
}

const QString &TeacherGroup::name() const
{
	return m_name;
}

void TeacherGroup::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
	emit fullNameChanged();
}

bool TeacherGroup::active() const
{
	return m_active;
}

void TeacherGroup::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

UserList *TeacherGroup::userList() const
{
	return m_userList.get();
}

UserList *TeacherGroup::memberList() const
{
	return m_memberList.get();
}

ClassList *TeacherGroup::classList() const
{
	return m_classList.get();
}


/**
 * @brief TeacherGroup::fullName
 * @return
 */

QString TeacherGroup::fullName() const
{
	QStringList l;

	for (const ClassObject *c : *m_classList)
		l.append(c->name());

	if (l.isEmpty())
		return m_name;

	std::sort(l.begin(), l.end());
	return QStringLiteral("%1 – %2").arg(m_name, l.join(QStringLiteral(", ")));
}



/**
 * @brief TeacherGroup::campaignList
 * @return
 */

CampaignList *TeacherGroup::campaignList() const
{
	return m_campaignList.get();
}






/**
 * @brief TeacherGroupCampaignResultModel::TeacherGroupCampaignResultModel
 * @param parent
 */

TeacherGroupCampaignResultModel::TeacherGroupCampaignResultModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}


/**
 * @brief TeacherGroupCampaignResultModel::~TeacherGroupCampaignResultModel
 */

TeacherGroupCampaignResultModel::~TeacherGroupCampaignResultModel()
{

}




/**
 * @brief TeacherGroupCampaignResultModel::data
 * @param index
 * @param role
 * @return
 */

QVariant TeacherGroupCampaignResultModel::data(const QModelIndex &index, int role) const
{
	const int &col = index.column();
	const int &row = index.row();

	if (role == Qt::DisplayRole) {											// display
		if (col == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (row <= 0 || row > m_userList.size())
				return QStringLiteral("???");

			User *u = m_userList.at(row-1).user;
			return u ? u->fullName() : QStringLiteral("???");
		} else if (row == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (col <= 0 || col > m_taskList.size())
				return QStringLiteral("???");

			const TaskOrSection &task = m_taskList.at(col-1);

			BaseMapList *ml = m_mapHandler ? reinterpret_cast<BaseMapList*>(m_mapHandler->mapList()) : nullptr;

			if (task.task())
				return task.task()->readableShortCriterion(ml);
			else
				return task.section();
		}

		return QStringLiteral("");

	} else if (role == Qt::CheckStateRole) {								// checked: 0 (false) : 1 (checked) : 2 (required but not checked)
		if (col == 0 && row > 0 && row <= m_userList.size()) {
			User *user = m_userList.at(row-1).user;

			return user->active() ? 1 : 0;
		}

		if (row <= 0 || row > m_userList.size() || col <= 0 || col > m_taskList.size())
			return 0;

		User *user = m_userList.at(row-1).user;
		Task *task = m_taskList.at(col-1).task();

		int idx = findResult(user, task);

		if (idx != -1) {
			if (m_resultList.at(idx).success)
				return 1;
			else if (task && task->required())
				return 2;
			else
				return 0;
		} else
			return 0;

	} else if (role == Qt::UserRole+1) {									// isPlacholder
		if (isSection(col))
			return false;

		if (col == 0 || row == 0)
			return m_showHeaderPlaceholders;
		else
			return m_showCellPlaceholders;

	} else if (role == Qt::UserRole+2) {									// result
		if (col != 0)
			return QStringLiteral("");

		if (row <= 0 || row > m_userList.size())
			return QStringLiteral("???");

		const UserResult &r = m_userList.at(row-1);

		return Task::readableGradeOrXpShort(r.grade, r.xp);

	} else if (role == Qt::UserRole+3) {									// isSection
		return isSection(col);
	}

	return QVariant();
}


/**
 * @brief TeacherGroupCampaignResultModel::roleNames
 * @return
 */

QHash<int, QByteArray> TeacherGroupCampaignResultModel::roleNames() const
{
	return {
		{ Qt::DisplayRole, QByteArrayLiteral("display") },
		{ Qt::CheckStateRole, QByteArrayLiteral("checked") },
		{ Qt::UserRole+1, QByteArrayLiteral("isPlaceholder") },
		{ Qt::UserRole+2, QByteArrayLiteral("result") },
		{ Qt::UserRole+3, QByteArrayLiteral("isSection") },
	};
}


/**
 * @brief TeacherGroupCampaignResultModel::teacherGroup
 * @return
 */

TeacherGroup *TeacherGroupCampaignResultModel::teacherGroup() const
{
	return m_teacherGroup;
}

void TeacherGroupCampaignResultModel::setTeacherGroup(TeacherGroup *newTeacherGroup)
{
	if (m_teacherGroup == newTeacherGroup)
		return;

	if (m_teacherGroup)
		disconnect(m_teacherGroup, &TeacherGroup::memberListReloaded, this, &TeacherGroupCampaignResultModel::reload);

	m_teacherGroup = newTeacherGroup;
	emit teacherGroupChanged();

	if (m_teacherGroup)
		connect(m_teacherGroup, &TeacherGroup::memberListReloaded, this, &TeacherGroupCampaignResultModel::reload);

	reload();
}


/**
 * @brief TeacherGroupCampaignResultModel::campaign
 * @return
 */

Campaign *TeacherGroupCampaignResultModel::campaign() const
{
	return m_campaign;
}

void TeacherGroupCampaignResultModel::setCampaign(Campaign *newCampaign)
{
	if (m_campaign == newCampaign)
		return;

	if (m_campaign)
		disconnect(m_campaign, &Campaign::taskListReloaded, this, &TeacherGroupCampaignResultModel::reload);

	m_campaign = newCampaign;
	emit campaignChanged();

	if (m_campaign)
		connect(m_campaign, &Campaign::taskListReloaded, this, &TeacherGroupCampaignResultModel::reload);

	reload();
}


/**
 * @brief TeacherGroupCampaignResultModel::isSection
 * @param col
 * @return
 */

bool TeacherGroupCampaignResultModel::isSection(const int &col) const
{
	if (col < 1 || col > m_taskList.size())
		return false;

	return !(m_taskList.at(col-1).task());
}



/**
 * @brief TeacherGroupCampaignResultModel::reload
 */

void TeacherGroupCampaignResultModel::reload()
{
	if (!m_teacherGroup || !m_campaign || !m_mapHandler)
		return;

	LOG_CDEBUG("client") << "Reload model" << this;

	beginRemoveColumns(Utils::noParent(), 0, columnCount()-1);
	m_taskList.clear();
	endRemoveColumns();

	beginRemoveRows(Utils::noParent(), 0, rowCount()-1);
	m_userList.clear();
	endRemoveRows();

	setShowHeaderPlaceholders(false);


	if (m_teacherGroup->memberList()->size()) {
		beginInsertRows(Utils::noParent(), 0, m_teacherGroup->memberList()->size()+1);

		for (User *u : *m_teacherGroup->memberList())
			m_userList.append(u);

		std::sort(m_userList.begin(), m_userList.end(), [](const UserResult &lResult, const UserResult &rResult) {
			User *left = lResult.user;
			User *right = rResult.user;

			if (!left || !right)
				return true;

			return (QString::localeAwareCompare(left->fullName(), right->fullName()) < 0);
		});

		endInsertRows();
	}

	if (m_campaign->taskList()->size()) {
		beginInsertColumns(Utils::noParent(), 0, m_campaign->taskList()->size()+1);

		m_taskList = m_campaign->getOrderedTaskList();

		endInsertColumns();
	}

	emit modelReloaded();
}



/**
 * @brief TeacherGroupCampaignResultModel::reloadResult
 */

void TeacherGroupCampaignResultModel::reloadContent()
{
	if (!m_campaign || !m_teacherGroup) {
		LOG_CWARNING("client") << "Missing Campaign or TeacherGroup";
		return;
	}

	Client *client = Application::instance()->client();

	client->send(HttpConnection::ApiTeacher, QStringLiteral("campaign/%1/result").arg(m_campaign->campaignid()))
			->fail(client, [client](const QString &err){client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, &TeacherGroupCampaignResultModel::reloadFromJson);
}




/**
 * @brief TeacherGroupCampaignResultModel::reloadFromJson
 * @param data
 */

void TeacherGroupCampaignResultModel::reloadFromJson(const QJsonObject &data)
{
	const QJsonArray &list = data.value(QStringLiteral("list")).toArray();

	for (const QJsonValue &v : std::as_const(list)) {
		const QJsonObject &obj = v.toObject();
		const QString &username = obj.value(QStringLiteral("username")).toString();
		int gradeid = obj.value(QStringLiteral("resultGrade")).toInt();
		int xp = obj.value(QStringLiteral("resultXP")).toInt();

		int idx = findUser(username);

		Grade *grade = qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"), gradeid));
		User *user = nullptr;

		if (idx != -1) {
			m_userList[idx].grade = grade;
			m_userList[idx].xp = xp;
			user = m_userList.at(idx).user;
		} else {
			LOG_CWARNING("client") << "Invalid user:" << username;
			continue;
		}

		if (obj.contains(QStringLiteral("included"))) {
			user->setActive(obj.value(QStringLiteral("included")).toVariant().toBool());
		} else {
			user->setActive(true);
		}

		const QJsonArray &list = obj.value(QStringLiteral("taskList")).toArray();

		for (const QJsonValue &v : std::as_const(list)) {
			const QJsonObject &obj = v.toObject();

			int taskid = obj.value(QStringLiteral("id")).toInt();
			bool success = obj.value(QStringLiteral("success")).toVariant().toBool();

			int idx = findResult(username, taskid);

			if (idx != -1) {
				m_resultList[idx].success = success;
			} else {
				Task *task = findTask(taskid);

				if (task)
					m_resultList.append({user, task, success});
				else
					LOG_CWARNING("client") << "Invalid task:" << taskid;
			}
		}
	}

	setShowCellPlaceholders(false);

	emit dataChanged(index(0, 0), index(m_userList.size(), m_taskList.size()));
}




/**
 * @brief TeacherGroupCampaignResultModel::findResult
 * @param user
 * @param task
 * @return
 */

int TeacherGroupCampaignResultModel::findResult(const User *user, const Task *task) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const UserTaskResult &r = m_resultList.at(i);

		if (r.user && r.task && r.user == user && r.task == task)
			return i;
	}

	return -1;
}


/**
 * @brief TeacherGroupCampaignResultModel::findResult
 * @param username
 * @param taskid
 * @return
 */

int TeacherGroupCampaignResultModel::findResult(const QString &username, const int &taskid) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const UserTaskResult &r = m_resultList.at(i);

		if (r.user && r.task && r.user->username() == username && r.task->taskid() == taskid)
			return i;
	}

	return -1;
}



/**
 * @brief TeacherGroupCampaignResultModel::findUser
 * @param user
 * @return
 */

int TeacherGroupCampaignResultModel::findUser(const User *user) const
{
	for (int i=0; i<m_userList.size(); ++i) {
		const UserResult &r = m_userList.at(i);

		if (r.user && r.user == user)
			return i;
	}

	return -1;
}




/**
 * @brief TeacherGroupCampaignResultModel::findUser
 * @param username
 * @return
 */

int TeacherGroupCampaignResultModel::findUser(const QString &username) const
{
	for (int i=0; i<m_userList.size(); ++i) {
		const UserResult &r = m_userList.at(i);

		if (r.user && r.user->username() == username)
			return i;
	}

	return -1;
}


/**
 * @brief TeacherGroupCampaignResultModel::findTask
 * @param taskid
 * @return
 */

Task *TeacherGroupCampaignResultModel::findTask(const int &taskid) const
{
	for (int i=0; i<m_taskList.size(); ++i) {
		const TaskOrSection &r = m_taskList.at(i);

		if (r.task() && r.task()->taskid() == taskid)
			return r.task();
	}

	return nullptr;
}



/**
 * @brief TeacherGroupCampaignResultModel::mapHandler
 * @return
 */

TeacherMapHandler *TeacherGroupCampaignResultModel::mapHandler() const
{
	return m_mapHandler;
}

void TeacherGroupCampaignResultModel::setMapHandler(TeacherMapHandler *newMapHandler)
{
	if (m_mapHandler == newMapHandler)
		return;

	if (m_mapHandler)
		disconnect(m_mapHandler, &TeacherMapHandler::reloaded, this, &TeacherGroupCampaignResultModel::reload);

	m_mapHandler = newMapHandler;
	emit mapHandlerChanged();

	if (m_mapHandler)
		connect(m_mapHandler, &TeacherMapHandler::reloaded, this, &TeacherGroupCampaignResultModel::reload);

	reload();
}





/**
 * @brief TeacherGroupCampaignResultModel::showCellPlaceholders
 * @return
 */

bool TeacherGroupCampaignResultModel::showCellPlaceholders() const
{
	return m_showCellPlaceholders;
}

void TeacherGroupCampaignResultModel::setShowCellPlaceholders(bool newShowCellPlaceholders)
{
	if (m_showCellPlaceholders == newShowCellPlaceholders)
		return;
	m_showCellPlaceholders = newShowCellPlaceholders;
	emit showCellPlaceholdersChanged();

	if (m_taskList.size() && m_userList.size()) {
		const QModelIndex &topLeft = index(1, 1);
		const QModelIndex &bottomRight = index(m_userList.size(), m_taskList.size());
		emit dataChanged(topLeft, bottomRight);
	}
}




/**
 * @brief TeacherGroupCampaignResultModel::userAt
 * @param row
 * @return
 */

User *TeacherGroupCampaignResultModel::userAt(const int &row) const
{
	if (m_showHeaderPlaceholders)
		return nullptr;

	if (row >= 0 && row < m_userList.size())
		return m_userList.at(row).user;

	return nullptr;
}




/**
 * @brief TeacherGroupCampaignResultModel::loadCampaignDataFromUser
 * @param campaign
 * @param user
 * @return
 */

bool TeacherGroupCampaignResultModel::loadCampaignDataFromUser(Campaign *campaign, User *user) const
{
	for (int i=0; i<m_userList.size(); ++i) {
		if (m_userList.at(i).user == user)
			return loadCampaignDataFromRow(campaign, i);
	}

	return false;
}



/**
 * @brief TeacherGroupCampaignResultModel::loadCampaignDataFromRow
 * @param campaign
 * @param row
 * @return
 */

bool TeacherGroupCampaignResultModel::loadCampaignDataFromRow(Campaign *campaign, const int &row) const
{
	if (!campaign || !m_campaign) {
		LOG_CERROR("client") << "Invalid campaign";
		return false;
	}

	if (m_showHeaderPlaceholders)
		return false;

	if (row < 0 || row >= m_userList.size())
		return false;

	campaign->setCampaignid(m_campaign->campaignid());
	campaign->setDescription(m_campaign->description());
	campaign->setStartTime(m_campaign->startTime());
	campaign->setEndTime(m_campaign->endTime());
	campaign->setStarted(m_campaign->started());
	campaign->setFinished(m_campaign->finished());
	campaign->setDefaultGrade(m_campaign->defaultGrade());
	campaign->setGroupid(m_campaign->groupid());


	const UserResult &userResult = m_userList.at(row);

	campaign->setResultGrade(userResult.grade);
	campaign->setResultXP(userResult.xp);

	for (Task *task : *m_campaign->taskList()) {
		if (!task)
			continue;

		int idx = findResult(userResult.user, task);
		bool success = false;

		if (idx != -1)
			success = m_resultList.at(idx).success;

		Task *t = campaign->appendTask();
		t->loadFromTask(task);
		t->setSuccess(success);
	}


	return true;
}


/**
 * @brief TeacherGroupCampaignResultModel::showHeaderPlaceholders
 * @return
 */

bool TeacherGroupCampaignResultModel::showHeaderPlaceholders() const
{
	return m_showHeaderPlaceholders;
}

void TeacherGroupCampaignResultModel::setShowHeaderPlaceholders(bool newShowHeaderPlaceholders)
{
	if (m_showHeaderPlaceholders == newShowHeaderPlaceholders)
		return;
	m_showHeaderPlaceholders = newShowHeaderPlaceholders;
	emit showHeaderPlaceholdersChanged();

	if (m_taskList.size())
		emit dataChanged(index(0, 0), index(0, m_taskList.size()));

	if (m_userList.size())
		emit dataChanged(index(0, 0), index(m_userList.size(), 0));
}








/**
 * @brief TeacherGroupResultModel::TeacherGroupResultModel
 * @param parent
 */

TeacherGroupResultModel::TeacherGroupResultModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}


/**
 * @brief TeacherGroupResultModel::~TeacherGroupResultModel
 */

TeacherGroupResultModel::~TeacherGroupResultModel()
{

}



/**
 * @brief TeacherGroupResultModel::data
 * @param index
 * @param role
 * @return
 */

QVariant TeacherGroupResultModel::data(const QModelIndex &index, int role) const
{
	const int &col = index.column();
	const int &row = index.row();

	if (role == Qt::DisplayRole) {											// display
		if (col == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (row <= 0 || row > m_userList.size())
				return QStringLiteral("???");

			User *u = m_userList.at(row-1);
			return u ? u->fullName() : QStringLiteral("???");
		} else if (row == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (col <= 0 || col > m_campaignList.size())
				return QStringLiteral("???");

			Campaign *c = m_campaignList.at(col-1);

			return c ? c->readableName() : QStringLiteral("???");
		}

		return QStringLiteral("");

	} else if (role == Qt::CheckStateRole) {								// campaignState
		if (col == 0 || row == 0)
			return Campaign::Invalid;

		if (col <= 0 || col > m_campaignList.size())
			return Campaign::Invalid;

		Campaign *c = m_campaignList.at(col-1);

		return c ? c->state() : Campaign::Invalid;


	} else if (role == Qt::UserRole+1) {									// result
		TeacherGroupResultResult r;

		if (row <= 0 || row > m_userList.size() || col <= 0 || col > m_campaignList.size())
			return QVariant::fromValue(r);

		User *user = m_userList.at(row-1);
		Campaign *campaign = m_campaignList.at(col-1);

		int idx = findResult(user, campaign);

		if (idx != -1)
			return QVariant::fromValue(m_resultList.at(idx).result);

		return QVariant::fromValue(r);

	} else if (role == Qt::UserRole+2) {									// isPlacholder
		if (col == 0 || row == 0)
			return m_showHeaderPlaceholders;
		else
			return m_showCellPlaceholders;

	}

	return QVariant();
}



/**
 * @brief TeacherGroupResultModel::roleNames
 * @return
 */

QHash<int, QByteArray> TeacherGroupResultModel::roleNames() const
{
	return {
		{ Qt::DisplayRole, QByteArrayLiteral("display") },
		{ Qt::CheckStateRole, QByteArrayLiteral("campaignState") },
		{ Qt::UserRole+1, QByteArrayLiteral("result") },
		{ Qt::UserRole+2, QByteArrayLiteral("isPlaceholder") },
	};
}



/**
 * @brief TeacherGroupResultModel::reload
 */

void TeacherGroupResultModel::reload()
{
	if (!m_teacherGroup)
		return;

	LOG_CDEBUG("client") << "Reload model" << this;

	beginRemoveColumns(Utils::noParent(), 0, columnCount()-1);
	m_campaignList.clear();
	endRemoveColumns();

	beginRemoveRows(Utils::noParent(), 0, rowCount()-1);
	m_userList.clear();
	endRemoveRows();

	setShowHeaderPlaceholders(false);


	if (m_teacherGroup->memberList()->size()) {
		beginInsertRows(Utils::noParent(), 0, m_teacherGroup->memberList()->size()+1);

		for (User *u : *m_teacherGroup->memberList())
			m_userList.append(u);

		std::sort(m_userList.begin(), m_userList.end(), [](User *left, User *right) {
			if (!left || !right)
				return true;

			return (QString::localeAwareCompare(left->fullName(), right->fullName()) < 0);
		});

		endInsertRows();
	}

	if (m_teacherGroup->campaignList()->size()) {
		beginInsertColumns(Utils::noParent(), 0, m_teacherGroup->campaignList()->size()+1);

		for (Campaign *c : *m_teacherGroup->campaignList())
			m_campaignList.append(c);


		std::sort(m_campaignList.begin(), m_campaignList.end(), [](Campaign *left, Campaign *right) {
			if (!left || !right)
				return true;

			if (!left->started() && right->started())
				return false;

			if (left->started() && !right->started())
				return true;

			if (!left->finished() && right->finished())
				return false;

			if (left->finished() && !right->finished())
				return true;

			if (!left->endTime().isValid() && right->endTime().isValid())
				return false;

			if (left->endTime().isValid() && !right->endTime().isValid())
				return true;

			if (left->endTime().isValid() && right->endTime().isValid() && left->endTime() < right->endTime())
				return true;

			if (left->endTime().isValid() && right->endTime().isValid() && left->endTime() > right->endTime())
				return false;

			return (left->campaignid() < right->campaignid());
		});

		endInsertColumns();
	}

	emit modelReloaded();
}


/**
 * @brief TeacherGroupResultModel::reloadContent
 */

void TeacherGroupResultModel::reloadContent()
{
	if (!m_teacherGroup) {
		LOG_CWARNING("client") << "Missing TeacherGroup";
		return;
	}

	Client *client = Application::instance()->client();

	client->send(HttpConnection::ApiTeacher, QStringLiteral("group/%1/result").arg(m_teacherGroup->groupid()))
			->fail(client, [client](const QString &err){client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, &TeacherGroupResultModel::reloadFromJson);
}




/**
 * @brief TeacherGroupResultModel::reloadFromJson
 * @param data
 */

void TeacherGroupResultModel::reloadFromJson(const QJsonObject &data)
{
	const QJsonArray &list = data.value(QStringLiteral("list")).toArray();

	for (const QJsonValue &v : std::as_const(list)) {
		const QJsonObject &obj = v.toObject();

		int campaignid = obj.value(QStringLiteral("campaignid")).toInt();
		const QJsonArray &rList = obj.value(QStringLiteral("resultList")).toArray();

		for (const QJsonValue &v : std::as_const(rList)) {
			const QJsonObject &obj = v.toObject();

			const QString &username = obj.value(QStringLiteral("username")).toString();
			int xp = obj.value(QStringLiteral("resultXP")).toInt();
			int gradeid = obj.value(QStringLiteral("resultGrade")).toInt();
			Grade *grade = qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"), gradeid));

			int idx = findResult(username, campaignid);

			if (idx != -1) {
				m_resultList[idx].result.grade = grade;
				m_resultList[idx].result.xp = xp;
			} else {
				Campaign *campaign = findCampaign(campaignid);
				User *user = findUser(username);

				if (!campaign) {
					LOG_CWARNING("client") << "Invalid campaign:" << campaignid;
					continue;
				}

				if (!user) {
					LOG_CWARNING("client") << "Invalid user:" << username;
					continue;
				}

				m_resultList.append({user, campaign, { grade, xp }});
			}
		}
	}

	setShowCellPlaceholders(false);

	emit dataChanged(index(0, 0), index(m_userList.size(), m_campaignList.size()));
}



/**
 * @brief TeacherGroupResultModel::findResult
 * @param user
 * @param campaign
 * @return
 */

int TeacherGroupResultModel::findResult(const User *user, const Campaign *campaign) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const UserResult &r = m_resultList.at(i);

		if (r.user && r.campaign && r.user == user && r.campaign == campaign)
			return i;
	}

	return -1;
}




/**
 * @brief TeacherGroupResultModel::findResult
 * @param username
 * @param campaignid
 * @return
 */

int TeacherGroupResultModel::findResult(const QString &username, const int &campaignid) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const UserResult &r = m_resultList.at(i);

		if (r.user && r.campaign && r.user->username() == username && r.campaign->campaignid() == campaignid)
			return i;
	}

	return -1;
}



/**
 * @brief TeacherGroupResultModel::findUser
 * @param username
 * @return
 */

User *TeacherGroupResultModel::findUser(const QString &username) const
{
	foreach (User *u, m_userList)
		if (u && u->username() == username)
			return u;

	return nullptr;
}


/**
 * @brief TeacherGroupResultModel::findCampaign
 * @param id
 * @return
 */

Campaign *TeacherGroupResultModel::findCampaign(const int &id) const
{
	foreach (Campaign *c, m_campaignList)
		if (c && c->campaignid() == id)
			return c;

	return nullptr;
}



/**
 * @brief TeacherGroupResultModel::showCellPlaceholders
 * @return
 */

bool TeacherGroupResultModel::showCellPlaceholders() const
{
	return m_showCellPlaceholders;
}

void TeacherGroupResultModel::setShowCellPlaceholders(bool newShowCellPlaceholders)
{
	if (m_showCellPlaceholders == newShowCellPlaceholders)
		return;
	m_showCellPlaceholders = newShowCellPlaceholders;
	emit showCellPlaceholdersChanged();

	if (m_campaignList.size() && m_userList.size()) {
		const QModelIndex &topLeft = index(1, 1);
		const QModelIndex &bottomRight = index(m_userList.size(), m_campaignList.size());
		emit dataChanged(topLeft, bottomRight);
	}
}


/**
 * @brief TeacherGroupResultModel::userAt
 * @param row
 * @return
 */

User *TeacherGroupResultModel::userAt(const int &row) const
{
	if (row >= 0 && row < m_userList.size())
		return m_userList.at(row);

	return nullptr;
}


/**
 * @brief TeacherGroupResultModel::campaignAt
 * @param column
 * @return
 */

Campaign *TeacherGroupResultModel::campaignAt(const int &column) const
{
	if (column >= 0 && column < m_campaignList.size())
		return m_campaignList.at(column);

	return nullptr;
}



/**
 * @brief TeacherGroupResultModel::showHeaderPlaceholders
 * @return
 */

bool TeacherGroupResultModel::showHeaderPlaceholders() const
{
	return m_showHeaderPlaceholders;
}


/**
 * @brief TeacherGroupResultModel::setShowHeaderPlaceholders
 * @param newShowHeaderPlaceholders
 */

void TeacherGroupResultModel::setShowHeaderPlaceholders(bool newShowHeaderPlaceholders)
{
	if (m_showHeaderPlaceholders == newShowHeaderPlaceholders)
		return;
	m_showHeaderPlaceholders = newShowHeaderPlaceholders;
	emit showHeaderPlaceholdersChanged();

	if (m_campaignList.size())
		emit dataChanged(index(0, 0), index(0, m_campaignList.size()));

	if (m_userList.size())
		emit dataChanged(index(0, 0), index(m_userList.size(), 0));
}


/**
 * @brief TeacherGroupResultModel::teacherGroup
 * @return
 */

TeacherGroup *TeacherGroupResultModel::teacherGroup() const
{
	return m_teacherGroup;
}


/**
 * @brief TeacherGroupResultModel::setTeacherGroup
 * @param newTeacherGroup
 */

void TeacherGroupResultModel::setTeacherGroup(TeacherGroup *newTeacherGroup)
{
	if (m_teacherGroup == newTeacherGroup)
		return;

	if (m_teacherGroup) {
		disconnect(m_teacherGroup, &TeacherGroup::memberListReloaded, this, &TeacherGroupResultModel::reload);
		disconnect(m_teacherGroup, &TeacherGroup::campaignListReloaded, this, &TeacherGroupResultModel::reload);
	}

	m_teacherGroup = newTeacherGroup;
	emit teacherGroupChanged();

	if (m_teacherGroup) {
		connect(m_teacherGroup, &TeacherGroup::memberListReloaded, this, &TeacherGroupResultModel::reload);
		connect(m_teacherGroup, &TeacherGroup::campaignListReloaded, this, &TeacherGroupResultModel::reload);
	}

	reload();
}
