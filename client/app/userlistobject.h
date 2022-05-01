/*
 * ---- Call of Suli ----
 *
 * userlistobject.h
 *
 * Created on: 2022. 05. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserListObject
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

#ifndef USERLISTOBJECT_H
#define USERLISTOBJECT_H

#include "objectlistmodelobject.h"
#include "objectlistmodel.h"

class UserListObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString firstname READ firstname WRITE setFirstname NOTIFY firstnameChanged)
	Q_PROPERTY(QString lastname READ lastname WRITE setLastname NOTIFY lastnameChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(int classid READ classid WRITE setClassid NOTIFY classidChanged)
	Q_PROPERTY(QString classname READ classname WRITE setClassname NOTIFY classnameChanged)
	Q_PROPERTY(bool isTeacher READ isTeacher WRITE setIsTeacher NOTIFY isTeacherChanged)
	Q_PROPERTY(bool isAdmin READ isAdmin WRITE setIsAdmin NOTIFY isAdminChanged)
	Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
	Q_PROPERTY(QString character READ character WRITE setCharacter NOTIFY characterChanged)
	Q_PROPERTY(QString picture READ picture WRITE setPicture NOTIFY pictureChanged)
	Q_PROPERTY(int rankid READ rankid WRITE setRankid NOTIFY rankidChanged)
	Q_PROPERTY(int ranklevel READ ranklevel WRITE setRanklevel NOTIFY ranklevelChanged)
	Q_PROPERTY(QString rankname READ rankname WRITE setRankname NOTIFY ranknameChanged)
	Q_PROPERTY(QString rankimage READ rankimage WRITE setRankimage NOTIFY rankimageChanged)

public:
	Q_INVOKABLE explicit UserListObject(QObject *parent = nullptr);
	virtual ~UserListObject();

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &firstname() const;
	void setFirstname(const QString &newFirstname);

	const QString &lastname() const;
	void setLastname(const QString &newLastname);

	bool active() const;
	void setActive(bool newActive);

	int classid() const;
	void setClassid(int newClassid);

	const QString &classname() const;
	void setClassname(const QString &newClassname);

	bool isTeacher() const;
	void setIsTeacher(bool newIsTeacher);

	bool isAdmin() const;
	void setIsAdmin(bool newIsAdmin);

	const QString &nickname() const;
	void setNickname(const QString &newNickname);

	const QString &character() const;
	void setCharacter(const QString &newCharacter);

	const QString &picture() const;
	void setPicture(const QString &newPicture);

	int rankid() const;
	void setRankid(int newRankid);

	int ranklevel() const;
	void setRanklevel(int newRanklevel);

	const QString &rankname() const;
	void setRankname(const QString &newRankname);

	const QString &rankimage() const;
	void setRankimage(const QString &newRankimage);

signals:
	void usernameChanged();
	void firstnameChanged();
	void lastnameChanged();
	void activeChanged();
	void classidChanged();
	void classnameChanged();
	void isTeacherChanged();
	void isAdminChanged();
	void nicknameChanged();
	void characterChanged();
	void pictureChanged();
	void rankidChanged();
	void ranklevelChanged();
	void ranknameChanged();
	void rankimageChanged();

private:
	QString m_username;
	QString m_firstname;
	QString m_lastname;
	bool m_active;
	int m_classid;
	QString m_classname;
	bool m_isTeacher;
	bool m_isAdmin;
	QString m_nickname;
	QString m_character;
	QString m_picture;
	int m_rankid;
	int m_ranklevel;
	QString m_rankname;
	QString m_rankimage;

};

Q_DECLARE_METATYPE(ObjectGenericListModel<UserListObject>*);

#endif // USERLISTOBJECT_H
