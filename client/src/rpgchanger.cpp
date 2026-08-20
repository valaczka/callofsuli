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
		  { "icon", "qrc:/rpg/castIcon/missionary.png" },
		  { "description", tr("Missionary") },
		  { "helper", tr("A semleges NPC-ket átállítja a saját csapathoz") },
	  }
	},

	{ RpgStream::PlayerConfig::UtilitySniper,
	  {
		  { "icon", "qrc:/rpg/castIcon/sniper.png" },
		  { "description", tr("Sniper") },
		  { "helper", tr("Távolról egyetlen lövéssel megsemmisíti az akadályokat vagy az ellenfelet") },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityInvisible,
	  {
		  { "icon", "qrc:/rpg/castIcon/invisible.png" },
		  { "description", tr("Invisible") },
		  { "helper", tr("%1 másodpercig láthatatlanná válik az ellenfelek számára")
			.arg(AbstractGame::TickTimer::tickToMsec(cfgUtilityInvisible.duration)/1000) },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityBlockMpPick,
	  {
		  { "icon", "qrc:/rpg/castIcon/blockPick.png" },
		  { "description", tr("MP pick blocker") },
		  { "helper", tr("Az ellenfél %1 másodpercig nem tud MP-t gyűjteni")
			.arg(AbstractGame::TickTimer::tickToMsec(cfgUtilityBlockMpPick.duration)/1000) },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityBlockMpConvert,
	  {
		  { "icon", "qrc:/rpg/castIcon/blockConvert.png" },
		  { "description", tr("MP convert blocker") },
		  { "helper", tr("Az ellenfél %1 másodpercig nem tud MP-t átváltani lőszerre, eszközre vagy képességre")
			.arg(AbstractGame::TickTimer::tickToMsec(cfgUtilityBlockMpConvert.duration)/1000) },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityBlockAttack,
	  {
		  { "icon", "qrc:/rpg/castIcon/blockAttack.png" },
		  { "description", tr("Attack blocker") },
		  { "helper", tr("Az ellenfél %1 másodpercig nem tud támadni")
			.arg(AbstractGame::TickTimer::tickToMsec(cfgUtilityBlockAttack.duration)/1000) },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityBoostAttackTower,
	  {
		  { "icon", "qrc:/rpg/castIcon/boostTower.png" },
		  { "description", tr("Boost power") },
		  { "helper", tr("Kérdés nélkül maximális termelésre állít egy Power Generatort") },
	  }
	},

	{ RpgStream::PlayerConfig::UtilityBoostPoint,
	  {
		  { "icon", "qrc:/rpg/castIcon/boostPower.png" },
		  { "description", tr("Turbo boost") },
		  { "helper", tr("%1 másodpercig +100%-kal megnöveli a Power Point termelést")
			.arg(AbstractGame::TickTimer::tickToMsec(cfgUtilityBoostPoint.duration)/1000) },
	  }
	},
};




const QHash<RpgStream::BaseDefenderObject::Type, QVariantMap> RpgChanger::m_dataDefenders = {
	{ RpgStream::BaseDefenderObject::Pulse,
	  {
		  { "icon", "qrc:/rpg/castIcon/pulse.png" },
		  { "description", tr("Pulse") },
		  { "helper", tr("Ha megközelíti az ellenfél, hátralöki és megsebzi") },
	  }
	},

	{ RpgStream::BaseDefenderObject::Multiplier1,
	  {
		  { "icon", "qrc:/Qaterial/Icons/fan-speed-2.svg" },
		  { "description", tr("Multiplicator") },
		  { "helper", tr("A Power Generator mellé helyezhető, a Power Point termelést +100%-kal megnöveli") },
	  }
	},

	{ RpgStream::BaseDefenderObject::Fog,
	  {
		  { "icon", "qrc:/rpg/castIcon/fog.png" },
		  { "description", tr("Fog") },
		  { "helper", tr("A környezetében elnyeli az ellenfél lövedékeit") },
	  }
	},

	{ RpgStream::BaseDefenderObject::Electric,
	  {
		  { "icon", "qrc:/rpg/castIcon/electricity.png" },
		  { "description", tr("Electricity") },
		  { "helper", tr("Minden ellene irányuló támadás esetén %1 HP-val megsebzi a támadóját")
		  .arg(cfgDefenderElectric.force) },
	  }
	},

	{ RpgStream::BaseDefenderObject::Questionnaire,
	  {
		  { "icon", "qrc:/rpg/castIcon/questionnaire.png" },
		  { "description", tr("Questionnaire") },
		  { "helper", tr("A közelében folyamatosan tesztkérdéseket ad az ellenfélnek") },
	  }
	},

	{ RpgStream::BaseDefenderObject::HpHealer,
	  {
		  { "icon", "qrc:/rpg/castIcon/hpHealer.png" },
		  { "description", tr("HP Healer") },
		  { "helper", tr("A közelében a saját csapat tagjainak folyamatosan visszaadja az elvesztett HP-kat") },
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
}



/**
 * @brief RpgChanger::useWeapon
 */

void RpgChanger::useWeapon(const bool &force)
{
	RpgMotorPlayerControlled *motor = m_player ? dynamic_cast<RpgMotorPlayerControlled*>(m_player->currentMotor()) : nullptr;

	if (!motor) {
		LOG_CWARNING("game") << "Missing player";
		return;
	}

	motor->changeMpToBullet(force);

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
		{ "icon", "qrc:/rpg/bullet/pickable.png" },
		{ "description", tr("Reload bullets") },
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
	checkBlocked();

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
	checkBlocked();

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
 * @brief RpgChanger::checkBlocked
 */

void RpgChanger::checkBlocked()
{
	if (!m_player)
		return;

	setIsBlocked(m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockMpConvert, m_player->team()));
}


/**
 * @brief RpgChanger::dataUtilities
 * @return
 */

const QHash<RpgStream::PlayerConfig::Utility, QVariantMap> &RpgChanger::dataUtilities()
{
	return m_dataUtilities;
}

const QHash<RpgStream::BaseDefenderObject::Type, QVariantMap> &RpgChanger::dataDefenders()
{
	return m_dataDefenders;
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

bool RpgChanger::isBlocked() const
{
	return m_isBlocked;
}

void RpgChanger::setIsBlocked(bool newIsBlocked)
{
	if (m_isBlocked == newIsBlocked)
		return;
	m_isBlocked = newIsBlocked;
	emit isBlockedChanged();
}
