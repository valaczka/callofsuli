/*
 * ---- Call of Suli ----
 *
 * user.cpp
 *
 * Created on: 2023. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * User
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

#include "user.h"
#include "application.h"

User::User(QObject *parent)
	: SelectableObject{parent}
{

}



/**
 * @brief User::loadFromJson
 * @param user
 * @param object
 * @param allField
 */

void User::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("username")) || allField)
		setUsername(object.value(QStringLiteral("username")).toString());

	if (object.contains(QStringLiteral("familyName")) || allField)
		setFamilyName(object.value(QStringLiteral("familyName")).toString());

	if (object.contains(QStringLiteral("givenName")) || allField)
		setGivenName(object.value(QStringLiteral("givenName")).toString());

	if (object.contains(QStringLiteral("picture")) || allField)
		setPicture(object.value(QStringLiteral("picture")).toString());

	if ((object.contains(QStringLiteral("rankid")) || allField) && Application::instance()->client()->server())
		setRank(Application::instance()->client()->server()->rank(object.value(QStringLiteral("rankid")).toInt()));

	if (object.contains(QStringLiteral("xp")) || allField)
		setXp(object.value(QStringLiteral("xp")).toInt());

	if (object.contains(QStringLiteral("classid")) || allField)
		setClassid(object.value(QStringLiteral("classid")).toInt());
}



/**
 * @brief User::username
 * @return
 */

const QString &User::username() const
{
	return m_username;
}

void User::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}

const QString &User::familyName() const
{
	return m_familyName;
}

void User::setFamilyName(const QString &newFamilyName)
{
	if (m_familyName == newFamilyName)
		return;
	m_familyName = newFamilyName;
	emit familyNameChanged();
	emit fullNameChanged();
}

const QString &User::givenName() const
{
	return m_givenName;
}

void User::setGivenName(const QString &newGivenName)
{
	if (m_givenName == newGivenName)
		return;
	m_givenName = newGivenName;
	emit givenNameChanged();
	emit fullNameChanged();
}

QString User::fullName() const
{
	return QStringList({m_familyName, m_givenName}).join(' ');
}

const Rank &User::rank() const
{
	return m_rank;
}

void User::setRank(const Rank &newRank)
{
	if (m_rank == newRank)
		return;
	m_rank = newRank;
	emit rankChanged();
}

const Credential::Roles &User::roles() const
{
	return m_roles;
}

void User::setRoles(const Credential::Roles &newRoles)
{
	if (m_roles == newRoles)
		return;
	m_roles = newRoles;
	emit rolesChanged();
}


/**
 * @brief User::clear
 */

void User::clear()
{
	setUsername(QLatin1String(""));
	setFamilyName(QLatin1String(""));
	setGivenName(QLatin1String(""));
	setRank(Rank());
	setRoles(Credential::None);
	setLoginState(LoggedOut);
}

const QString &User::picture() const
{
	return m_picture;
}

void User::setPicture(const QString &newPicture)
{
	if (m_picture == newPicture)
		return;
	m_picture = newPicture;
	emit pictureChanged();
}

int User::classid() const
{
	return m_classid;
}

void User::setClassid(int newClassid)
{
	if (m_classid == newClassid)
		return;
	m_classid = newClassid;
	emit classidChanged();
}

int User::xp() const
{
	return m_xp;
}

void User::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	m_xp = newXp;
	emit xpChanged();
}


const User::LoginState &User::loginState() const
{
	return m_loginState;
}

void User::setLoginState(const LoginState &newLoginState)
{
	if (m_loginState == newLoginState)
		return;
	m_loginState = newLoginState;
	emit loginStateChanged();
}


