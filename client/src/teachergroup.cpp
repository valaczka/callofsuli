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


TeacherGroup::TeacherGroup(QObject *parent)
	: QObject{parent}
	, m_userList(new UserList(this))
	, m_memberList(new UserList(this))
	, m_classList(new ClassList(this))
	, m_campaignList(new CampaignList(this))
{
	LOG_CTRACE("client") << "TeacherGroup created" << this;

}


/**
 * @brief TeacherGroup::~TeacherGroup
 */

TeacherGroup::~TeacherGroup()
{
	delete m_campaignList;
	delete m_classList;
	delete m_userList;
	delete m_memberList;

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
		OlmLoader::loadFromJsonArray<ClassObject>(m_classList, object.value(QStringLiteral("classList")).toArray(), "id", "id", true);
		emit fullNameChanged();
	}

	if (object.contains(QStringLiteral("userList")) || allField)
		OlmLoader::loadFromJsonArray<User>(m_userList, object.value(QStringLiteral("userList")).toArray(), "username", "username", true);

	if (object.contains(QStringLiteral("memberList")) || allField) {
		OlmLoader::loadFromJsonArray<User>(m_memberList, object.value(QStringLiteral("memberList")).toArray(), "username", "username", true);
		emit memberListReloaded();
	}

	if (object.contains(QStringLiteral("campaignList")) || allField)
		OlmLoader::loadFromJsonArray<Campaign>(m_campaignList, object.value(QStringLiteral("campaignList")).toArray(), "id", "campaignid", true);
}


/**
 * @brief TeacherGroup::reload
 */

void TeacherGroup::reload()
{
	if (m_groupid <= 0)
		return;

	Application::instance()->client()->send(WebSocket::ApiTeacher, QStringLiteral("group/%1").arg(m_groupid))
			->fail([](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Letöltés sikertelen"));
	})
			->done(std::bind(&TeacherGroup::loadFromJson, this, std::placeholders::_1, true));
}


/**
 * @brief TeacherGroup::reloadAndCall
 * @param v
 */

void TeacherGroup::reloadAndCall(QJSValue v)
{
	if (m_groupid <= 0)
		return;

	Application::instance()->client()->send(WebSocket::ApiTeacher, QStringLiteral("group/%1").arg(m_groupid))
			->fail([](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Letöltés sikertelen"));
	})
			->done([this, v](const QJsonObject &o) mutable {
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
	return m_userList;
}

UserList *TeacherGroup::memberList() const
{
	return m_memberList;
}

ClassList *TeacherGroup::classList() const
{
	return m_classList;
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
	return m_campaignList;
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

	if (role == Qt::DisplayRole) {
		if (col == 0) {
			if (row <= 0 || row > m_userList.size())
				return QStringLiteral("???");

			User *u = m_userList.at(row-1);
			return u->fullName();
		} else if (row == 0) {
			if (col <= 0 || col > m_taskList.size())
				return QStringLiteral("???");

			const TaskOrSection &task = m_taskList.at(col-1);

			if (task.task())
				return task.task()->readableCriterion(nullptr) + " " + QString::number(task.task()->gradeValue())+ " " +
						QString::number(task.task()->xp());
			else
				return task.section();
		}

		return QString("%1, %2").arg(index.column()).arg(index.row());
	}

	return QVariant();
}


/**
 * @brief TeacherGroupCampaignResultModel::roleNames
 * @return
 */

QHash<int, QByteArray> TeacherGroupCampaignResultModel::roleNames() const
{
	return { { Qt::DisplayRole, QByteArrayLiteral("display") } };
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
	if (col < 1 || col >= m_taskList.size())
		return false;

	return !(m_taskList.at(col-1).task());
}



/**
 * @brief TeacherGroupCampaignResultModel::reload
 */

void TeacherGroupCampaignResultModel::reload()
{
	LOG_CTRACE("client") << "Reload model" << this;

	if (m_taskList.size()) {
		beginRemoveColumns(Utils::noParent(), 0, columnCount());
		m_taskList.clear();
		endRemoveColumns();
	}

	if (m_userList.size()) {
		beginRemoveRows(Utils::noParent(), 0, rowCount());
		m_userList.clear();
		endRemoveRows();
	}


	if (m_teacherGroup && m_teacherGroup->memberList()->size()) {
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

	if (m_campaign && m_campaign->taskList()->size()) {
		beginInsertColumns(Utils::noParent(), 0, m_campaign->taskList()->size()+1);

		m_taskList = m_campaign->getOrderedTaskList();

		endInsertColumns();
	}

	emit modelReloaded();
}
