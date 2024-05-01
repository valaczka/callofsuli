/*
 * ---- Call of Suli ----
 *
 * tiledcontainer.cpp
 *
 * Created on: 2024. 04. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledContainer
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

#include "tiledcontainer.h"

TiledContainer::TiledContainer(QObject *parent)
	: QObject{parent}
{

}

TiledContainer::ContainerType TiledContainer::type() const
{
	return m_type;
}

void TiledContainer::setType(const ContainerType &newType)
{
	if (m_type == newType)
		return;
	m_type = newType;
	emit typeChanged();
}

bool TiledContainer::isActive() const
{
	return m_isActive;
}

void TiledContainer::setIsActive(bool newIsActive)
{
	if (m_isActive == newIsActive)
		return;
	m_isActive = newIsActive;
	emit isActiveChanged();

	if (m_isActive)
		onActivated();
	else
		onDeactivated();
}

TiledScene *TiledContainer::scene() const
{
	return m_scene;
}

void TiledContainer::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();
}
