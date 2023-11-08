/*
 * ---- Call of Suli ----
 *
 * studentgroup.h
 *
 * Created on: 2023. 01. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentGroup
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

#ifndef STUDENTGROUP_H
#define STUDENTGROUP_H

#include "campaign.h"
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "user.h"

class StudentGroup;
using StudentGroupList = qolm::QOlm<StudentGroup>;
Q_DECLARE_METATYPE(StudentGroupList*)

/**
 * @brief The StudentGroup class
 */

class StudentGroup : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int groupid READ groupid WRITE setGroupid NOTIFY groupidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(UserList* memberList READ memberList CONSTANT)
	Q_PROPERTY(CampaignList *campaignList READ campaignList CONSTANT)

public:
	explicit StudentGroup(QObject *parent = nullptr);
	virtual ~StudentGroup();

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int groupid() const;
	void setGroupid(int newGroupid);

	const QString &name() const;
	void setName(const QString &newName);

	bool active() const;
	void setActive(bool newActive);

	UserList *memberList() const;

	CampaignList *campaignList() const;


signals:
	void groupidChanged();
	void nameChanged();
	void activeChanged();

private:
	int m_groupid = -1;
	QString m_name;
	bool m_active = false;
	std::unique_ptr<UserList> m_memberList;
	std::unique_ptr<CampaignList> m_campaignList;
};

#endif // STUDENTGROUP_H
