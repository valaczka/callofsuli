/*
 * ---- Call of Suli ----
 *
 * gamepickable.cpp
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePickable
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

#include "gamepickable.h"
#include "Logger.h"
#include "actiongame.h"
#include "gameplayer.h"


/// Pickable types

const QVector<GamePickable::GamePickableData> GamePickable::m_pickableDataTypes = {
	{ QStringLiteral("health"), tr("HP"), GamePickable::PickableHealth,
	  QStringLiteral("qrc:/internal/game/powerup.gif"), GamePickable::FormatAnimated,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("time1"), tr("Idő (30 másodperc)"), GamePickable::PickableTime30,
	  QStringLiteral("qrc:/internal/game/time-30.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("time2"), tr("Idő (1 perc)"), GamePickable::PickableTime60,
	  QStringLiteral("qrc:/internal/game/time-60.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("shield1"), tr("Pajzs (x1)"), GamePickable::PickableShield1,
	  QStringLiteral("qrc:/internal/game/shield-green.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("shield2"), tr("Pajzs (x2)"), GamePickable::PickableShield2,
	  QStringLiteral("qrc:/internal/game/shield-blue.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("shield3"), tr("Pajzs (x3)"), GamePickable::PickableShield3,
	  QStringLiteral("qrc:/internal/game/shield-red.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("shield4"), tr("Pajzs (x5)"), GamePickable::PickableShield5,
	  QStringLiteral("qrc:/internal/game/shield-gold.png"), GamePickable::FormatPixmap,
	  QLatin1String(""), QLatin1String(""), QLatin1String("") },

	{ QStringLiteral("water"), tr("Víz"), GamePickable::PickableWater,
	  QStringLiteral("qrc:/internal/game/water.svg"), GamePickable::FormatPixmap,
	  QLatin1String("qrc:/Qaterial/Icons/water.svg"), QLatin1String("#1565c0"), tr("water") },

	{ QStringLiteral("pliers"), tr("Drótvágó"), GamePickable::PickablePliers,
	  QStringLiteral("qrc:/internal/game/pliers.png"), GamePickable::FormatPixmap,
	  QLatin1String("qrc:/Qaterial/Icons/wrench.svg"), QLatin1String("#4e342e"), tr("wrench") },

	{ QStringLiteral("camouflage"), tr("Álruha"), GamePickable::PickableCamouflage,
	  QStringLiteral("qrc:/internal/game/camouflage.png"), GamePickable::FormatPixmap,
	  QLatin1String("qrc:/Qaterial/Icons/domino-mask.svg"), QLatin1String("#fbc02d"), tr("camouflage") },

	{ QStringLiteral("teleporter"), tr("Teleportáló"), GamePickable::PickableTeleporter,
	  QStringLiteral("qrc:/internal/game/teleporter.png"), GamePickable::FormatPixmap,
	  QLatin1String("qrc:/Qaterial/Icons/remote.svg"), QLatin1String("#00acc1"), tr("teleporter") },
};



/**
 * @brief GamePickable::GamePickable
 * @param parent
 */

GamePickable::GamePickable(QQuickItem *parent)
	: GameObject(parent)
{
	connect(this, &GamePickable::pickFinished, this, &GamePickable::deleteLater);
}


/**
 * @brief GamePickable::~GamePickable
 */

GamePickable::~GamePickable()
{

}

const QVector<GamePickable::GamePickableData> &GamePickable::pickableDataTypes()
{
	return m_pickableDataTypes;
}



/**
 * @brief GamePickable::pickableDataHash
 * @return
 */

QHash<QString, GamePickable::GamePickableData> GamePickable::pickableDataHash()
{
	QHash<QString, GamePickable::GamePickableData> hash;

	foreach (const GamePickableData &d, m_pickableDataTypes) {
		hash.insert(d.id, d);
	}

	return hash;
}


/**
 * @brief GamePickable::pickableDataDetails
 * @param type
 * @return
 */

GamePickable::GamePickableData GamePickable::pickableDataDetails(const PickableType &type)
{
	foreach (const GamePickableData &d, m_pickableDataTypes) {
		if (d.type == type)
			return d;
	}

	return GamePickableData();
}




/**
 * @brief GamePickable::data
 * @return
 */

const GamePickable::GamePickableData &GamePickable::pickableData() const
{
	return m_pickableData;
}

