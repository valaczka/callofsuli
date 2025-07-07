/*
 * ---- Call of Suli ----
 *
 * rpgcontrol.h
 *
 * Created on: 2025. 05. 16.
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

#include "rpggamedataiface.h"
#include "rpgquestion.h"
#include "rpgplayer.h"
#include "tiledvisualitem.h"
#include "tilelayeritem.h"
#include <rpgconfig.h>
#include <libtiled/grouplayer.h>
#include <libtiled/imagelayer.h>
#include "tiledscene.h"
#include "application.h"


class RpgGame;


/**
 * @brief The RpgControlBase class
 */

class RpgControlBase
{
public:

	RpgControlBase(const RpgConfig::ControlType &type);
	virtual ~RpgControlBase() = default;

	const RpgConfig::ControlType &type() const { return m_type; }

	virtual void linkControls(const std::vector<std::unique_ptr<RpgControlBase>> &controls) {
		Q_UNUSED(controls);
	}

	RpgGame *game() const { return m_game; }
	void setGame(RpgGame *newGame) { m_game = newGame; }

	virtual void onShapeAboutToDelete(cpShape *shape) { Q_UNUSED(shape); }

protected:
	const RpgConfig::ControlType m_type;
	RpgGame *m_game = nullptr;
};








/**
 * @brief The RpgControl class
 */

template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<RpgGameData::Control, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<RpgGameData::ControlBaseData, T2>::value>::type>
class RpgControl : public RpgControlBase, public RpgGameDataInterface<T, T2>
{
public:
	virtual ~RpgControl() = default;

protected:
	RpgControl(const RpgConfig::ControlType &type)
		: RpgControlBase(type)
		, RpgGameDataInterface<T, T2>()
	{}

};






class RpgActiveIface;
class RpgPlayer;



/**
 * @brief The RpgActiveControl class
 */


class RpgActiveControlObject : public QObject, public TiledObjectBody
{
	Q_OBJECT

	Q_PROPERTY(RpgConfig::ControlType type READ type CONSTANT FINAL)
	Q_PROPERTY(QString keyLock READ keyLock NOTIFY keyLockChanged FINAL)
	Q_PROPERTY(bool questionLock READ questionLock NOTIFY questionLockChanged FINAL)
	Q_PROPERTY(bool isLocked READ isLocked NOTIFY isLockedChanged FINAL)
	Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(bool inVisibleArea READ inVisibleArea WRITE setInVisibleArea NOTIFY inVisibleAreaChanged FINAL)

public:
	explicit RpgActiveControlObject(RpgActiveIface *iface,
									const QPolygonF &polygon,
									TiledGame *game,
									Tiled::MapRenderer *renderer = nullptr,
									const cpBodyType &type = CP_BODY_TYPE_STATIC);

	explicit RpgActiveControlObject(RpgActiveIface *iface,
									const QPointF &center, const qreal &radius,
									TiledGame *game,
									Tiled::MapRenderer *renderer = nullptr,
									const cpBodyType &type = CP_BODY_TYPE_STATIC);

	explicit RpgActiveControlObject(RpgActiveIface *iface,
									const Tiled::MapObject *object,
									TiledGame *game,
									Tiled::MapRenderer *renderer = nullptr,
									const cpBodyType &type = CP_BODY_TYPE_STATIC);

	virtual ~RpgActiveControlObject() = default;

	const RpgConfig::ControlType &type() const;

	RpgActiveIface *activeControl() const { return m_iface; }

	QString keyLock() const;
	bool questionLock() const;
	bool isLocked() const;
	bool isActive() const;

	bool inVisibleArea() const;
	void setInVisibleArea(bool newInVisibleArea);

signals:
	void keyLockChanged();
	void questionLockChanged();
	void isLockedChanged();
	void isActiveChanged();
	void inVisibleAreaChanged();

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	void connectScene();
	virtual void updateInVisibleArea();

	RpgActiveIface *m_iface = nullptr;
	bool m_inVisibleArea = false;

	QMarginsF m_visibleAreaMargins;

private:
	void checkVisibleForContactedPlayer() const;
	bool hasContactedPlayer() const;

	friend class RpgActiveIface;
};







class RpgGame;


/**
 * @brief The RpgActiveIface class
 */

class RpgActiveIface
{
public:
	RpgActiveIface() = default;
	virtual ~RpgActiveIface();

	virtual const RpgConfig::ControlType &activeType() const = 0;

	virtual RpgGameData::BaseData pureBaseData() const = 0;

	virtual bool loadFromGroupLayer(RpgGame *game, TiledScene *scene,
									Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr) = 0;

	TiledVisualItem *createVisualItem(TiledScene *scene, Tiled::GroupLayer *layer);

	TiledVisualItem* visualItem() const;
	void setVisualItem(TiledVisualItem *item);

	const QList<QPointer<RpgActiveControlObject> > &controlObjectList() const { return m_controlObjectList; }
	RpgActiveControlObject *controlObjectAdd(RpgActiveControlObject *object);

