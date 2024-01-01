/*
 * ---- Call of Suli ----
 *
 * teachergroup.h
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

#ifndef TEACHERGROUP_H
#define TEACHERGROUP_H

#include "campaign.h"
#include "teachermaphandler.h"
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "user.h"
#include "classobject.h"

class TeacherGroup;
using TeacherGroupList = qolm::QOlm<TeacherGroup>;
Q_DECLARE_METATYPE(TeacherGroupList*)

class TeacherGroupCampaignResultModel;


/**
 * @brief The TeacherGroup class
 */

class TeacherGroup : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int groupid READ groupid WRITE setGroupid NOTIFY groupidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(UserList* userList READ userList CONSTANT)
	Q_PROPERTY(UserList* memberList READ memberList CONSTANT)
	Q_PROPERTY(ClassList* classList READ classList CONSTANT)
	Q_PROPERTY(CampaignList *campaignList READ campaignList CONSTANT)
	Q_PROPERTY(QString fullName READ fullName NOTIFY fullNameChanged)

public:
	explicit TeacherGroup(QObject *parent = nullptr);
	virtual ~TeacherGroup();

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	Q_INVOKABLE void reload();
	Q_INVOKABLE void reloadAndCall(QObject *inst, QJSValue v);

	int groupid() const;
	void setGroupid(int newGroupid);

	const QString &name() const;
	void setName(const QString &newName);

	bool active() const;
	void setActive(bool newActive);

	UserList *userList() const;
	UserList *memberList() const;
	ClassList *classList() const;
	CampaignList *campaignList() const;

	QString fullName() const;

signals:
	void memberListReloaded();
	void campaignListReloaded();
	void groupidChanged();
	void nameChanged();
	void activeChanged();
	void fullNameChanged();

private:
	int m_groupid = -1;
	QString m_name;
	bool m_active = false;
	std::unique_ptr<UserList> m_userList;
	std::unique_ptr<UserList> m_memberList;
	std::unique_ptr<ClassList> m_classList;
	std::unique_ptr<CampaignList> m_campaignList;
};





/**
 * @brief The TeacherGroupCampaignResultModel class
 */

class TeacherGroupCampaignResultModel : public QAbstractTableModel
{
	Q_OBJECT

	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged)
	Q_PROPERTY(Campaign *campaign READ campaign WRITE setCampaign NOTIFY campaignChanged)
	Q_PROPERTY(TeacherMapHandler *mapHandler READ mapHandler WRITE setMapHandler NOTIFY mapHandlerChanged)

	Q_PROPERTY(bool showHeaderPlaceholders READ showHeaderPlaceholders WRITE setShowHeaderPlaceholders NOTIFY showHeaderPlaceholdersChanged)
	Q_PROPERTY(bool showCellPlaceholders READ showCellPlaceholders WRITE setShowCellPlaceholders NOTIFY showCellPlaceholdersChanged)

public:
	explicit TeacherGroupCampaignResultModel(QObject *parent = nullptr);
	virtual ~TeacherGroupCampaignResultModel();

	int rowCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 10 : m_userList.size()+1; }
	int columnCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 5 : m_taskList.size()+1; }
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

	Campaign *campaign() const;
	void setCampaign(Campaign *newCampaign);

	TeacherMapHandler *mapHandler() const;
	void setMapHandler(TeacherMapHandler *newMapHandler);

	Q_INVOKABLE bool isSection(const int &col) const;

	bool showHeaderPlaceholders() const;
	void setShowHeaderPlaceholders(bool newShowHeaderPlaceholders);

	bool showCellPlaceholders() const;
	void setShowCellPlaceholders(bool newShowCellPlaceholders);

	Q_INVOKABLE User* userAt(const int &row) const;
	Q_INVOKABLE bool loadCampaignDataFromUser(Campaign *campaign, User *user) const;
	Q_INVOKABLE bool loadCampaignDataFromRow(Campaign *campaign, const int &row) const;


