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



class TiledObjectSprite : public QSerializer
{
	Q_GADGET

public:
	TiledObjectSprite()
		: count(0)
		, duration(0)
		, height(0)
		, width(0)
		, x(0)
		, y(0)
		, loops(0)
	{}

	QUrl source() const { return m_source; }
	void setSource(const QString &source) { m_source = QUrl(source); }

private:
	QUrl m_source;

	QS_SERIALIZABLE
	QS_FIELD(int, count)
	QS_FIELD(int, duration)
	QS_FIELD(int, height)
	QS_FIELD(int, width)
	QS_FIELD(int, x)
	QS_FIELD(int, y)
	QS_FIELD(int, loops)
	QS_FIELD(QString, name)

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
 * @brief The TiledMapObjectAlterableSprite class
 */

class TiledMapObjectAlterableSprite : public QSerializer
{
	Q_GADGET

public:
	TiledMapObjectAlterableSprite()
	{}

	QS_SERIALIZABLE
	QS_QT_DICT(QMap, QString, QString, alterations)
	QS_COLLECTION_OBJECTS(QList, TiledObjectSprite, sprites)
};





/**
 * @brief The TiledObjectAlterableSpriteList class
 */

class TiledObjectAlterableSpriteList : public QSerializer
{
	Q_GADGET

public:
	TiledObjectAlterableSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, TiledMapObjectAlterableSprite, list)
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

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, IsometricObjectSprite, sprites)
};






/**
 * @brief The TiledMapObjectAlterableSprite class
 */

class IsometricObjectAlterableSprite : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectAlterableSprite()
	{}

	QS_SERIALIZABLE
	QS_QT_DICT(QMap, QString, QString, alterations)
	QS_COLLECTION_OBJECTS(QList, IsometricObjectSprite, sprites)
};





/**
 * @brief The TiledMapObjectAlterableSpriteList class
 */

class IsometricObjectAlterableSpriteList : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectAlterableSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, IsometricObjectAlterableSprite, list)
};




#endif // TILEDOBJECTSPRITEDEF_H
