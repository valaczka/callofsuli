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
#include "server.h"

User::User(QObject *parent)
	: SelectableObject{parent}
{
	m_mapConvertFuncs.insert(QStringLiteral("rank"), [](const QVariant &v) -> QVariantMap {
		return qvariant_cast<Rank>(v).toJson().toVariantMap();
	});

	m_jsonConvertFuncs.insert(QStringLiteral("rank"), [](const QVariant &v) -> QJsonObject {
		return qvariant_cast<Rank>(v).toJson();
	});
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

	if (object.contains(QStringLiteral("nickname")) || allField)
		setNickName(object.value(QStringLiteral("nickname")).toString());

	if (object.contains(QStringLiteral("picture")) || allField)
		setPicture(object.value(QStringLiteral("picture")).toString());

	if (object.contains(QStringLiteral("character")) || allField)
		setCharacter(object.value(QStringLiteral("character")).toString());

	if ((object.contains(QStringLiteral("rankid")) || allField) && Application::instance()->client()->server())
		setRank(Application::instance()->client()->server()->rank(object.value(QStringLiteral("rankid")).toInt()));

	if (object.contains(QStringLiteral("xp")) || allField)
		setXp(object.value(QStringLiteral("xp")).toInt());

	if (object.contains(QStringLiteral("streak")) || allField)
		setStreak(object.value(QStringLiteral("streak")).toInt());

	if (object.contains(QStringLiteral("streakToday")) || allField)
		setStreakToday(object.value(QStringLiteral("streakToday")).toVariant().toBool());

	if (object.contains(QStringLiteral("trophy")) || allField)
		setTrophy(object.value(QStringLiteral("trophy")).toInt());

	if (object.contains(QStringLiteral("classid")) || allField)
		setClassid(object.value(QStringLiteral("classid")).toInt(-1));

	if (object.contains(QStringLiteral("className")) || allField)
		setClassName(object.value(QStringLiteral("className")).toString());

	if (object.contains(QStringLiteral("active")) || allField)
		setActive(object.value(QStringLiteral("active")).toInt());

	if (object.contains(QStringLiteral("oauth")) || allField)
		setOauth(object.value(QStringLiteral("oauth")).toString());

	if (object.contains(QStringLiteral("dailyRate")) || allField)
		setDailyRate(object.value(QStringLiteral("dailyRate")).toDouble());

	if (object.contains(QStringLiteral("dailyLimit")) || allField)
		setDailyLimit(object.value(QStringLiteral("dailyLimit")).toInt());

	Credential::Roles roles = m_roles;

	if (object.contains(QStringLiteral("isAdmin")) || allField)
		roles.setFlag(Credential::Admin, object.value(QStringLiteral("isAdmin")).toInt());

	if (object.contains(QStringLiteral("isTeacher")) || allField)
		roles.setFlag(Credential::Teacher, object.value(QStringLiteral("isTeacher")).toInt());

	if (object.contains(QStringLiteral("isPanel")) || allField)
		roles.setFlag(Credential::Panel, object.value(QStringLiteral("isPanel")).toInt());

	setRoles(roles);

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
	emit fullNickNameChanged();
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
	emit fullNickNameChanged();
}

QString User::fullName() const
{
	return QStringList({m_familyName, m_givenName}).join(' ');
}


/**
 * @brief User::fullNickName
 * @return
 */

QString User::fullNickName() const
{
	if (!m_nickName.simplified().isEmpty())
		return m_nickName;
	else
		return fullName();
}

/**
 * @brief User::rank
 * @return
 */

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
	setUsername(QStringLiteral(""));
	setFamilyName(QStringLiteral(""));
	setGivenName(QStringLiteral(""));
	setNickName(QStringLiteral(""));
	setCharacter(QStringLiteral(""));
	setRank(Rank());
	setRoles(Credential::None);
	setLoginState(LoggedOut);
}

qreal User::dailyRate() const
{
	return m_dailyRate;
}

void User::setDailyRate(qreal newDailyRate)
{
	if (qFuzzyCompare(m_dailyRate, newDailyRate))
		return;
	m_dailyRate = newDailyRate;
	emit dailyRateChanged();
}

const QString &User::character() const
{
	return m_character;
}

void User::setCharacter(const QString &newCharacter)
{
	if (m_character == newCharacter)
		return;
	m_character = newCharacter;
	emit characterChanged();
}

bool User::streakToday() const
{
	return m_streakToday;
}

void User::setStreakToday(bool newStreakToday)
{
	if (m_streakToday == newStreakToday)
		return;
	m_streakToday = newStreakToday;
	emit streakTodayChanged();
}




/**
 * @brief User::nickName
 * @return
 */

const QString &User::nickName() const
{
	return m_nickName;
}

void User::setNickName(const QString &newNickName)
{
	if (m_nickName == newNickName)
		return;
	m_nickName = newNickName;
	emit nickNameChanged();
	emit fullNickNameChanged();
}

int User::streak() const
{
	return m_streak;
}

void User::setStreak(int newStreak)
{
	if (m_streak == newStreak)
		return;
	m_streak = newStreak;
	emit streakChanged();
}

const QString &User::className() const
{
	return m_className;
}

void User::setClassName(const QString &newClassName)
{
	if (m_className == newClassName)
		return;
	m_className = newClassName;
	emit classNameChanged();
}

const QString &User::oauth() const
{
	return m_oauth;
}

void User::setOauth(const QString &newOauth)
{
	if (m_oauth == newOauth)
		return;
	m_oauth = newOauth;
	emit oauthChanged();
}

bool User::active() const
{
	return m_active;
}

void User::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

QString User::picture() const
{
#ifdef Q_OS_WASM
	if (!m_picture.isEmpty()) {
		LOG_CTRACE("client") << "URL override:" << m_picture;
	}
	return QStringLiteral("");
#endif
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



int User::trophy() const
{
	return m_trophy;
}

void User::setTrophy(int newTrophy)
{
	if (m_trophy == newTrophy)
		return;
	m_trophy = newTrophy;
	emit trophyChanged();
}

int User::dailyLimit() const
{
	return m_dailyLimit;
}

void User::setDailyLimit(int newDailyLimit)
{
	if (m_dailyLimit == newDailyLimit)
		return;
	m_dailyLimit = newDailyLimit;
	emit dailyLimitChanged();
}
