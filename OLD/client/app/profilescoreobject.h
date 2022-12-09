/*
 * ---- Call of Suli ----
 *
 * profilescoreobject.h
 *
 * Created on: 2021. 12. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ProfileScoreObject
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

#ifndef PROFILESCOREOBJECT_H
#define PROFILESCOREOBJECT_H

#include "objectlistmodel.h"
#include "objectlistmodelobject.h"


class ProfileScoreObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString firstname READ firstname WRITE setFirstname NOTIFY firstnameChanged)
	Q_PROPERTY(QString lastname READ lastname WRITE setLastname NOTIFY lastnameChanged)
	Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
	Q_PROPERTY(int rankid READ rankid WRITE setRankid NOTIFY rankidChanged)
	Q_PROPERTY(QString rankname READ rankname WRITE setRankname NOTIFY ranknameChanged)
	Q_PROPERTY(int ranklevel READ ranklevel WRITE setRanklevel NOTIFY ranklevelChanged)
	Q_PROPERTY(QString rankimage READ rankimage WRITE setRankimage NOTIFY rankimageChanged)
	Q_PROPERTY(int xp READ xp WRITE setXp NOTIFY xpChanged)
	Q_PROPERTY(QString picture READ picture WRITE setPicture NOTIFY pictureChanged)
	Q_PROPERTY(bool isTeacher READ isTeacher WRITE setIsTeacher NOTIFY isTeacherChanged)
	Q_PROPERTY(bool isAdmin READ isAdmin WRITE setIsAdmin NOTIFY isAdminChanged)
	Q_PROPERTY(int classid READ classid WRITE setClassid NOTIFY classidChanged)

public:
	Q_INVOKABLE explicit ProfileScoreObject(QObject *parent = nullptr);
	virtual ~ProfileScoreObject();

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &firstname() const;
	void setFirstname(const QString &newFirstname);

	const QString &lastname() const;
	void setLastname(const QString &newLastname);

	const QString &nickname() const;
	void setNickname(const QString &newNickname);

	int rankid() const;
	void setRankid(int newRankid);

	const QString &rankname() const;
	void setRankname(const QString &newRankname);

	int ranklevel() const;
	void setRanklevel(int newRanklevel);

	const QString &rankimage() const;
	void setRankimage(const QString &newRankimage);

	int xp() const;
	void setXp(int newXp);

	const QString &picture() const;
	void setPicture(const QString &newPicture);

	bool isTeacher() const;
	void setIsTeacher(bool newIsTeacher);

	bool isAdmin() const;
	void setIsAdmin(bool newIsAdmin);

	int classid() const;
	void setClassid(int newClassid);

signals:
	void usernameChanged();
	void firstnameChanged();
	void lastnameChanged();
	void nicknameChanged();
	void rankidChanged();
	void ranknameChanged();
	void ranklevelChanged();
	void rankimageChanged();
	void xpChanged();
	void pictureChanged();
	void isTeacherChanged();
	void isAdminChanged();
	void classidChanged();

private:
	QString m_username;
	QString m_firstname;
	QString m_lastname;
	QString m_nickname;
	int m_rankid;
	QString m_rankname;
	int m_ranklevel;
	QString m_rankimage;
	int m_xp;
	QString m_picture;
	bool m_isTeacher;
	bool m_isAdmin;
	int m_classid;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<ProfileScoreObject>*);

#endif // PROFILESCOREOBJECT_H
