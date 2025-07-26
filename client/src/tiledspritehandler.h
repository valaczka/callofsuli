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
	Q_PROPERTY(TiledSpriteHandler *handlerMaster READ handlerMaster WRITE setHandlerMaster NOTIFY handlerMasterChanged FINAL)
	Q_PROPERTY(TiledSpriteHandler *handlerSlave READ handlerSlave WRITE setHandlerSlave NOTIFY handlerSlaveChanged FINAL)
	Q_PROPERTY(bool syncHandlers READ syncHandlers WRITE setSyncHandlers NOTIFY syncHandlersChanged FINAL)
	Q_PROPERTY(OpacityMask opacityMask READ opacityMask WRITE setOpacityMask NOTIFY opacityMaskChanged FINAL)

public:
	explicit TiledSpriteHandler(QQuickItem *parent = nullptr);
	virtual ~TiledSpriteHandler();

	enum JumpMode {
		JumpImmediate,
		JumpAtFinished
	};

	Q_ENUM(JumpMode);

	enum OpacityMask {
		MaskFull,
		MaskTop,
		MaskBottom
	};

	Q_ENUM(OpacityMask);

	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

	bool addSprite(const TextureSprite &sprite, const QString &layer, const QString &source);
	bool addSprite(const TextureSprite &sprite, const QString &layer,
				   const TiledObject::Direction &direction, const QString &source);

	bool jumpToSprite(const QString &name, const TiledObject::Direction &direction, const JumpMode &mode = JumpImmediate);

	void clear();

	void setStartFrameSeed(const qreal &percent = -1.);

	void setDirty();
	void updateDirty();

	const TiledObject::Direction &currentDirection() const { return m_currentDirection; }

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

	TiledSpriteHandler *handlerMaster() const;
	void setHandlerMaster(TiledSpriteHandler *newHandlerMaster);

	bool syncHandlers() const;
	void setSyncHandlers(bool newSyncHandlers);

	TiledSpriteHandler *handlerSlave() const;
	void setHandlerSlave(TiledSpriteHandler *newHandlerSlave);

	OpacityMask opacityMask() const;
	void setOpacityMask(const OpacityMask &newOpacityMask);

signals:
	void cleared();
	void currentSpriteChanged();
	void baseObjectChanged();
	void clearAtEndChanged();
	void handlerMasterChanged();
	void syncHandlersChanged();
	void handlerSlaveChanged();
	void opacityMaskChanged();

protected:
	void timerEvent(QTimerEvent *) override final;

private:
	struct Sprite {
		QString layer;
		TiledObject::Direction direction = TiledObject::Invalid;
		TextureSprite data;
		QSGTexture *texture = nullptr;
	};

	bool createSpriteItem(const TextureSprite &sprite, const QString &source,
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

	void createNodes(QSGNode *node, const QList<QVector<TiledSpriteHandler::Sprite>::const_iterator> &iteratorList);


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
	qreal m_startFrameSeed = 0.;
	bool m_isReverse = false;
	bool m_clearAtEnd = false;

	TiledSpriteHandler *m_handlerMaster = nullptr;
	TiledSpriteHandler *m_handlerSlave = nullptr;
	bool m_syncHandlers = false;

	bool m_isDirty = false;

	OpacityMask m_opacityMask = MaskFull;
};




#endif // TILEDSPRITEHANDLER_H
