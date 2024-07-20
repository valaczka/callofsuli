/*
 * ---- Call of Suli ----
 *
 * tiledeffect.h
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

#ifndef TILEDEFFECT_H
#define TILEDEFFECT_H

#include "tiledobject.h"



/**
 * @brief The TiledEffect class
 */

class TiledEffect
{

public:
	TiledEffect(TiledObject *parentObject, const QString &spriteName = {});
	virtual ~TiledEffect() = default;

	virtual void play() = 0;
	virtual void stop();
	virtual bool active() const;

	void clear();

protected:
	void playSprite(const QString &path, const TiledObjectSprite &sprite);
	void playSprite(const QString &path, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite);
	void playSprite(const QString &path, const TiledObjectSprite &sprite,
					const QString &soundPath, const float &baseVolume = 1.);
	void playSprite(const QString &path, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite,
					const QString &soundPath, const float &baseVolume = 1.);


	const TiledObject *m_parentObject;
	const QString m_spriteName;
	TiledObject::AuxHandler m_auxHandler = TiledObject::AuxFront;
	bool m_alignToBody = false;
};



/**
 * @brief The TiledEffectHealed class
 */

class TiledEffectHealed : public TiledEffect
{
public:
	TiledEffectHealed(TiledObject *parentObject) : TiledEffect(parentObject, m_staticSpriteName) {}
	virtual void play() override;

private:
	static const QString m_staticSpriteName;
};





/**
 * @brief The TiledEffectHealed class
 */

class TiledEffectSleep : public TiledEffect
{
public:
	TiledEffectSleep(TiledObject *parentObject) : TiledEffect(parentObject, m_staticSpriteName) {}
	virtual void play() override;

private:
	static const QString m_staticSpriteName;
};




/**
 * @brief The TiledEffectShield class
 */

class TiledEffectShield : public TiledEffect
{
public:
	TiledEffectShield(TiledObject *parentObject) : TiledEffect(parentObject, m_staticSpriteName) {
		m_auxHandler = TiledObject::AuxBack;
		m_alignToBody = true;
	}

	virtual void play() override;

private:
	static const QString m_staticSpriteName;
};





/**
 * @brief The TiledEffectSpark class
 */

class TiledEffectSpark : public TiledEffect
{
public:
	enum Type {
		SparkInvalid = 0,
		SparkOrange1 = 1,
		SparkOrange2 = 1 << 1,
		SparkBlue1 = 1 << 2,
		SparkBlue2 = 1 << 3,
		SparkRed1 = 1 << 4,
		SparkRed2 = 1 << 5,

		SparkAllOrange = SparkOrange1 | SparkOrange2,
		SparkAllBlue = SparkBlue1 | SparkBlue2,
		SparkAllRed = SparkRed1 | SparkRed2,

		SparkAll = SparkAllOrange | SparkAllBlue | SparkAllRed
	};

	Q_DECLARE_FLAGS(Types, Type);

	TiledEffectSpark(const Types &types, TiledObject *parentObject)
		: TiledEffect(parentObject, m_staticSpriteName)
		, m_types(types)
	{}
	virtual void play() override;

private:
	Types m_types;
	static const QString m_staticSpriteName;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(TiledEffectSpark::Types)





/**
 * @brief The TiledEffectFire class
 */

class TiledEffectFire : public TiledEffect
{
public:
	TiledEffectFire(TiledObject *parentObject)
		: TiledEffect(parentObject, m_staticSpriteName) {
		m_alignToBody = false;
	}
	virtual void play() override;

private:
	static const QString m_staticSpriteName;
};





/**
 * @brief The TiledEffectSmoke class
 */

class TiledEffectSmoke : public TiledEffect
{
public:
	TiledEffectSmoke(TiledObject *parentObject) : TiledEffect(parentObject)
	{}

	virtual void play() override;
	void stop() override;
	bool active() const override;

	bool isRunning() const;
};

#endif // TILEDEFFECT_H
