/*
 * ---- Call of Suli ----
 *
 * tiledspritehandler.h
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledSpriteHandler
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

#ifndef TILEDSPRITEHANDLER_H
#define TILEDSPRITEHANDLER_H

#include <QQuickItem>
#include <QSGGeometryNode>
#include <QSGTexture>
#include "qbasictimer.h"
#include "tiledobjectspritedef.h"
#include "tiledobject.h"

class TiledSpriteHandler : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QString currentSprite READ currentSprite WRITE setCurrentSprite NOTIFY currentSpriteChanged FINAL)
	Q_PROPERTY(TiledObject *baseObject READ baseObject WRITE setBaseObject NOTIFY baseObjectChanged FINAL)

public:
	explicit TiledSpriteHandler(QQuickItem *parent = nullptr);
	virtual ~TiledSpriteHandler();

	enum JumpMode {
		JumpImmediate,
		JumpAtFinished
	};

	Q_ENUM(JumpMode);

	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

	bool addSprite(const TiledObjectSprite &sprite, const QString &source);
	bool addSprite(const TiledObjectSprite &sprite, const QString &alteration, const QString &source);
	bool addSprite(const TiledObjectSprite &sprite,
				   const TiledObject::Direction &direction, const QString &source);
	bool addSprite(const TiledObjectSprite &sprite, const QString &alteration,
				   const TiledObject::Direction &direction, const QString &source);

	bool jumpToSprite(const QString &name, const QString &alteration, const TiledObject::Direction &direction,
					  const JumpMode &mode = JumpImmediate);

	const QStringList &spriteNames() const;

	const QString &currentSprite() const;
	void setCurrentSprite(const QString &newCurrentSprite);

	TiledObject *baseObject() const;
	void setBaseObject(TiledObject *newBaseObject);

signals:
	void currentSpriteChanged();
	void baseObjectChanged();

protected:
	void timerEvent(QTimerEvent *) override final;

private:
	struct Sprite {
		QString alteration;
		TiledObject::Direction direction = TiledObject::Invalid;
		TiledObjectSprite data;
		std::shared_ptr<QSGTexture> shr_texture;

		QSGTexture *texture() const { return shr_texture.get(); }
	};


	bool createSpriteItem(const TiledObjectSprite &sprite, const QString &source,
						  const QString &alteration = QStringLiteral(""),
						  const TiledObject::Direction &direction = TiledObject::Invalid);

	int find(const QString &baseName,
			 const QString &alteration = QStringLiteral(""),
			 const TiledObject::Direction &direction = TiledObject::Invalid) const;

	std::optional<Sprite> findSprite(const QString &baseName,
			 const QString &alteration = QStringLiteral(""),
			 const TiledObject::Direction &direction = TiledObject::Invalid) const;

	void changeSprite(const int &id);

	int m_lastId = 0;
	QHash<int, Sprite> m_spriteList;
	QStringList m_spriteNames;
	QString m_currentSprite;
	int m_currentId = -1;
	int m_jumpToId = -1;
	QPointer<TiledObject> m_baseObject;
	QBasicTimer m_timer;
	int m_currentFrame = 0;
};




#endif // TILEDSPRITEHANDLER_H
