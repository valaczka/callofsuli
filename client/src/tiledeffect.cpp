/*
 * ---- Call of Suli ----
 *
 * tiledeffect.cpp
 *
 * Created on: 2024. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledEffect
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

#include "tiledeffect.h"
#include "qrandom.h"
#include "tiledgame.h"
#include "tiledspritehandler.h"

TiledEffect::TiledEffect(TiledObject *parentObject)
	: m_parentObject(parentObject)
{
	Q_ASSERT(m_parentObject);
}


/**
 * @brief TiledEffect::playSprite
 * @param path
 * @param sprite
 */

void TiledEffect::playSprite(const QString &path, const TiledObjectSprite &sprite)
{
	if (!QFile::exists(path))
		return;
	m_parentObject->playAuxSprite(m_auxHandler, m_alignToBody, path, sprite);
}


/**
 * @brief TiledEffect::playSprite
 * @param path
 * @param sprite
 * @param replaceCurrentSprite
 */

void TiledEffect::playSprite(const QString &path, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite)
{
	if (!QFile::exists(path))
		return;
	m_parentObject->playAuxSprite(m_auxHandler, m_alignToBody, path, sprite, replaceCurrentSprite);
}


/**
 * @brief TiledEffect::playSprite
 * @param path
 * @param sprite
 * @param soundPath
 * @param baseVolume
 */


void TiledEffect::playSprite(const QString &path, const TiledObjectSprite &sprite,
							 const QString &soundPath, const float &baseVolume)
{
	if (!QFile::exists(path))
		return;
	m_parentObject->playAuxSprite(m_auxHandler, m_alignToBody, path, sprite);
	m_parentObject->m_game->playSfx(soundPath,
									m_parentObject->m_scene,
									m_parentObject->m_body->bodyPosition(),
									baseVolume
									);
}



/**
 * @brief TiledEffect::playSprite
 * @param path
 * @param sprite
 * @param replaceCurrentSprite
 * @param soundPath
 * @param baseVolume
 */

void TiledEffect::playSprite(const QString &path, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite, const QString &soundPath, const float &baseVolume)
{
	if (!QFile::exists(path))
		return;
	m_parentObject->playAuxSprite(m_auxHandler, m_alignToBody, path, sprite, replaceCurrentSprite);
	m_parentObject->m_game->playSfx(soundPath,
									m_parentObject->m_scene,
									m_parentObject->m_body->bodyPosition(),
									baseVolume
									);
}



/**
 * @brief TiledEffect::clear
 */

void TiledEffect::clear()
{
	switch (m_auxHandler) {
		case TiledObject::AuxFront:
			m_parentObject->spriteHandlerAuxFront()->clear();
			m_parentObject->spriteHandlerAuxFront()->update();
			break;

		case TiledObject::AuxBack:
			m_parentObject->spriteHandlerAuxBack()->clear();
			m_parentObject->spriteHandlerAuxBack()->update();
			break;
	}
}



//// ----------------------------------------------- ////


/**
 * @brief TiledEffectHealed::play
 */

void TiledEffectHealed::play()
{
	static const TiledObjectSprite sprite = {
		QStringLiteral("base"),
		6,
		0, 0, 64, 64,
		60,
		1
	};

	playSprite(QStringLiteral(":/rpg/common/heal.png"), sprite);
}



/**
 * @brief TiledEffectSpark::play
 */

void TiledEffectSpark::play()
{
	static const TiledObjectSprite spriteRow1 = {
		QStringLiteral("base"),
		4,
		0, 0, 64, 64,
		90,
		1
	};

	static const TiledObjectSprite spriteRow2 = {
		QStringLiteral("base"),
		4,
		0, 64, 64, 64,
		90,
		1
	};

	struct Sprite {
		QString path;
		const TiledObjectSprite *ptr = nullptr;
	};

	static const std::map<Type, Sprite> map = {
		{ SparkOrange1, { QStringLiteral(":/rpg/common/spark_orange.png"), &spriteRow1 } },
		{ SparkOrange2, { QStringLiteral(":/rpg/common/spark_orange.png"), &spriteRow2 } },
		{ SparkRed1, { QStringLiteral(":/rpg/common/spark_red.png"), &spriteRow1 } },
		{ SparkRed2, { QStringLiteral(":/rpg/common/spark_red.png"), &spriteRow2 } },
		{ SparkBlue1, { QStringLiteral(":/rpg/common/spark_blue.png"), &spriteRow1 } },
		{ SparkBlue2, { QStringLiteral(":/rpg/common/spark_blue.png"), &spriteRow2 } },
	};

	std::vector<Sprite> list;

	for (const auto &[type, sprite] : std::as_const(map)) {
		if (m_types.testFlag(type))
			list.push_back(sprite);
	}

	if (list.empty())
		return;

	const Sprite &sprite = list.at(QRandomGenerator::global()->bounded((int) list.size()));

	playSprite(sprite.path, *sprite.ptr, QStringLiteral(":/sound/sfx/pick.mp3"));
}




/**
 * @brief TiledEffectShield::play
 */

void TiledEffectShield::play()
{
	static const TiledObjectSprite sprite = {
		QStringLiteral("base"),
		4,
		0, 0, 128, 128,
		60,
		0
	};

	playSprite(QStringLiteral(":/rpg/common/shield.png"), sprite, true);
}








/**
 * @brief TiledEffectFire::play
 */

void TiledEffectFire::play()
{
	static const TiledObjectSprite sprite = {
		QStringLiteral("base"),
		25,
		256, 0, 128, 126,
		30,
		1,
		true
	};

	playSprite(QStringLiteral(":/rpg/common/explosion_fire_smoke.png"), sprite, true);
}