	const QList<QPointer<QQuickItem> > &overlays() const { return m_overlays; }
	void overlayAdd(QQuickItem *item);

	QPointF basePosition() const { return m_basePosition; }
	void setBasePosition(QPointF newBasePosition) { m_basePosition = newBasePosition; }

	const QString &keyLock() const { return m_keyLock; }
	void setKeyLock(const QString &newKey);

	const bool &questionLock() const { return m_questionLock; }
	void setQuestionLock(const bool &newLock);

	const bool &isLocked() const { return m_isLocked; }
	void setIsLocked(const bool &newLocked);

	const bool &isActive() const { return m_isActive; }
	void setIsActive(const bool &newActive);


	enum DefaultEnum {
		Default = 0
	};

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other);
	virtual void onShapeContactEnd(cpShape *self, cpShape *other);

	virtual bool loadFromLayer(RpgGame *game, TiledScene *scene,
							   Tiled::Layer *layer, Tiled::MapRenderer *renderer = nullptr) = 0;

	virtual void refreshVisualItem() = 0;

	virtual void onActivated();
	virtual void onDeactivated();

	virtual void updateGlow(const bool &glow);
	virtual void updateOverlays();

	QString m_keyLock;
	bool m_questionLock = false;
	bool m_isLocked = false;
	bool m_isActive = false;

	QPointF m_basePosition;
	qint64 m_lastSyncTick = -1;

	TiledVisualItem* m_visualItem = nullptr;
	QList<QPointer<RpgActiveControlObject> > m_controlObjectList;
	QList<QPointer<QQuickItem>> m_overlays;

	QList<cpShape *> m_contactedFixtures;

	friend class RpgActiveControlObject;
};






/**
 * @brief The RpgActiveControl class
 */

template <typename T, typename T2, typename E,
		  typename = std::enable_if<std::is_base_of<RpgGameData::ControlActive, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<RpgGameData::ControlActiveBaseData, T2>::value>::type>
class RpgActiveControl : public RpgControl<T, T2>, public RpgActiveIface
{
public:
	virtual ~RpgActiveControl() = default;

	virtual RpgGameData::BaseData pureBaseData() const override;

	virtual const RpgConfig::ControlType &activeType() const override { return RpgControlBase::type(); }

	QString stateString(const E &key) const { return m_stateHash.key(key); }

	E stateValue(const QString &str) const { return m_stateHash.value(str); }
	E stateValue(const QString &str, const E &defaultKey) const { return m_stateHash.value(str, defaultKey); }

	const QHash<E, QList<QPointer<QQuickItem> > > &tileLayerList() const { return m_tileLayerList; }

	virtual bool loadFromGroupLayer(RpgGame *game, TiledScene *scene,
									Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr) override;

	const E &currentState() const { return m_currentState; }
	void setCurrentState(const E &newState);

protected:
	RpgActiveControl(const RpgConfig::ControlType &type)
		: RpgControl<T, T2>(type)
		, RpgActiveIface()
	{  }

	virtual void refreshVisualItem() override final;
	virtual void updateGlow(const bool &glow) override final;
	virtual void onShapeAboutToDelete(cpShape *shape) override;
	virtual void onCurrentStateChanged() {}

	bool loadQuestion(const RpgGameData::SnapshotInterpolation<T> &snapshot,
					  RpgPlayer *player, RpgQuestion *question);

	virtual bool loadFromLayer(RpgGame *game, TiledScene *scene,
							   Tiled::Layer *layer, Tiled::MapRenderer *renderer = nullptr) override {
		Q_UNUSED(game);
		Q_UNUSED(scene);
		Q_UNUSED(layer);
		Q_UNUSED(renderer);
		return false;
	}

	QQuickItem *tileLayerAdd(const E &state, QQuickItem *layer);

	E m_currentState;

	QHash<QString, E> m_stateHash;

	struct VisualInfo {
		QPointF relativePosition;
		QUrl image;
	};

	QHash<E, VisualInfo> m_visualInfo;

	QHash<E, QList<QPointer<QQuickItem> > > m_tileLayerList;


};









