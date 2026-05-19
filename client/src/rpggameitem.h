/*
 * ---- Call of Suli ----
 *
 * rpggameitem.h
 *
 * Created on: 2026. 05. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGameItem
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

#ifndef RPGGAMEITEM_H
#define RPGGAMEITEM_H

#include <QQmlEngine>
#include "tiledgame.h"
#include "rpggame.h"
#include "tiledvisualitem.h"



/**
 * @brief The RpgGameItem class
 */

class RpgGameItem : public TiledGame
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgGame* game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(bool isContentReady READ isContentReady NOTIFY isContentReadyChanged FINAL)

public:
	RpgGameItem(QQuickItem *parent = nullptr);
	virtual ~RpgGameItem();


	enum Fixture {
		FixtureInvalid			= 0,
		FixtureGround			= 1 << 0,
		FixtureExcluded			= 1 << 1,
		FixturePlayerBody		= 1 << 2,
		FixturePlayerTarget		= 1 << 3,
		FixtureSensor			= 1 << 4,
		FixtureVirtualCircle	= 1 << 5,
		FixtureControl			= 1 << 6,


		FixtureAll =
		FixtureGround |
		FixtureExcluded	|
		FixturePlayerBody |
		FixturePlayerTarget |
		FixtureSensor |
		FixtureVirtualCircle |
		FixtureControl

	};

	Q_ENUM(Fixture)
	Q_DECLARE_FLAGS(Fixtures, Fixture)

	static const QHash<RpgStream::Team, QColor> &teamColor() { return m_teamColor; }

	RpgGame *game() const;
	void setGame(RpgGame *newGame);

	bool load(const RpgGameDefinition &def);

	static QRect loadTextureSprites(TiledSpriteHandler *handler, const QString &path);

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers) override;

	bool isContentReady() const;
	void setIsContentReady(bool newIsContentReady);

protected:
	virtual void sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene) override;
	virtual void loadTileLayer(TiledScene *scene, Tiled::TileLayer *layer, Tiled::MapRenderer *renderer) override;
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer) override;
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer) override;
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer) override;
	virtual void loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer) override;

	virtual void timeStepPrepareEvent() override final;
	virtual void timeBeforeWorldStepEvent(const qint64 &tick) override final;
	virtual void timeAfterWorldStepEvent(const qint64 &tick) override final;
	virtual void timeSteppedEvent() override final;

	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void joystickStateEvent(const Joystick &joystick, const JoystickState &state) override;

signals:
	void gameChanged();
	void isContentReadyChanged();

private:
	void loadTower(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	RpgGamePrivate *d = nullptr;

	RpgGame *m_game = nullptr;
	bool m_isContentReady = false;

	static const QHash<RpgStream::Team, QColor> m_teamColor;

	friend class RpgGame;
	friend class RpgGamePrivate;
	void loadMp(Tiled::GroupLayer *group, TiledScene *scene, Tiled::MapRenderer *renderer);
};






/**
 * @brief The RpgVisualState class
 */

template <typename T>
class RpgVisualState
{
public:
	RpgVisualState(const T &defaultState)
		: m_state(defaultState)
	{}

	const T &state() const { return m_state; }
	void setState(const T &state) {
		m_state = state;
		updateState();
	}


	void refresh() { updateState(); }

	void addLayer(const T &state, QQuickItem *layer) {
		if (!layer)
			return;

		m_layers[state].append(layer);
	}


	TiledVisualItem *imageItem() const { return m_imageItem; }
	void setImageItem(TiledVisualItem *newImageItem) { m_imageItem = newImageItem; }

	void addSource(const T &state, const QUrl &source, const QPointF &offset = {}) {
		m_sources[state].append({.url = source, .offset = offset});
	}

	const QPointF &basePosition() const { return m_basePosition; }
	void setBasePosition(QPointF newBasePosition) { m_basePosition = newBasePosition; }

protected:
	virtual void updateState() {
		for (auto [state, list] : m_layers.asKeyValueRange()) {
			for (QQuickItem *item : list) {
				if (!item)
					continue;

				item->setVisible(state == m_state);
			}
		}

		if (!m_imageItem)
			return;

		if (const auto it = m_sources.find(m_state); it != m_sources.cend()) {
			m_imageItem->setSource(it->url);
			m_imageItem->setPosition(m_basePosition + it->offset);
			m_imageItem->setVisible(true);
		} else {
			m_imageItem->setSource({});
			m_imageItem->setVisible(false);
		}
	}

	struct SourceData {
		QUrl url;
		QPointF offset;
	};

	QHash<T, QList<QPointer<QQuickItem> > > m_layers;
	QHash<T, SourceData> m_sources;

	TiledVisualItem *m_imageItem = nullptr;;
	QPointF m_basePosition;

	T m_state;
};


















#endif // RPGGAMEITEM_H
