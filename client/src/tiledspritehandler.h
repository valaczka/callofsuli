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
	Q_PROPERTY(bool clearAtEnd READ clearAtEnd WRITE setClearAtEnd NOTIFY clearAtEndChanged FINAL)

public:
	explicit TiledSpriteHandler(QQuickItem *parent = nullptr);
	virtual ~TiledSpriteHandler();

	enum JumpMode {
		JumpImmediate,
		JumpAtFinished
	};

	Q_ENUM(JumpMode);

	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

	bool addSprite(const TiledObjectSprite &sprite, const QString &layer, const QString &source);
	bool addSprite(const TiledObjectSprite &sprite, const QString &layer,
				   const TiledObject::Direction &direction, const QString &source);

	bool jumpToSprite(const QString &name, const TiledObject::Direction &direction, const JumpMode &mode = JumpImmediate);

	void clear();

	const QStringList &spriteNames() const;
	const QStringList &layers() const;

	const QString &currentSprite() const;
	void setCurrentSprite(const QString &newCurrentSprite);

	TiledObject *baseObject() const;
	void setBaseObject(TiledObject *newBaseObject);

	const QStringList &visibleLayers() const;
	QStringList &visibleLayers() { return m_visibleLayers; }
	void setVisibleLayers(const QStringList &newVisibleLayers);

	bool clearAtEnd() const;
	void setClearAtEnd(bool newClearAtEnd);

signals:
	void currentSpriteChanged();
	void baseObjectChanged();
	void clearAtEndChanged();

protected:
	void timerEvent(QTimerEvent *) override final;

private:
	struct Sprite {
		QString layer;
		TiledObject::Direction direction = TiledObject::Invalid;
		TiledObjectSprite data;
		QSGTexture *texture = nullptr;
	};

	bool createSpriteItem(const TiledObjectSprite &sprite, const QString &source,
						  const QString &layer = QStringLiteral("default"),
						  const TiledObject::Direction &direction = TiledObject::Invalid);

	QList<QVector<Sprite>::const_iterator> find(const QString &baseName,
					const TiledObject::Direction &direction = TiledObject::Invalid) const;

	std::optional<QVector<Sprite>::const_iterator> findFirst(const QString &baseName,
					const TiledObject::Direction &direction = TiledObject::Invalid) const;

	bool exists(const QString &baseName,
			  const TiledObject::Direction &direction = TiledObject::Invalid) const;

	bool exists(const QString &baseName, const QString &layer,
			  const TiledObject::Direction &direction = TiledObject::Invalid) const;


	void changeSprite(const QString &name, const TiledObject::Direction &direction);


	QVector<Sprite> m_spriteList;
	QStringList m_spriteNames;
	QString m_currentSprite;
	TiledObject::Direction m_currentDirection = TiledObject::Invalid;

	QStringList m_layers;
	QStringList m_visibleLayers = { QStringLiteral("default") };

	Sprite m_jumpToSprite;
	QPointer<TiledObject> m_baseObject;
	QBasicTimer m_timer;
	int m_currentFrame = 0;
	bool m_clearAtEnd = false;
};




#endif // TILEDSPRITEHANDLER_H
