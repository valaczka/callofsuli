/*
 * ---- Call of Suli ----
 *
 * rpgchanger.cpp
 *
 * Created on: 2026. 07. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgChanger
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

#include "rpgchanger.h"



// Data hash

const QHash<RpgStream::PlayerConfig::Utility, QVariantMap> RpgChanger::m_dataUtilities = {
	{ RpgStream::PlayerConfig::UtilityMissionary,
	  {
		  { "icon", "qrc:/internal/medal/Icon.1_04.png" },
		  { "description", tr("Missionary") },
	  }
	},

	{ RpgStream::PlayerConfig::UtilitySniper,
	  {
		  { "icon", "qrc:/internal/medal/Icon.1_05.png" },
		  { "description", tr("Sniper") },
	  }
	},
};




const QHash<RpgStream::BaseDefenderObject::Type, QVariantMap> RpgChanger::m_dataDefenders = {
	{ RpgStream::BaseDefenderObject::Pulse,
	  {
		  { "icon", "qrc:/internal/medal/Icon.1_02.png" },
		  { "description", tr("Pulse") },
	  }
	},

	{ RpgStream::BaseDefenderObject::Multiplier1,
	  {
		  { "icon", "qrc:/internal/medal/Icon.1_01.png" },
		  { "description", tr("Multiplier1") },
	  }
	},

	{ RpgStream::BaseDefenderObject::Fog,
	  {
		  { "icon", "qrc:/internal/medal/Icon.1_03.png" },
		  { "description", tr("Fog") },
	  }
	},
};





/**
 * @brief RpgChanger::RpgChanger
 * @param parent
 */

RpgChanger::RpgChanger(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CERROR("game") << "<<<<<<<<<<<<<<<<<< REMOVE";
	setReplaceEnabled(true);
	//////////////////////////
}



/**
 * @brief RpgChanger::useWeapon
 */

void RpgChanger::useWeapon()
{
	RpgMotorPlayerControlled *motor = m_player ? dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor()) : nullptr;

	if (!motor) {
		LOG_CWARNING("game") << "Missing player";
		return;
	}

	motor->changeMpToBullet();

	close();
}


/**
 * @brief RpgChanger::useDefender
 */

void RpgChanger::useDefender()
{
	RpgMotorPlayerControlled *motor = m_player ? dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor()) : nullptr;

	if (!motor) {
		LOG_CWARNING("game") << "Missing player";
		return;
	}

	motor->changeMpToDefender();

	close();
}




/**
 * @brief RpgChanger::useUtility
 */

void RpgChanger::useUtility()
{
	RpgMotorPlayerControlled *motor = m_player ? dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor()) : nullptr;

	if (!motor) {
		LOG_CWARNING("game") << "Missing player";
		return;
	}

	motor->changeMpToUtility();

	close();
}


/**
 * @brief RpgChanger::use
 * @param mode
 */

void RpgChanger::use(const QString &mode)
{
	if (mode == QStringLiteral("weapon"))
		useWeapon();
	else if (mode == QStringLiteral("defender"))
		useDefender();
	else if (mode == QStringLiteral("utility"))
		useUtility();
	else
		LOG_CERROR("game") << "Invalid mode" << mode;
}


/**
 * @brief RpgChanger::currentDefender
 * @return
 */

int RpgChanger::currentDefender()
{
	if (!m_player)
		return RpgStream::BaseDefenderObject::None;

	return m_player->currentDefender();
}


/**
 * @brief RpgChanger::currentUtility
 * @return
 */

int RpgChanger::currentUtility()
{
	if (!m_player)
		return RpgStream::PlayerConfig::UtilityNone;

	return m_player->currentUtility();
}


/**
 * @brief RpgChanger::setDefender
 * @param key
 */

void RpgChanger::setDefender(const int &key)
{
	if (!m_player)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor());

	if (!motor)
		return;

	motor->replaceDefender(RpgStream::BaseDefenderObject::Type(key));
}


/**
 * @brief RpgChanger::setUtility
 * @param key
 */

void RpgChanger::setUtility(const int &key)
{
	if (!m_player)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor());

	if (!motor)
		return;

	motor->replaceUtility(RpgStream::PlayerConfig::Utility(key));
}