/**
 * @brief RpgActiveControl::pureBaseData
 * @return
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline RpgGameData::BaseData RpgActiveControl<T, T2, E, T4, T5>::pureBaseData() const
{
	return RpgControl<T,T2>::baseData();
}




/**
 * @brief RpgActiveControl::refreshVisualItem
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline void RpgActiveControl<T, T2, E, T4, T5>::refreshVisualItem()
{
	for (auto it = m_tileLayerList.cbegin(); it != m_tileLayerList.cend(); ++it) {
		const bool visible = it.key() == m_currentState;

		for (QQuickItem *item  : *it) {
			if (item)
				item->setVisible(visible);
		}
	}

	if (!m_visualItem)
		return;

	const auto &it = m_visualInfo.find(m_currentState);

	if (it == m_visualInfo.cend()) {
		m_visualItem->setSource({});
		m_visualItem->setVisible(false);
	} else {
		m_visualItem->setSource(it->image);
		m_visualItem->setPosition(m_basePosition + it->relativePosition);
		m_visualItem->setVisible(true);
	}
}




/**
 * @brief RpgActiveControl::setCurrentState
 * @param newState
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline void RpgActiveControl<T, T2, E, T4, T5>::setCurrentState(const E &newState)
{
	if (newState == m_currentState)
		return;

	m_currentState = newState;
	onCurrentStateChanged();
	refreshVisualItem();
}



/**
 * @brief RpgActiveControl::updateGlow
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline void RpgActiveControl<T, T2, E, T4, T5>::updateGlow(const bool &glow)
{
	RpgActiveIface::updateGlow(glow);

	for (const auto &list : m_tileLayerList) {
		for (QQuickItem *item : list) {
			if (TiledVisualItem *i = qobject_cast<TiledVisualItem*>(item))
				i->setGlowEnabled(glow);
		}
	}

}


/**
 * @brief RpgActiveControl::onShapeAboutToDelete
 * @param shape
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline void RpgActiveControl<T, T2, E, T4, T5>::onShapeAboutToDelete(cpShape *shape)
{
	m_contactedFixtures.removeAll(shape);
}





/**
 * @brief RpgActiveControl::loadQuestion
 * @param snapshot
 * @return
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline bool RpgActiveControl<T, T2, E, T4, T5>::loadQuestion(const RpgGameData::SnapshotInterpolation<T> &snapshot,
															 RpgPlayer *player, RpgQuestion *question)
{
	if (!player ||
			!snapshot.last.u.isValid() ||
			!snapshot.last.u.isBaseEqual(player->baseData()))
		return false;

	if (snapshot.last.f <= m_lastSyncTick) {
		return false;
	}


	LOG_CINFO("game") << "LOAD CONTAINER FOR" << snapshot.last.u.o;

	if (question) {
		if (question->control() == this)
			return false;

		if (question->nextQuestion(player, this)) {
			Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
			m_lastSyncTick = snapshot.last.f;
		}
	}

	return true;
}







/**
 * @brief RpgActiveControl::tileLayerAdd
 * @param state
 * @param layer
 * @return
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline QQuickItem *RpgActiveControl<T, T2, E, T4, T5>::tileLayerAdd(const E &state, QQuickItem *layer)
{
	if (!layer)
		return nullptr;

	QList<QPointer<QQuickItem> > &list = m_tileLayerList[state];

	if (list.contains(layer))
		return nullptr;

	list.append(layer);

	return layer;
}





/**
 * @brief RpgActiveControl::loadFromGroupLayer
 * @param game
 * @param scene
 * @param group
 * @return
 */

template<typename T, typename T2, typename E, typename T4, typename T5>
inline bool RpgActiveControl<T, T2, E, T4, T5>::loadFromGroupLayer(RpgGame *game, TiledScene *scene,
																   Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);

	setBasePosition(group->position()+group->totalOffset());

	for (Tiled::Layer *layer : std::as_const(*group)) {

		if (loadFromLayer(game, scene, layer, renderer))
			continue;

		if (Tiled::ImageLayer *tl = layer->asImageLayer()) {
			const auto &it = m_stateHash.find(tl->name());

			if (it == m_stateHash.cend()) {
				LOG_CWARNING("scene") << "Invalid image layer" << tl->name();
				continue;
			}

			m_visualInfo[it.value()] = VisualInfo {
									   .relativePosition = tl->position() + tl->offset(),
									   .image = tl->imageSource()

		};

			LOG_CTRACE("scene") << "Add image layer" << tl->name() << "url:" << tl->imageSource() << this;

		} else if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			if (tl->className() == QStringLiteral("overlay")) {
				overlayAdd(scene->addTileLayer(tl, renderer));
				continue;
			}

			auto it = m_stateHash.find(tl->name());

			if (it == m_stateHash.cend()) {
				it = m_stateHash.find(tl->className());
			}

			if (it == m_stateHash.cend()) {
				scene->addTileLayer(tl, renderer);
				continue;
			}

			// TODO: add with glow effects (e.g. glow) - texture size problem!

			tileLayerAdd(it.value(), scene->addTileLayer(tl, renderer));

			/*TiledVisualItem *vItem = scene->addVisualItem(tl, renderer);

			// A group neve alapján rendezi el

			vItem->setName(group->name());
			vItem->setGlowColor(QStringLiteral("#FFF59D"));

			tileLayerAdd(it.value(), vItem);

			LOG_CINFO("scene") << "Add tile layer" << tl->name() << this << vItem->name() << vItem->size() << vItem->position()
							   << tl->offset() << tl->position() << tl->totalOffset();*/
		}
	}

	if (!m_visualItem && !m_visualInfo.isEmpty()) {
		TiledVisualItem *item = createVisualItem(scene, group);

		LOG_CTRACE("scene") << "Add visual item" << this << item->name();
	}

	refreshVisualItem();

	return true;
}



#endif // RPGCONTROL_H
