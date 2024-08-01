/*
 * ---- Call of Suli ----
 *
 * tiledobjectspritedef.h
 *
 * Created on: 2024. 03. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef TILEDOBJECTSPRITEDEF_H
#define TILEDOBJECTSPRITEDEF_H

#include <QSerializer>


/**
 * @brief The TextureSpriteFrameRect class
 */

class TextureSpriteFrameRect : public QSerializer
{
	Q_GADGET

public:
	TextureSpriteFrameRect()
		: QSerializer()
		, x(0)
		, y(0)
		, w(0)
		, h(0)
	{}

private:
	QS_SERIALIZABLE

	QS_FIELD(int, x)
	QS_FIELD(int, y)
	QS_FIELD(int, w)
	QS_FIELD(int, h)
};




/**
 * @brief The TextureSpriteFrameSize class
 */

class TextureSpriteFrameSize : public QSerializer
{
	Q_GADGET

public:
	TextureSpriteFrameSize(int _w = 0, int _h = 0)
		: QSerializer()
		, w(_w)
		, h(_h)
	{}

private:
	QS_SERIALIZABLE

	QS_FIELD(int, w)
	QS_FIELD(int, h)
};





/**
 * @brief The TextureSpriteFrame class
 */

class TextureSpriteFrame : public QSerializer
{
	Q_GADGET

public:
	TextureSpriteFrame()
		: QSerializer()
	{}

private:
	QS_SERIALIZABLE

	QS_OBJECT(TextureSpriteFrameRect, frame)
	QS_OBJECT(TextureSpriteFrameRect, spriteSourceSize)
	QS_OBJECT(TextureSpriteFrameSize, sourceSize)
};





/**
 * @brief The TextureSpriteDef class
 */

class TextureSpriteDef : public QSerializer
{
	Q_GADGET

public:
	TextureSpriteDef()
		: QSerializer()
	{}

private:
	QS_SERIALIZABLE

	QS_FIELD(QJsonObject, meta)
	QS_QT_DICT_OBJECTS(QMap, int, TextureSpriteFrame, frames)
};




/**
 * @brief The TiledObjectSprite class
 */

class TiledObjectSprite : public QSerializer
{
	Q_GADGET

public:
	TiledObjectSprite()
		: count(0)
		, x(0)
		, y(0)
		, width(0)
		, height(0)
		, duration(0)
		, loops(0)
		, flow(false)
	{}

	TiledObjectSprite(const QString &_name,
					  const int &_count,
					  const int &_x,
					  const int &_y,
					  const int &_width,
					  const int &_height,
					  const int &_duration,
					  const int &_loops = 0,
					  const bool &_flow = false)
		: name(_name)
		, count(_count)
		, x(_x)
		, y(_y)
		, width(_width)
		, height(_height)
		, duration(_duration)
		, loops(_loops)
		, flow(_flow)
	{}

private:
	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_FIELD(int, count)
	QS_FIELD(int, x)
	QS_FIELD(int, y)
	QS_FIELD(int, width)
	QS_FIELD(int, height)
	QS_FIELD(int, duration)
	QS_FIELD(int, loops)					// 0: forever, -1: forever forward+reverse
	QS_FIELD(bool, flow)

};




/**
 * @brief The TiledObjectSpriteList class
 */

class TiledObjectSpriteList : public QSerializer
{
	Q_GADGET

public:
	TiledObjectSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, TiledObjectSprite, sprites)
};






/**
 * @brief The IsometricObjectSpriteDirection class
 */

class IsometricObjectSprite : public TiledObjectSprite
{
	Q_GADGET

public:
	IsometricObjectSprite()
		: startRow(0)
		, startColumn(-1)
	{}

	QS_SERIALIZABLE

	QS_FIELD(int, startRow)
	QS_FIELD(int, startColumn)
	QS_COLLECTION(QList, int, directions)
};



/**
 * @brief The IsometricObjectSprite class
 */

class IsometricObjectSpriteList : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectSpriteList()
	{}

	IsometricObjectSpriteList(const QByteArray &data)
	{
		this->fromJson(data);
	}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, IsometricObjectSprite, sprites)
};






/**
 * @brief The TextureSprite class
 */

class TextureSprite : public QSerializer
{
	Q_GADGET

public:
	TextureSprite()
		: QSerializer()
		, duration(0)
		, loops(0)
	{}

	TextureSprite(const QString &_name,
				  const int &_width,
				  const int &_height,
				  const int &_duration,
				  const QVector<TextureSpriteFrame> &_frames,
				  const int &_loops = 0)
		: QSerializer()
		, name(_name)
		, size({_width, _height})
		, frames(_frames)
		, duration(_duration)
		, loops(_loops)
	{}

	TextureSprite(const QString &_name,
				  const int &_width,
				  const int &_height,
				  const int &_duration,
				  const int &_loops,
				  const QVector<TextureSpriteFrame> &_frames)
		: QSerializer()
		, name(_name)
		, size({_width, _height})
		, frames(_frames)
		, duration(_duration)
		, loops(_loops)
	{}

private:
	QS_SERIALIZABLE

	QS_FIELD(QString, name)
	QS_OBJECT(TextureSpriteFrameSize, size)
	QS_COLLECTION_OBJECTS(QVector, TextureSpriteFrame, frames)

	QS_FIELD(int, duration)
	QS_FIELD(int, loops)					// 0: forever, -1: forever forward+reverse

};

#endif // TILEDOBJECTSPRITEDEF_H