/**
 * @brief RpgChanger::set
 * @param mode
 * @param key
 */

void RpgChanger::set(const QString &mode, const int &key)
{
	if (mode == QStringLiteral("defender"))
		setDefender(key);
	else if (mode == QStringLiteral("utility"))
		setUtility(key);
	else
		LOG_CERROR("game") << "Invalid mode" << mode;
}


/**
 * @brief RpgChanger::availableWeapon
 * @return
 */

QVariantMap RpgChanger::availableWeapon()
{
	return QVariantMap{
		{ "icon", "qrc:/internal/medal/Icon.1_08.png" },
		{ "description", tr("Weapon") },
		{ "cost", CFG_MP_CHANGE_BULLET }
	};
}



/**
 * @brief RpgChanger::active
 * @return
 */

bool RpgChanger::active() const
{
	return m_active;
}

void RpgChanger::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}



/**
 * @brief RpgChanger::game
 * @return
 */

RpgGame *RpgChanger::game() const
{
	return m_game;
}

void RpgChanger::setGame(RpgGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();

	if (!m_game)
		return;

	connectPlayer();

	connect(m_game, &RpgGame::controlledPlayerChanged, this, &RpgChanger::connectPlayer);
}


/**
 * @brief RpgChanger::player
 * @return
 */

RpgPlayer *RpgChanger::player() const
{
	return m_player;
}

void RpgChanger::setPlayer(RpgPlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();
}

void RpgChanger::connectPlayer()
{
	if (!m_game)
		return;

	setPlayer(m_game->controlledPlayer());

	reloadDefenders();
	reloadUtilities();

	emit playerReloaded();
}


/**
 * @brief RpgChanger::reloadDefenders
 */

void RpgChanger::reloadDefenders()
{
	if (!m_player)
		return setAvailableDefenders({});

	const auto &list = m_player->config().defender;

	QVariantList d;
	d.reserve(list.size());

	for (const auto &ptr : list) {
		QVariantMap m = m_dataDefenders.value(ptr);

		if (!m.isEmpty()) {
			m.insert(QStringLiteral("key"), ptr);
			m.insert(QStringLiteral("cost"), cfgRequiredMpDefender.value(ptr));
			d.append(m);
		}
	}

	setAvailableDefenders(d);
}


/**
 * @brief RpgChanger::reloadUtilities
 */

void RpgChanger::reloadUtilities()
{
	if (!m_player)
		return setAvailableUtilites({});

	const auto &list = m_player->config().utility;

	QVariantList d;
	d.reserve(list.size());

	for (const auto &ptr : list) {
		QVariantMap m = m_dataUtilities.value(ptr);
		if (!m.isEmpty()) {
			m.insert(QStringLiteral("key"), ptr);
			m.insert(QStringLiteral("cost"), cfgRequiredMpUtility.value(ptr));
			d.append(m);
		}
	}

	setAvailableUtilites(d);
}


/**
 * @brief RpgChanger::availableUtilites
 * @return
 */

QVariantList RpgChanger::availableUtilites() const
{
	return m_availableUtilites;
}

void RpgChanger::setAvailableUtilites(const QVariantList &newAvailableUtilites)
{
	if (m_availableUtilites == newAvailableUtilites)
		return;
	m_availableUtilites = newAvailableUtilites;
	emit availableUtilitesChanged();
}


/**
 * @brief RpgChanger::availableDefenders
 * @return
 */

QVariantList RpgChanger::availableDefenders() const
{
	return m_availableDefenders;
}

void RpgChanger::setAvailableDefenders(const QVariantList &newAvailableDefenders)
{
	if (m_availableDefenders == newAvailableDefenders)
		return;
	m_availableDefenders = newAvailableDefenders;
	emit availableDefendersChanged();
}



bool RpgChanger::replaceEnabled() const
{
	return m_replaceEnabled;
}

void RpgChanger::setReplaceEnabled(bool newReplaceEnabled)
{
	if (m_replaceEnabled == newReplaceEnabled)
		return;
	m_replaceEnabled = newReplaceEnabled;
	emit replaceEnabledChanged();
}
