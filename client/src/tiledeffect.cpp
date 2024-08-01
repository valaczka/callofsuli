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
#include "utils_.h"

const QString TiledEffectHealed::m_staticSpriteName = QStringLiteral("healed");
const QString TiledEffectSleep::m_staticSpriteName = QStringLiteral("sleep");
const QString TiledEffectShield::m_staticSpriteName = QStringLiteral("shield");
const QString TiledEffectSpark::m_staticSpriteName = QStringLiteral("spark");
const QString TiledEffectFire::m_staticSpriteName = QStringLiteral("fire");
const QString TiledEffectRing::m_staticSpriteName = QStringLiteral("ring");



/**
 * @brief fromSprite
 * @param sprite
 * @return
 */

namespace TiledEffectNS {
static QVector<TiledGame::TextureSpriteMapper> fromSprite(const TiledObjectSprite &sprite, const QString &name) {
	QVector<TiledGame::TextureSpriteMapper> mapper;

	TiledGame::TextureSpriteMapper m;
	m.name = name;
	m.width = sprite.width;
	m.height = sprite.height;
	m.duration = sprite.duration;
	m.loops = sprite.loops;

	for (int i=0; i<sprite.count; ++i) {
		mapper.append(m);
	}

	return mapper;
}
}


/**
 * @brief TiledEffect::TiledEffect
 * @param parentObject
 * @param spriteName
 */

TiledEffect::TiledEffect(TiledObject *parentObject, const QString &spriteName)
	: m_parentObject(parentObject)
	, m_spriteName(spriteName)
{
	Q_ASSERT(m_parentObject);
}


/**
 * @brief TiledEffect::stop
 */

void TiledEffect::stop()
{
	TiledSpriteHandler *handler = m_auxHandler == TiledObject::AuxFront ? m_parentObject->spriteHandlerAuxFront() :
																		  m_parentObject->spriteHandlerAuxBack();

	Q_ASSERT(handler);

	if (!m_spriteName.isEmpty() && handler->currentSprite() != m_spriteName)
		return;

	handler->clear();
}


/**
 * @brief TiledEffect::active
 * @return
 */

bool TiledEffect::active() const
{
	if (m_spriteName.isEmpty())
		return false;

	TiledSpriteHandler *handler = m_auxHandler == TiledObject::AuxFront ? m_parentObject->spriteHandlerAuxFront() :
																		  m_parentObject->spriteHandlerAuxBack();

	Q_ASSERT(handler);

	return (handler->currentSprite() == m_spriteName);
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
 * @brief TiledEffect::playSprite
 * @param path
 * @param sprite
 */

void TiledEffect::playSprite(const QString &path, const TextureSprite &sprite)
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

void TiledEffect::playSprite(const QString &path, const TextureSprite &sprite, const bool &replaceCurrentSprite)
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

void TiledEffect::playSprite(const QString &path, const TextureSprite &sprite,
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

void TiledEffect::playSprite(const QString &path, const TextureSprite &sprite, const bool &replaceCurrentSprite,
							 const QString &soundPath, const float &baseVolume)
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
			break;

		case TiledObject::AuxBack:
			m_parentObject->spriteHandlerAuxBack()->clear();
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
		m_staticSpriteName,
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
		m_staticSpriteName,
		4,
		0, 0, 64, 64,
		90,
		1
	};

	static const TiledObjectSprite spriteRow2 = {
		m_staticSpriteName,
		4,
		0, 64, 64, 64,
		90,
		1
	};

	static const TiledObjectSprite spriteBall1 = {
		m_staticSpriteName,
		10,
		0, 0, 184, 85,
		45,
		1,
		true
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
		{ SparkBall1, { QStringLiteral(":/rpg/common/sparkBall1.png"), &spriteBall1 } },
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
		m_staticSpriteName,
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
		m_staticSpriteName,
		25,
		256, 0, 128, 126,
		30,
		1,
		true
	};

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/rpg/common/explosion_texture.json"));

	if (!ptr)
		return;

	TextureSpriteDef def;
	def.fromJson(*ptr);

	TextureSprite s = TiledGame::spriteFromMapper(TiledEffectNS::fromSprite(sprite, QStringLiteral("default")),
												  def, QStringLiteral("default"));

	playSprite(QStringLiteral(":/rpg/common/explosion_texture.png"), s, true);
}




/**
 * @brief TiledEffectSleep::play
 */

void TiledEffectSleep::play()
{
	static const TiledObjectSprite sprite = {
		m_staticSpriteName,
		21,
		0, 0, 128, 128,
		30,
		0
	};

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/rpg/common/sparkle_texture.json"));

	if (!ptr)
		return;

	TextureSpriteDef def;
	def.fromJson(*ptr);

	TextureSprite s = TiledGame::spriteFromMapper(TiledEffectNS::fromSprite(sprite, QStringLiteral("default")),
												  def, QStringLiteral("default"));

	playSprite(QStringLiteral(":/rpg/common/sparkle_texture.png"), s);
}




/**
 * @brief TiledEffectRing::play
 */

void TiledEffectRing::play()
{
	static const TiledObjectSprite sprite = {
		m_staticSpriteName,
		8*8,
		0, 0, 128, 128,
		15,
		0,
		true
	};

	TiledSpriteHandler *front = m_parentObject->spriteHandlerAuxFront();
	TiledSpriteHandler *back = m_parentObject->spriteHandlerAuxBack();

	Q_ASSERT(front);
	Q_ASSERT(back);

	const auto &ptr = TiledObject::toTextureSprite(sprite, m_source);

	if (!ptr)
		return;

	front->clear();
	front->setOpacityMask(TiledSpriteHandler::MaskBottom);
	front->setWidth(128);
	front->setHeight(128);
	front->addSprite(ptr.value(), QStringLiteral("default"), TiledObject::Direction::Invalid, m_source);
	front->setProperty("alignToBody", true);

	back->clear();
	back->setOpacityMask(TiledSpriteHandler::MaskTop);
	back->setWidth(128);
	back->setHeight(128);
	back->addSprite(ptr.value(), QStringLiteral("default"), TiledObject::Direction::Invalid, m_source);
	back->setProperty("alignToBody", true);

	front->setSyncHandlers(true);

	front->jumpToSprite(m_staticSpriteName, TiledObject::Direction::Invalid, TiledSpriteHandler::JumpImmediate);
}


