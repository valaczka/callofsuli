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
#include "actiongame.h"
#include "gameplayer.h"



const QVector<GamePickable::GamePickableData> GamePickable::m_pickableDataTypes = {
	{ "health", tr("HP"), GamePickable::PickableHealth, "qrc:/internal/game/powerup.gif", GamePickable::FormatAnimated },
	{ "time1", tr("Idő (30 másodperc)"), GamePickable::PickableTime30, "qrc:/internal/game/time-30.png", GamePickable::FormatPixmap },
	{ "time2", tr("Idő (1 perc)"), GamePickable::PickableTime60, "qrc:/internal/game/time-60.png", GamePickable::FormatPixmap },
	{ "shield1", tr("Pajzs (x1)"), GamePickable::PickableShield1, "qrc:/internal/game/shield-green.png", GamePickable::FormatPixmap },
	{ "shield2", tr("Pajzs (x2)"), GamePickable::PickableShield2, "qrc:/internal/game/shield-blue.png", GamePickable::FormatPixmap },
	{ "shield3", tr("Pajzs (x3)"), GamePickable::PickableShield3, "qrc:/internal/game/shield-red.png", GamePickable::FormatPixmap },
	{ "shield4", tr("Pajzs (x5)"), GamePickable::PickableShield5, "qrc:/internal/game/shield-gold.png", GamePickable::FormatPixmap },
	{ "water", tr("Víz"), GamePickable::PickableWater, "qrc:/internal/game/water.svg", GamePickable::FormatPixmap },
	{ "pliers", tr("Drótvágó"), GamePickable::PickablePliers, "qrc:/internal/game/pliers.png", GamePickable::FormatPixmap },
	{ "camouflage", tr("Álruha"), GamePickable::PickableCamouflage, "qrc:/internal/game/camouflage.png", GamePickable::FormatPixmap },
	{ "teleporter", tr("Teleportáló"), GamePickable::PickableTeleporter, "qrc:/internal/game/teleporter.png", GamePickable::FormatPixmap },
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
		game->setMsecLeft(game->msecLeft()+30*1000);
		game->message(tr("30 seconds gained"), QStringLiteral("#00bcd4"));
		break;

	case PickableTime60:
		game->setMsecLeft(game->msecLeft()+60*1000);
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

	case PickableWater:
	case PickablePliers:
	case PickableCamouflage:
	case PickableTeleporter:
	case PickableInvalid:
		qCWarning(lcGame).noquote() << tr("Can't pick type:") << m_pickableData.type;
		break;

	}

	setState("picked");
}








