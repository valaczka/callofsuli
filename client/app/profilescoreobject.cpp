/*
 * ---- Call of Suli ----
 *
 * profilescoreobject.cpp
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

#include "profilescoreobject.h"

ProfileScoreObject::ProfileScoreObject(QObject *parent)
	: ObjectListModelObject{parent}
	, m_rankid(-1)
	, m_ranklevel(-1)
	, m_xp(0)
	, m_isTeacher(false)
	, m_isAdmin(false)
	, m_classid(-1)
{

}

ProfileScoreObject::~ProfileScoreObject()
{

}

const QString &ProfileScoreObject::username() const
{
	return m_username;
}

void ProfileScoreObject::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}

const QString &ProfileScoreObject::firstname() const
{
	return m_firstname;
}

void ProfileScoreObject::setFirstname(const QString &newFirstname)
{
	if (m_firstname == newFirstname)
		return;
	m_firstname = newFirstname;
	emit firstnameChanged();
}

const QString &ProfileScoreObject::lastname() const
{
	return m_lastname;
}

void ProfileScoreObject::setLastname(const QString &newLastname)
{
	if (m_lastname == newLastname)
		return;
	m_lastname = newLastname;
	emit lastnameChanged();
}

const QString &ProfileScoreObject::nickname() const
{
	return m_nickname;
}

void ProfileScoreObject::setNickname(const QString &newNickname)
{
	if (m_nickname == newNickname)
		return;
	m_nickname = newNickname;
	emit nicknameChanged();
}

int ProfileScoreObject::rankid() const
{
	return m_rankid;
}

void ProfileScoreObject::setRankid(int newRankid)
{
	if (m_rankid == newRankid)
		return;
	m_rankid = newRankid;
	emit rankidChanged();
}

const QString &ProfileScoreObject::rankname() const
{
	return m_rankname;
}

void ProfileScoreObject::setRankname(const QString &newRankname)
{
	if (m_rankname == newRankname)
		return;
	m_rankname = newRankname;
	emit ranknameChanged();
}

int ProfileScoreObject::ranklevel() const
{
	return m_ranklevel;
}

void ProfileScoreObject::setRanklevel(int newRanklevel)
{
	if (m_ranklevel == newRanklevel)
		return;
	m_ranklevel = newRanklevel;
	emit ranklevelChanged();
}

const QString &ProfileScoreObject::rankimage() const
{
	return m_rankimage;
}

void ProfileScoreObject::setRankimage(const QString &newRankimage)
{
	if (m_rankimage == newRankimage)
		return;
	m_rankimage = newRankimage;
	emit rankimageChanged();
}

int ProfileScoreObject::xp() const
{
	return m_xp;
}

void ProfileScoreObject::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	if (m_xp)
		qDebug() << "CHANGE XP" << m_username << m_xp << "->" << newXp << this;
	m_xp = newXp;
	emit xpChanged();
}

const QString &ProfileScoreObject::picture() const
{
	return m_picture;
}

void ProfileScoreObject::setPicture(const QString &newPicture)
{
	if (m_picture == newPicture)
		return;
	m_picture = newPicture;
	emit pictureChanged();
}

bool ProfileScoreObject::isTeacher() const
{
	return m_isTeacher;
}

void ProfileScoreObject::setIsTeacher(bool newIsTeacher)
{
	if (m_isTeacher == newIsTeacher)
		return;
	m_isTeacher = newIsTeacher;
	emit isTeacherChanged();
}

bool ProfileScoreObject::isAdmin() const
{
	return m_isAdmin;
}

void ProfileScoreObject::setIsAdmin(bool newIsAdmin)
{
	if (m_isAdmin == newIsAdmin)
		return;
	m_isAdmin = newIsAdmin;
	emit isAdminChanged();
}

int ProfileScoreObject::classid() const
{
	return m_classid;
}

void ProfileScoreObject::setClassid(int newClassid)
{
	if (m_classid == newClassid)
		return;
	m_classid = newClassid;
	emit classidChanged();
}
