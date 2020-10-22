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
 * @brief GamePlayerPrivate::onBodyBeginContact
 * @param other
 */

void GamePlayerPrivate::onBodyBeginContact(Box2DFixture *other)
{
	qDebug() << "BEGIN" << other;
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

	/*QVariant body = parentEntity()->property("body");

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

	for (int i=0; i<f.count(&f); ++i) {
		qDebug() << i << f.at(&f, i);
	}


	Box2DBox *box = new Box2DBox(parentEntity());

	box->setWidth(100);
	box->setHeight(100);
	box->setX(-20);
	box->setY(-20);
	box->setSensor(true);

	f.append(&f, box);

	QQmlListProperty<Box2DFixture> f2 = b2body->fixtures();

	for (int i=0; i<f2.count(&f2); ++i) {
		qDebug() << i << f.at(&f2, i);
	} */
}


