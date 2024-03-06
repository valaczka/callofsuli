/*
 * ---- Call of Suli ----
 *
 * tiledgame.cpp
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledGame
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

#include "tiledgame.h"
#include "Logger.h"
#include "application.h"
#include "isometricplayer.h"

TiledGame::TiledGame(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CTRACE("scene") << "TiledGame created" << this;
}


/**
 * @brief TiledGame::~TiledGame
 */

TiledGame::~TiledGame()
{
	LOG_CTRACE("scene") << "TiledGame destroyed" << this;
}


/**
 * @brief TiledGame::load
 * @return
 */

bool TiledGame::load()
{
	for (const QString &s : QStringList{"qrc:/teszt.tmx", "qrc:/teszt2.tmx"}) {
		if (loadScene(s))
			LOG_CINFO("scene") << "Scene loaded:" << s;
		else
			return false;
	}

	const Scene &item = m_sceneList.first();
	item.scene->setJoystick(m_joystick);
	item.scene->setActive(true);
	item.scene->setDebugView(true);
	item.container->setVisible(true);

	setCurrentScene(item.scene);
	item.scene->forceActiveFocus();

	return true;
}



/**
 * @brief TiledGame::loadPlayer
 * @param scene
 * @param pos
 */

void TiledGame::loadPlayer(TiledScene *scene, const QPointF &pos)
{
	m_player.reset(IsometricPlayer::createPlayer());

	Q_ASSERT(m_player);

	m_player->setScene(scene);
	m_player->emplace(pos);
	m_player->setCurrentDirection(TiledObject::South);

	scene->appendToObjects(m_player.get());
	scene->setFollowedItem(m_player.get());
	scene->setControlledItem(m_player.get());
}




/**
 * @brief TiledGame::joystick
 * @return
 */

QQuickItem *TiledGame::joystick() const
{
	return m_joystick;
}

void TiledGame::setJoystick(QQuickItem *newJoystick)
{
	if (m_joystick == newJoystick)
		return;
	m_joystick = newJoystick;
	emit joystickChanged();

	if (m_joystick && !m_sceneList.isEmpty())
		m_sceneList.first().scene->setJoystick(m_joystick);

}

bool TiledGame::debugView() const
{
	return m_debugView;
}

void TiledGame::setDebugView(bool newDebugView)
{
		if (m_debugView == newDebugView)
		return;
	m_debugView = newDebugView;
	emit debugViewChanged();
}

TiledScene *TiledGame::currentScene() const
{
	return m_currentScene;
}

void TiledGame::setCurrentScene(TiledScene *newCurrentScene)
{
	if (m_currentScene == newCurrentScene)
		return;
	m_currentScene = newCurrentScene;
	emit currentSceneChanged();
}



/**
 * @brief TiledGame::loadScene
 * @param file
 * @return
 */

bool TiledGame::loadScene(const QString &file)
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledFlickableScene.qml"), this);

	LOG_CDEBUG("scene") << "Create scene flickable item:" << component.isReady();

	Scene item;

	item.container = qobject_cast<QQuickItem*>(component.create());

	if (!item.container) {
		LOG_CERROR("scene") << "Scene create error" << component.errorString();
		return false;
	}

	item.scene = qvariant_cast<TiledScene*>(item.container->property("scene"));
	Q_ASSERT(item.scene);

	item.scene->setGame(this);
	item.container->setParentItem(this);

	if (!item.scene->load(QUrl(file))) {
		LOG_CERROR("scene") << "Scene load error" << file;
		item.container->deleteLater();
		return false;
	}

	m_sceneList.append(item);

	return true;
}
