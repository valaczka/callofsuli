/*
 * ---- Call of Suli ----
 *
 * gameplayerprivate.cpp
 *
 * Created on: 2020. 10. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPrivate
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QDebug>
#include <QDir>

#include "entity.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "cosclient.h"
#include "gameplayerprivate.h"

GamePlayerPrivate::GamePlayerPrivate(QQuickItem *parent)
	: GameEntityPrivate(parent)
{
	connect(this, &GameEntityPrivate::cosGameChanged, this, &GamePlayerPrivate::onCosGameChanged);
}





/**
 * @brief GamePlayerPrivate::loadQrcData
 */

void GamePlayerPrivate::setQrcDir()
{
	CosGame *game = cosGame();

	if (!game)
		return;

	QString playerCharacter = game->playerCharacter();

	if (playerCharacter.isEmpty())
		return;

	QString dir = "qrc:/character/"+playerCharacter;

	setQrcDirName(dir);
}


/**
 * @brief GamePlayerPrivate::createFixtures
 */

void GamePlayerPrivate::createFixtures()
{
	qDebug() << "Game player private: create fixtures()";

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
	box->setCategories(Box2DFixture::Category1);
	box->setCollidesWith(Box2DFixture::Category1);

	f.append(&f, box);

	setBoundBox(box);




	// Create body polygon

	Box2DPolygon *polygon = new Box2DPolygon(parentEntity());

	polygon->setSensor(true);
	polygon->setCategories(Box2DFixture::Category2);
	polygon->setCollidesWith(Box2DFixture::Category2|Box2DFixture::Category3);

	QVariantList l;
	l << QVariant(QPoint(0,0))
	  << QVariant(QPoint(10,0))
	  << QVariant(QPoint(10,10))
	  << QVariant(QPoint(0,10));

	polygon->setVertices(l);

	f.append(&f, polygon);

	setBodyPolygon(polygon);


	QQmlListProperty<Box2DFixture> f2 = b2body->fixtures();

	for (int i=0; i<f2.count(&f2); ++i) {
		qDebug() << i << f.at(&f2, i);
	}
}




/**
 * @brief GamePlayerPrivate::onCosGameChanged
 * @param game
 */

void GamePlayerPrivate::onCosGameChanged(CosGame *)
{
	if (!cosGame())
		return;

	setQrcDir();
	loadQrcData();


}


