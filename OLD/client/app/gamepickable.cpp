/*
 * ---- Call of Suli ----
 *
 * gamepickable.cpp
 *
 * Created on: 2021. 01. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePickable
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

#include "gamepickable.h"
#include "cosgame.h"

GamePickable::GamePickable(QObject *parent)
	: QObject(parent)
	, m_type(PickableInvalid)
	, m_game(nullptr)
	, m_data()
{

}

GamePickable::~GamePickable()
{

}


/**
 * @brief GamePickable::pick
 */

void GamePickable::pick()
{
	emit picked();

	if (m_game) {
		switch (m_type) {
			case PickableShield:
				emit m_game->gameToolTipRequest("notification/pickableShield",
												"qrc:/internal/icon/shield.svg",
												tr("A pajzs megvéd attól, hogy XP-t veszíts"),
												tr("Ha az ellenség meglő, akkor amíg van pajzsod, abból veszítesz XP helyett"));
				break;
			case PickableWater:
				emit m_game->gameToolTipRequest("notification/pickableWater",
												"qrc:/internal/game/drop.png",
												tr("A vízzel elolthatod a tüzet"),
												tr("Közelítsd meg óvatosan a tüzet, majd nyomd meg a vizet ábrázoló gombot"));
				break;
			case PickablePliers:
				emit m_game->gameToolTipRequest("notification/pickablePliers",
												"qrc:/internal/icon/pliers.svg",
												tr("A drótvágóval átvághatod a kerítést"),
												tr("Közelítsd meg a kerítést, majd nyomd meg a drótvágót ábrázoló gombot"));
				break;
			case PickableCamouflage:
				emit m_game->gameToolTipRequest("notification/pickableCamouflage",
												"qrc:/internal/icon/domino-mask.svg",
												tr("Az álruhával láthatatlanná válhatsz"),
												tr("Egy álruha használatával 10 másodpercen keresztül nem vesz észre az ellenség"));
				break;
			case PickableTeleporter:
				emit m_game->gameToolTipRequest("notification/pickableTeleleporter",
												"qrc:/internal/icon/remote.svg",
												tr("A teleportálóval a teleportok között közlekedhetsz"),
												"");
				break;
			default:
				break;
		}
	}
}



void GamePickable::setType(PickableType type)
{
	if (m_type == type)
		return;

	m_type = type;
	emit typeChanged(m_type);
}

void GamePickable::setGame(CosGame *game)
{
	if (m_game == game)
		return;

	m_game = game;
	emit gameChanged(m_game);
}

void GamePickable::setData(QVariantMap data)
{
	if (m_data == data)
		return;

	m_data = data;
	emit dataChanged(m_data);
}
