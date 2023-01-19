/*
 * ---- Call of Suli ----
 *
 * user.h
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

#ifndef USER_H
#define USER_H

#include <selectableobject.h>
#include <QObject>
#include "QOlm/QOlm.hpp"
#include "rank.h"
#include "credential.h"

class User;

using UserList = qolm::QOlm<User>;

/**
 * @brief The User class
 */

class User : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString familyName READ familyName WRITE setFamilyName NOTIFY familyNameChanged)
	Q_PROPERTY(QString givenName READ givenName WRITE setGivenName NOTIFY givenNameChanged)
	Q_PROPERTY(QString fullName READ fullName NOTIFY fullNameChanged)
	Q_PROPERTY(Rank rank READ rank WRITE setRank NOTIFY rankChanged)
	Q_PROPERTY(Credential::Roles roles READ roles WRITE setRoles NOTIFY rolesChanged)
	Q_PROPERTY(LoginState loginState READ loginState WRITE setLoginState NOTIFY loginStateChanged)

public:
	explicit User(QObject *parent = nullptr);
	virtual ~User() {}

	enum LoginState {
		LoggedOut,
		LoggingIn,
		LoggedIn
	};

	Q_ENUM(LoginState);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &familyName() const;
	void setFamilyName(const QString &newFamilyName);

	const QString &givenName() const;
	void setGivenName(const QString &newGivenName);

	QString fullName() const;

	const Rank &rank() const;
	void setRank(const Rank &newRank);

	const Credential::Roles &roles() const;
	void setRoles(const Credential::Roles &newRoles);

	const LoginState &loginState() const;
	void setLoginState(const LoginState &newLoginState);

public slots:
	void clear();

signals:
	void usernameChanged();
	void familyNameChanged();
	void givenNameChanged();
	void fullNameChanged();
	void rankChanged();
	void rolesChanged();
	void loginStateChanged();

private:
	QString m_username;
	QString m_familyName;
	QString m_givenName;
	Rank m_rank;
	Credential::Roles m_roles = Credential::None;
	LoginState m_loginState = LoggedOut;

};

Q_DECLARE_METATYPE(UserList*)

#endif // USER_H
