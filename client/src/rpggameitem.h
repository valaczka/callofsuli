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


		FixtureAll =
		FixtureGround |
		FixtureExcluded	|
		FixturePlayerBody |
		FixturePlayerTarget |
		FixtureSensor |
		FixtureVirtualCircle

	};

	Q_ENUM(Fixture)
	Q_DECLARE_FLAGS(Fixtures, Fixture)


	RpgGame *game() const;
	void setGame(RpgGame *newGame);

	bool load(const RpgGameDefinition &def);

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers) override;

	bool isContentReady() const;
	void setIsContentReady(bool newIsContentReady);

protected:
	virtual void sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene) override;
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
	RpgGamePrivate *d = nullptr;

	RpgGame *m_game = nullptr;
	bool m_isContentReady = false;

	friend class RpgGame;
	friend class RpgGamePrivate;
};

#endif // RPGGAMEITEM_H
