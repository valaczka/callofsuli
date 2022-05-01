/*
 * ---- Call of Suli ----
 *
 * userlistobject.cpp
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

#include "userlistobject.h"

UserListObject::UserListObject(QObject *parent)
	: ObjectListModelObject{parent}
{

}



/**
 * @brief UserListObject::~UserListObject
 */

UserListObject::~UserListObject()
{

}

const QString &UserListObject::username() const
{
	return m_username;
}

void UserListObject::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}

const QString &UserListObject::firstname() const
{
	return m_firstname;
}

void UserListObject::setFirstname(const QString &newFirstname)
{
	if (m_firstname == newFirstname)
		return;
	m_firstname = newFirstname;
	emit firstnameChanged();
}

const QString &UserListObject::lastname() const
{
	return m_lastname;
}

void UserListObject::setLastname(const QString &newLastname)
{
	if (m_lastname == newLastname)
		return;
	m_lastname = newLastname;
	emit lastnameChanged();
}

bool UserListObject::active() const
{
	return m_active;
}

void UserListObject::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

int UserListObject::classid() const
{
	return m_classid;
}

void UserListObject::setClassid(int newClassid)
{
	if (m_classid == newClassid)
		return;
	m_classid = newClassid;
	emit classidChanged();
}

const QString &UserListObject::classname() const
{
	return m_classname;
}

void UserListObject::setClassname(const QString &newClassname)
{
	if (m_classname == newClassname)
		return;
	m_classname = newClassname;
	emit classnameChanged();
}

bool UserListObject::isTeacher() const
{
	return m_isTeacher;
}

void UserListObject::setIsTeacher(bool newIsTeacher)
{
	if (m_isTeacher == newIsTeacher)
		return;
	m_isTeacher = newIsTeacher;
	emit isTeacherChanged();
}

bool UserListObject::isAdmin() const
{
	return m_isAdmin;
}

void UserListObject::setIsAdmin(bool newIsAdmin)
{
	if (m_isAdmin == newIsAdmin)
		return;
	m_isAdmin = newIsAdmin;
	emit isAdminChanged();
}

const QString &UserListObject::nickname() const
{
	return m_nickname;
}

void UserListObject::setNickname(const QString &newNickname)
{
	if (m_nickname == newNickname)
		return;
	m_nickname = newNickname;
	emit nicknameChanged();
}

const QString &UserListObject::character() const
{
	return m_character;
}

void UserListObject::setCharacter(const QString &newCharacter)
{
	if (m_character == newCharacter)
		return;
	m_character = newCharacter;
	emit characterChanged();
}

const QString &UserListObject::picture() const
{
	return m_picture;
}

void UserListObject::setPicture(const QString &newPicture)
{
	if (m_picture == newPicture)
		return;
	m_picture = newPicture;
	emit pictureChanged();
}

int UserListObject::rankid() const
{
	return m_rankid;
}

void UserListObject::setRankid(int newRankid)
{
	if (m_rankid == newRankid)
		return;
	m_rankid = newRankid;
	emit rankidChanged();
}

int UserListObject::ranklevel() const
{
	return m_ranklevel;
}

void UserListObject::setRanklevel(int newRanklevel)
{
	if (m_ranklevel == newRanklevel)
		return;
	m_ranklevel = newRanklevel;
	emit ranklevelChanged();
}

const QString &UserListObject::rankname() const
{
	return m_rankname;
}

void UserListObject::setRankname(const QString &newRankname)
{
	if (m_rankname == newRankname)
		return;
	m_rankname = newRankname;
	emit ranknameChanged();
}

const QString &UserListObject::rankimage() const
{
	return m_rankimage;
}

void UserListObject::setRankimage(const QString &newRankimage)
{
	if (m_rankimage == newRankimage)
		return;
	m_rankimage = newRankimage;
	emit rankimageChanged();
}
