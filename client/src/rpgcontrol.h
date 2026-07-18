/*
 * ---- Call of Suli ----
 *
 * rpgcontrol.h
 *
 * Created on: 2026. 07. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControl
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

#ifndef RPGCONTROL_H
#define RPGCONTROL_H

#include "rpgobject.h"
#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include <QQmlEngine>



/**
 * @brief The RpgDefender class
 */

class RpgControl : public RpgObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool isAlive READ isAlive WRITE setIsAlive NOTIFY isAliveChanged FINAL)

public:
	RpgControl(RpgGameItem *gameItem, const Rpg::Control &config);
	virtual ~RpgControl();

	enum StateCommon {
		StateNormal,
		StateActive,
		StateDestroyed
	};

	Q_ENUM(StateCommon);

	static RpgControl* createControl(const Rpg::Control &config, RpgGameItem *gameItem, TiledScene *scene);

	//virtual void initialize() override;

	const Rpg::Control &config() const;

	bool isAlive() const;
	void setIsAlive(bool newIsAlive);

signals:

	void isAliveChanged();

protected:
	const Rpg::Control m_config;
	bool m_isAlive = false;

	TiledScene *m_scene = nullptr;
	QList<TiledQuick::TileLayerItem *> m_layerItems;

	QQuickItem *m_markerItem = nullptr;

	friend class RpgControlMotor;
};






/**
 * @brief The RpgControlMotor class
 */

class RpgControlMotor : public AbstractRpgMotor
{
public:
	RpgControlMotor(RpgControl *object)
		: AbstractRpgMotor(object)
		, m_control(object)
	{}

	virtual void updateBody(TiledObject *) override {};
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;

	virtual bool beforeWorldStepControl(const RpgStream::ControlState &state, entt::entity &entity) {
		Q_UNUSED(state);
		Q_UNUSED(entity);
		return true;
	};

protected:
	QPointer<RpgControl> m_control;
};











/**
 * @brief The RpgControlCommonBase class
 */

template <typename T>
class RpgControlCommonIface
{
public:
	RpgControlCommonIface(const QString &name, const Rpg::Control &config, const T &stateDefault)
		: m_stateDefault(stateDefault)
		, m_name(name)
		, m_visual(stateDefault)
	{
		m_visual.setBasePosition(TiledObjectBody::toPointF(config.pos));
	}

	virtual ~RpgControlCommonIface() {
		m_visual.clear();
	}


protected:
	virtual QHash<QString, T> stateHash() const = 0;
	virtual QHash<T, QString> baseImageHash() const = 0;

	const T m_stateDefault;

	TiledVisualItem* loadFromCommonMap(RpgGame *game, TiledScene *scene,
									   QList<TiledQuick::TileLayerItem *> *layerItems,
									   const QString &name, const QHash<QString, T> &stateHash,
									   const QHash<T, QString> &baseImageHash) {
		Q_ASSERT(game);
		Q_ASSERT(scene);
		Q_ASSERT(layerItems);

		const Tiled::Map *map = game->commonMap(name);
		Tiled::MapRenderer *renderer = game->commonRenderer(name);

		if (!map) {
			LOG_CERROR("game") << "Invalid map";
			return nullptr;
		}

		if (!renderer) {
			LOG_CERROR("game") << "Invalid renderer";
			return nullptr;
		}

		std::optional<QPointF> ref;

		for (Tiled::Layer *layer : map->layers()) {
			if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
				for (Tiled::MapObject *object : std::as_const(gr->objects())) {
					if (object->className() == QStringLiteral("base")) {
						ref = renderer->pixelToScreenCoords(object->position())+gr->totalOffset();
					}
				}
			}
		}

		TiledVisualItem *item = scene->addVisualItem();

		m_visual.setImageItem(item);


		for (const auto &[st, url] : baseImageHash.asKeyValueRange())
			m_visual.addSource(st, QUrl::fromLocalFile(url));


		for (Tiled::Layer *layer : map->layers()) {
			if (Tiled::TileLayer *tl = layer->asTileLayer()) {
				TiledQuick::TileLayerItem *layerItem = scene->addTileLayer(tl, renderer);

				const T st = stateHash.value(tl->className(), m_stateDefault);

				QPointF r;
				if (ref.has_value())
					r = ref.value();
				else {
					r.setX(layerItem->width()/2);
					r.setY(layerItem->height()/2);
				}

				QObject::connect(item, &TiledVisualItem::zChanged, layerItem, [item, layerItem]() {
					layerItem->setZ(item->z());
				});
				layerItem->setPosition(layerItem->position() + m_visual.basePosition() - r);

				layerItems->append(layerItem);
				m_visual.addLayer(st, layerItem);
			}
		}

		item->setGlowColor(RpgGame::colorGlow());

		return item;
	}

protected:
	const QString m_name;
	RpgVisualState<T> m_visual;
};





/**
 * @brief The RpgControlCommon class
 */

class RpgControlCommon : public RpgControl, public RpgControlCommonIface<RpgControl::StateCommon>
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgControlCommon(const QString &name, RpgGameItem *gameItem, const Rpg::Control &config,
					 const QHash<RpgControl::StateCommon, QString> &baseImageHash = {},
					 const QString &displayName = {})
		: RpgControl(gameItem, config)
		, RpgControlCommonIface(name, config, StateNormal)
		, m_baseImageHash(baseImageHash)
	{
		if (!displayName.isEmpty())
			setDisplayName(displayName);

		connect(this, &RpgControlCommon::isAliveChanged, this, [this]() {
			m_visual.setState(m_isAlive ? StateNormal : StateDestroyed);
			if (m_markerItem)
				m_markerItem->setVisible(m_isAlive);
		});
	}

	virtual void initialize() override {
		m_scene = scene();

		Q_ASSERT(m_scene);

		TiledVisualItem* item = loadFromCommonMap(m_rpgGame, m_scene, &m_layerItems, m_name, stateHash(), baseImageHash());

		if (!item) {
			LOG_CERROR("game") << "Common control load failed" << m_name;
			return;
		}

		m_visualItem = item;

		resetMarkerDisplay(m_displayName);
	}

	void resetMarkerDisplay(const QString &displayName) {
		if (displayName.isEmpty())
			return;

		if (!m_markerItem)
			m_markerItem = createMarkerItem();

		setDisplayName(displayName);
	}

protected:
	virtual QHash<QString, RpgControl::StateCommon> stateHash() const override {
		static const QHash<QString, RpgControl::StateCommon> hash = {
			{ "active", StateActive },
			{ "destroyed", StateDestroyed },
			{ "normal", StateNormal }
		};

		return hash;
	}

	virtual QHash<RpgControlCommon::StateCommon, QString> baseImageHash() const override { return m_baseImageHash; }

	const QHash<RpgControl::StateCommon, QString> m_baseImageHash;

};








#endif // RPGCONTROL_H