public slots:
	void reload();
	void reloadContent();

signals:
	void modelReloaded();
	void teacherGroupChanged();
	void campaignChanged();
	void showHeaderPlaceholdersChanged();
	void showCellPlaceholdersChanged();
	void mapHandlerChanged();

private slots:
	void reloadFromJson(const QJsonObject &data);

private:
	/**
	 * @brief The UserResult class
	 */

	struct UserResult {
		QPointer<User> user;
		Grade *grade = nullptr;
		int xp = 0;

		UserResult(User *u) : user(u) {}
	};


	/**
	 * @brief The UserTaskResult class
	 */

	struct UserTaskResult {
		QPointer<User> user;
		QPointer<Task> task;
		bool success = false;
	};

	int findResult(const User *user, const Task *task) const;
	int findResult(const QString &username, const int &taskid) const;
	int findUser(const User *user) const;
	int findUser(const QString &username) const;
	Task* findTask(const int &taskid) const;

	TeacherGroup *m_teacherGroup = nullptr;
	Campaign *m_campaign = nullptr;
	TeacherMapHandler *m_mapHandler = nullptr;
	QVector<UserResult> m_userList;
	QList<TaskOrSection> m_taskList;
	QVector<UserTaskResult> m_resultList;

	bool m_showHeaderPlaceholders = true;
	bool m_showCellPlaceholders = true;
};







/**
 * @brief The Result class
 */

class TeacherGroupResultResult {
	Q_GADGET

	Q_PROPERTY(Grade* grade MEMBER grade);
	Q_PROPERTY(int xp MEMBER xp);

public:
	Grade *grade = nullptr;
	int xp = 0;
};


Q_DECLARE_METATYPE(TeacherGroupResultResult)



/**
 * @brief The TeacherGroupResultModel class
 */

class TeacherGroupResultModel : public QAbstractTableModel
{
	Q_OBJECT

	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged)
	Q_PROPERTY(bool showHeaderPlaceholders READ showHeaderPlaceholders WRITE setShowHeaderPlaceholders NOTIFY showHeaderPlaceholdersChanged)
	Q_PROPERTY(bool showCellPlaceholders READ showCellPlaceholders WRITE setShowCellPlaceholders NOTIFY showCellPlaceholdersChanged)

public:
	explicit TeacherGroupResultModel(QObject *parent = nullptr);
	virtual ~TeacherGroupResultModel();

	int rowCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 10 : m_userList.size()+1; }
	int columnCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 5 : m_campaignList.size()+1; }
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

	bool showHeaderPlaceholders() const;
	void setShowHeaderPlaceholders(bool newShowHeaderPlaceholders);

	bool showCellPlaceholders() const;
	void setShowCellPlaceholders(bool newShowCellPlaceholders);

	Q_INVOKABLE User* userAt(const int &row) const;
	Q_INVOKABLE Campaign* campaignAt(const int &column) const;

public slots:
	void reload();
	void reloadContent();

signals:
	void modelReloaded();
	void teacherGroupChanged();
	void showHeaderPlaceholdersChanged();
	void showCellPlaceholdersChanged();

private slots:
	void reloadFromJson(const QJsonObject &data);

private:
	struct UserResult {
		QPointer<User> user;
		QPointer<Campaign> campaign;
		TeacherGroupResultResult result;
	};

	int findResult(const User *user, const Campaign *campaign) const;
	int findResult(const QString &username, const int &campaignid) const;
	User *findUser(const QString &username) const;
	Campaign *findCampaign(const int &id) const;

	TeacherGroup *m_teacherGroup = nullptr;
	QVector<QPointer<User>> m_userList;
	QVector<QPointer<Campaign>> m_campaignList;
	QVector<UserResult> m_resultList;

	bool m_showHeaderPlaceholders = true;
	bool m_showCellPlaceholders = true;
};





#endif // TEACHERGROUP_H