void GamePickable::setPickableData(const GamePickableData &newData)
{
	m_pickableData = newData;
	emit idChanged();
	emit nameChanged();
	emit typeChanged();
	emit imageChanged();
	emit formatChanged();
	emit pickableDataChanged();
}


/**
 * @brief GamePickable::bottomPoint
 * @return
 */

QPointF GamePickable::bottomPoint() const
{
	return m_bottomPoint;
}

void GamePickable::setBottomPoint(QPointF newBottomPoint)
{
	if (m_bottomPoint == newBottomPoint)
		return;
	m_bottomPoint = newBottomPoint;
	emit bottomPointChanged();
}


/**
 * @brief GamePickable::pick
 * @param game
 * @param player
 */

void GamePickable::pick(ActionGame *game)
{
	Q_ASSERT(game);

	GamePlayer *player = game->player();

	switch (m_pickableData.type) {
	case PickableHealth:
		player->setHp(player->hp()+1);
		game->message(tr("1 HP gained"), QStringLiteral("#e53935"));
		break;

	case PickableTime30:
		game->addMSec(30000);
		game->message(tr("30 seconds gained"), QStringLiteral("#00bcd4"));
		break;

	case PickableTime60:
		game->addMSec(60000);
		game->message(tr("60 seconds gained"), QStringLiteral("#00bcd4"));
		break;

	case PickableShield1:
		player->setShield(player->shield()+1);
		game->message(tr("1 shield gained"), QStringLiteral("#4CAF50"));
		break;

	case PickableShield2:
		player->setShield(player->shield()+2);
		game->message(tr("2 shields gained"), QStringLiteral("#4CAF50"));
		break;

	case PickableShield3:
		player->setShield(player->shield()+3);
		game->message(tr("3 shields gained"), QStringLiteral("#4CAF50"));
		break;

	case PickableShield5:
		player->setShield(player->shield()+5);
		game->message(tr("5 shields gained"), QStringLiteral("#4CAF50"));
		break;

	default:
		// Tool
		if (!pickableDataDetails(m_pickableData.type).icon.isEmpty()) {
			auto d = pickableDataDetails(m_pickableData.type);
			game->toolAdd(m_pickableData.type);
			game->message(tr("1 %1 gained").arg(d.messageName.isEmpty() ? d.name : d.messageName), d.iconColor);
		} else {
			LOG_CWARNING("game") << "Can't pick type:" << m_pickableData.type;
		}
		break;
	}

	setState(QStringLiteral("picked"));
}





/**
 * @brief GamePickable::operate
 * @param game
 * @param player
 */

void GamePickable::operate(ActionGame *game, const GamePickable::PickableType &type)
{
	Q_ASSERT(game);

	GamePlayer *player = game->player();

	if (!player) {
		LOG_CWARNING("game") << "Missing player";
		return;
	}

	if (player->playerState() == GamePlayer::Operate || player->playerState() == GamePlayer::MoveToOperate) {
		LOG_CWARNING("game") << "Operation already in progress";
		return;
	}



	// Looking for terrain objects

	QStringList terrainObjectTypes;

	switch (type) {
	case GamePickable::PickableWater:
		terrainObjectTypes = QStringList({QStringLiteral("fire")});
		break;
	case GamePickable::PickablePliers:
		terrainObjectTypes = QStringList({QStringLiteral("fence")});
		break;
	default:
		break;
	}

	GameObject *terrainObject = nullptr;

	if (!terrainObjectTypes.isEmpty()) {
		foreach (const QString &s, terrainObjectTypes) {
			GameObject *object = player->terrainObject(s);

			if (object && game->canOperate(object)) {
				terrainObject = object;
				break;
			}
		}
	}



	// Operate

	switch (type) {

	case GamePickable::PickableWater:
		if (game->toolCount(type) && terrainObject) {
			game->toolRemove(type);
			player->operate(terrainObject);
			return;
		}
		break;

	case GamePickable::PickablePliers:
		if (game->toolCount(type) && terrainObject) {
			player->operate(terrainObject);
			return;
		}
		break;

	case GamePickable::PickableCamouflage:
		if (game->toolCount(type)) {
			game->toolRemove(type);
			player->startInvisibility(15000);
			return;
		}

	default:
		break;
	}


	LOG_CWARNING("game") << "Invalid operation for tool:" << type;
}








