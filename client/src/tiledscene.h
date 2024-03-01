/*
 * ---- Call of Suli ----
 *
 * tiledscene.h
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledScene
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

#ifndef TILEDSCENE_H
#define TILEDSCENE_H

#include "box2dworld.h"
#include "libtiledquick/mapitem.h"
#include "libtiledquick/maploader.h"
#include "tiledobject.h"
#include <QQuickItem>

class TiledScene : public TiledQuick::MapItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged FINAL)
	Q_PROPERTY(QQuickItem *joystick READ joystick WRITE setJoystick NOTIFY joystickChanged FINAL)
	Q_PROPERTY(QQuickItem *followedItem READ followedItem WRITE setFollowedItem NOTIFY followedItemChanged FINAL)
	Q_PROPERTY(JoystickState joystickState READ joystickState WRITE setJoystickState NOTIFY joystickStateChanged FINAL)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged FINAL)
	Q_PROPERTY(QList<TiledObject *> tiledObjects READ tiledObjects WRITE setTiledObjects NOTIFY tiledObjectsChanged FINAL)

public:
	explicit TiledScene(QQuickItem *parent = nullptr);
	virtual ~TiledScene() {}

	/**
	 * @brief The JoystickState class
	 */

	struct JoystickState {
		qreal dx = 0;
		qreal dy = 0;
		qreal angle = 0;
		qreal distance = 0;
		bool hasTouch = false;
		bool hasKeyboard = false;

		inline friend bool operator==(const JoystickState &s1, const JoystickState &s2) {
			return s1.dx == s2.dx &&
					s1.dy == s2.dy &&
					s1.angle == s2.angle &&
					s1.distance == s2.distance &&
					s1.hasTouch == s2.hasTouch &&
					s1.hasKeyboard == s2.hasKeyboard;
		}
	};

	Q_INVOKABLE int getDynamicZ(const QPointF &point, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(const qreal &x, const qreal &y, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(QQuickItem *item, const int &defaultValue = 1) const;

	Q_INVOKABLE bool load(const QUrl &url);

	bool running() const;
	void setRunning(bool newRunning);

	QQuickItem *joystick() const;
	void setJoystick(QQuickItem *newJoystick);

	JoystickState joystickState() const;
	void setJoystickState(const JoystickState &newJoystickState);

	TiledQuick::MapLoader *mapLoader() const;

	bool debugView() const;
	void setDebugView(bool newDebugView);


	QVariantList testPoints() const;
	void setTestPoints(const QVariantList &newTestPoints);

	TiledObject *testObject() const;
	void setTestObject(TiledObject *newTestObject);

	QList<TiledObject *> tiledObjects() const;
	void setTiledObjects(const QList<TiledObject *> &newTiledObjects);

	QQuickItem *followedItem() const;
	void setFollowedItem(QQuickItem *newFollowedItem);


	Box2DWorld *world() const;
	void setWorld(Box2DWorld *newWorld);

signals:
	void runningChanged();
	void joystickChanged();
	void joystickStateChanged();
	void debugViewChanged();
	void tiledObjectsChanged();
	void followedItemChanged();

	void testPointsChanged();

	void worldChanged();

protected:
	virtual void refresh() override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;

	virtual void loadObjectLayer(Tiled::ObjectGroup *group);
	virtual void loadGround(Tiled::MapObject *object);

	struct DynamicZ {
		QPolygonF polygon;
		bool vertical = true;
		bool horizontal = false;
	};

	std::map<int, DynamicZ> m_dynamicZList;
	std::unique_ptr<TiledQuick::MapLoader> m_mapLoader;
	Box2DWorld *m_world = nullptr;
	QList<TiledObject*> m_tiledObjects;

private:
	struct KeyboardJoystickState {
		qreal dx = 0.5;
		qreal dy = 0.5;
	};

	void joystickConnect(const bool &connect = true);
	Q_INVOKABLE void updateJoystick();
	void updateKeyboardJoystick(const KeyboardJoystickState &state);

	void onSceneStatusChanged(const TiledQuick::MapLoader::Status &status);
	void onWorldStepped();



	QQuickItem *m_joystick = nullptr;
	QQuickItem *m_followedItem = nullptr;
	JoystickState m_joystickState;
	KeyboardJoystickState m_keyboardJoystickState;

	bool m_debugView = false;


	QVariantList m_testPoints;
	Q_PROPERTY(QVariantList testPoints READ testPoints WRITE setTestPoints NOTIFY testPointsChanged FINAL)

	Q_PROPERTY(Box2DWorld *world READ world WRITE setWorld NOTIFY worldChanged FINAL)
};

#endif // TILEDSCENE_H
