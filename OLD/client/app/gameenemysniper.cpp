/*
 * ---- Call of Suli ----
 *
 * gameenemysniper.cpp
 *
 * Created on: 2022. 07. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySniper
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

#include "gameenemysniper.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "scene.h"

/**
 * @brief GameEnemySniper::GameEnemySniper
 */

GameEnemySniper::GameEnemySniper(QQuickItem *parent)
	: GameEnemy(parent)
	, m_msecBeforeTurn(5000)
	, m_shotSoundFile("qrc:/sound/sfx/rifle.wav")
	, m_sniperType("sniper1")
{
	connect(this, &GameEnemy::cosGameChanged, this, &GameEnemySniper::onCosGameChanged);
	/*connect(this, &GameEnemy::movingChanged, this, [=]() {
		if (m_moving)
			m_movingTimer->start();
		else
			m_movingTimer->stop();
	});*/

	connect(this, &GameEntity::qrcDataChanged, this, &GameEnemySniper::onQrcDataChanged);

}

/**
 * @brief GameEnemySniper::~GameEnemySniper
 */

GameEnemySniper::~GameEnemySniper()
{

}



/**
 * @brief GameEnemySniper::setQrcDir
 */

void GameEnemySniper::setQrcDir()
{
	CosGame *game = cosGame();

	if (!game)
		return;


	QString dir = "qrc:/snipers/"+m_sniperType;

	setQrcDirName(dir);
}



/**
 * @brief GameEnemySniper::createFixtures
 */

void GameEnemySniper::createFixtures()
{
	QVariant body = parentEntity()->property("body");

	if (!body.isValid()) {
		qWarning() << "Invalid property: body" << parentEntity();
		return;
	}


	Box2DBody *b2body = qvariant_cast<Box2DBody*>(body);

	if (!b2body) {
		qWarning() << "Invalid variant cast" << body;
		return;
	}

	QQmlListProperty<Box2DFixture> f = b2body->fixtures();


	// Create bound box

	Box2DBox *box = new Box2DBox(parentEntity());

	box->setWidth(10);
	box->setHeight(10);
	box->setX(0);
	box->setY(0);
	box->setSensor(false);


	box->setRestitution(0);
	box->setFriction(0);
	box->setCategories(Box2DFixture::Category5);
	box->setCollidesWith(Box2DFixture::Category1);

	f.append(&f, box);

	setBoundBox(box);
}





int GameEnemySniper::msecBeforeTurn() const
{
	return m_msecBeforeTurn;
}

void GameEnemySniper::setMsecBeforeTurn(int newMsecBeforeTurn)
{
	if (m_msecBeforeTurn == newMsecBeforeTurn)
		return;
	m_msecBeforeTurn = newMsecBeforeTurn;
	emit msecBeforeTurnChanged();
}

const QString &GameEnemySniper::shotSoundFile() const
{
	return m_shotSoundFile;
}

void GameEnemySniper::setShotSoundFile(const QString &newShotSoundFile)
{
	if (m_shotSoundFile == newShotSoundFile)
		return;
	m_shotSoundFile = newShotSoundFile;
	emit shotSoundFileChanged();
}

const QString &GameEnemySniper::sniperType() const
{
	return m_sniperType;
}

void GameEnemySniper::setSniperType(const QString &newSniperType)
{
	if (m_sniperType == newSniperType)
		return;
	m_sniperType = newSniperType;
	emit sniperTypeChanged();
}



/**
 * @brief GameEnemySniper::onGameDataReady
 * @param map
 */

void GameEnemySniper::onGameDataReady(const QVariantMap &map)
{
	if (!map.contains("sniper"))
		return;

	QVariantMap m = map.value("sniper").toMap();

	setRayCastElevation(m.value("rayCastElevation", m_rayCastElevation).toReal());
	setRayCastLength(m.value("rayCastLength", m_rayCastLength).toReal());
	setMsecBeforeTurn(m.value("msecBeforeTurn", m_msecBeforeTurn).toInt());
	setCastAttackFraction(m.value("castAttackFraction", m_castAttackFraction).toReal());
	setMsecBeforeAttack(m.value("msecBeforeAttack", m_msecBeforeAttack).toInt());
	setMsecBetweenAttack(m.value("msecBetweenAttack", m_msecBetweenAttack).toInt());
}


/**
 * @brief GameEnemySniper::onCosGameChanged
 */


void GameEnemySniper::onCosGameChanged(CosGame *)
{
	if (!cosGame())
		return;

	setQrcDir();
	loadQrcData();
}



/**
 * @brief GameEnemySniper::onQrcDataChanged
 */

void GameEnemySniper::onQrcDataChanged(QVariantMap)
{
	QString f = m_qrcData.value("shotSound").toString();

	if (f.isEmpty())
		return;

	QString shotFile = m_qrcDirName+"/"+f;
	QString realFile = shotFile;
	if (realFile.startsWith("qrc:/"))
		realFile.replace("qrc:/", ":/");

	if (QFile::exists(realFile)) {
		setShotSoundFile(shotFile);
	}
}
