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
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "rank.h"
#include "credential.h"

class User;
using UserList = qolm::QOlm<User>;
Q_DECLARE_METATYPE(UserList*)


class UserPrivate;
class RpgUserWalletList;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_RpgUserWalletList
#define OPAQUE_PTR_RpgUserWalletList
  Q_DECLARE_OPAQUE_POINTER(RpgUserWalletList*)
#endif

#endif


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
	Q_PROPERTY(QString fullNickName READ fullNickName NOTIFY fullNickNameChanged)
	Q_PROPERTY(Rank rank READ rank WRITE setRank NOTIFY rankChanged)
	Q_PROPERTY(Credential::Roles roles READ roles WRITE setRoles NOTIFY rolesChanged)
	Q_PROPERTY(LoginState loginState READ loginState WRITE setLoginState NOTIFY loginStateChanged)
	Q_PROPERTY(QString picture READ picture WRITE setPicture NOTIFY pictureChanged)
	Q_PROPERTY(QString nickName READ nickName WRITE setNickName NOTIFY nickNameChanged)
	Q_PROPERTY(QString character READ character WRITE setCharacter NOTIFY characterChanged)
	Q_PROPERTY(qreal dailyRate READ dailyRate WRITE setDailyRate NOTIFY dailyRateChanged)
	Q_PROPERTY(int dailyLimit READ dailyLimit WRITE setDailyLimit NOTIFY dailyLimitChanged FINAL)

	Q_PROPERTY(int xp READ xp WRITE setXp NOTIFY xpChanged)
	Q_PROPERTY(int streak READ streak WRITE setStreak NOTIFY streakChanged)
	Q_PROPERTY(bool streakToday READ streakToday WRITE setStreakToday NOTIFY streakTodayChanged)
	Q_PROPERTY(int trophy READ trophy WRITE setTrophy NOTIFY trophyChanged)
	Q_PROPERTY(int classid READ classid WRITE setClassid NOTIFY classidChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(QString oauth READ oauth WRITE setOauth NOTIFY oauthChanged)

	Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
	Q_PROPERTY(RpgUserWalletList* wallet READ wallet STORED false CONSTANT FINAL)

public:
	explicit User(QObject *parent = nullptr);
	virtual ~User();

	enum LoginState {
		LoggedOut,
		LoggingIn,
		LoggedIn
	};

	Q_ENUM(LoginState);

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &familyName() const;
	void setFamilyName(const QString &newFamilyName);

	const QString &givenName() const;
	void setGivenName(const QString &newGivenName);

	QString fullName() const;
	QString fullNickName() const;

	const Rank &rank() const;
	void setRank(const Rank &newRank);

	const Credential::Roles &roles() const;
	void setRoles(const Credential::Roles &newRoles);

	const LoginState &loginState() const;
	void setLoginState(const LoginState &newLoginState);

	int xp() const;
	void setXp(int newXp);

	int classid() const;
	void setClassid(int newClassid);

	QString picture() const;
	void setPicture(const QString &newPicture);

	bool active() const;
	void setActive(bool newActive);

	const QString &oauth() const;
	void setOauth(const QString &newOauth);

	const QString &className() const;
	void setClassName(const QString &newClassName);

	int streak() const;
	void setStreak(int newStreak);

	const QString &nickName() const;
	void setNickName(const QString &newNickName);

	bool streakToday() const;
	void setStreakToday(bool newStreakToday);

	const QString &character() const;
	void setCharacter(const QString &newCharacter);

	int trophy() const;
	void setTrophy(int newTrophy);

	qreal dailyRate() const;
	void setDailyRate(qreal newDailyRate);

	int dailyLimit() const;
	void setDailyLimit(int newDailyLimit);

	RpgUserWalletList* wallet() const;

public slots:
	void clear();

signals:
	void usernameChanged();
	void familyNameChanged();
	void givenNameChanged();
	void fullNameChanged();
	void fullNickNameChanged();
	void rankChanged();
	void rolesChanged();
	void loginStateChanged();
	void xpChanged();
	void classidChanged();
	void pictureChanged();
	void activeChanged();
	void oauthChanged();
	void classNameChanged();
	void streakChanged();
	void nickNameChanged();
	void streakTodayChanged();
	void characterChanged();
	void trophyChanged();
	void dailyRateChanged();
	void dailyLimitChanged();

private:
	QString m_username;
	QString m_familyName;
	QString m_givenName;
	QString m_nickName;
	Rank m_rank;
	Credential::Roles m_roles = Credential::None;
	LoginState m_loginState = LoggedOut;
	int m_xp = 0;
	int m_classid = -1;
	QString m_picture;
	bool m_active = true;
	QString m_oauth;
	QString m_className;
	int m_streak = 0;
	bool m_streakToday = false;
	QString m_character;
	int m_trophy = 0;
	qreal m_dailyRate = 0.0;
	int m_dailyLimit;

	UserPrivate *d = nullptr;
};



#endif // USER_H
