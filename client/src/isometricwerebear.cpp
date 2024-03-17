/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricWerebear
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

#include "isometricwerebear.h"
#include "tiledspritehandler.h"
#include "utils_.h"



/**
 * @brief IsometricWerebear::IsometricWerebear
 * @param parent
 */

IsometricWerebear::IsometricWerebear(QQuickItem *parent)
	: IsometricEnemy(EnemyWerebear, parent)
{
	m_metric.speed = 2.;
	m_metric.returnSpeed = 3.;
	m_metric.pursuitSpeed = 3.;
	m_autoAttackSprite = "hit";

	m_footSoundTimer.setInterval(450);
	connect(&m_footSoundTimer, &QTimer::timeout, this, &IsometricWerebear::onFootSoundTimerTimeout);
}


/**
 * @brief IsometricWerebear::~IsometricWerebear
 */

IsometricWerebear::~IsometricWerebear()
{

}






/**
 * @brief IsometricWerebear::load
 */

void IsometricWerebear::load()
{
	setMaxHp(5);
	setHp(5);

	setAvailableDirections(Direction_8);

	const char *file = nullptr;
	const char *data = "data.json";

	switch (m_werebearType) {
		case WerebearBrownArmor:
			file = "werebear_brown_armor.png";
			break;
		case WerebearBrownBare:
			file = "werebear_brown_bare.png";
			break;
		case WerebearBrownShirt:
			file = "werebear_brown_shirt.png";
			break;
		case WerebearWhiteArmor:
			file = "werebear_white_armor.png";
			break;
		case WerebearWhiteBare:
			file = "werebear_white_bare.png";
			break;
		case WerebearWhiteShirt:
			file = "werebear_white_shirt.png";
			break;
		case WerebearDefault:
			file = "werebear_0.png";
			data = "data0.json";
			break;
	}

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/enemies/werebear/").append(data));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error";
		return;
	}

	IsometricObjectSpriteList json;
	json.fromJson(*ptr);

	appendSprite(QStringLiteral(":/enemies/werebear/").append(file), json);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);


	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, [this](){
		if (m_spriteHandler->currentSprite() == "run" && !m_footSoundTimer.isActive()) {
			onFootSoundTimerTimeout();
			m_footSoundTimer.start();
		} else if (m_spriteHandler->currentSprite() != "run" && m_footSoundTimer.isActive())
			m_footSoundTimer.stop();
	});
}


/**
 * @brief IsometricWerebear::onFootSoundTimerTimeout
 */

void IsometricWerebear::onFootSoundTimerTimeout()
{
	m_game->playSfx(QStringLiteral(":/enemies/werebear/stepdirt_%1.wav").arg(m_pNum++), m_scene, m_body->bodyPosition());

	if (m_pNum>8)
		m_pNum = 7;
}



IsometricWerebear::WerebearType IsometricWerebear::werebearType() const
{
	return m_werebearType;
}

void IsometricWerebear::setWerebearType(const WerebearType &newWerebearType)
{
	if (m_werebearType == newWerebearType)
		return;
	m_werebearType = newWerebearType;
	emit werebearTypeChanged();
}


/**
 * @brief IsometricWerebear::setWerebearType
 * @param text
 */

void IsometricWerebear::setWerebearType(const QString &text)
{
	static const QMap<QString, WerebearType> map = {
		{ QStringLiteral("brownArmor"),	WerebearBrownArmor },
		{ QStringLiteral("brownBare"),	WerebearBrownBare },
		{ QStringLiteral("brownShirt"),	WerebearBrownShirt },
		{ QStringLiteral("whiteArmor"),	WerebearWhiteArmor },
		{ QStringLiteral("whiteBare"),	WerebearWhiteBare },
		{ QStringLiteral("whiteShirt"),	WerebearWhiteShirt },
	};

	setWerebearType(map.value(text, WerebearDefault));
}
