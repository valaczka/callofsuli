/*
 * ---- Call of Suli ----
 *
 * tiledvisualitem.cpp
 *
 * Created on: 2024. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledVisualItem
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

#include "tiledvisualitem.h"
#include "tiledscene.h"

TiledVisualItem::TiledVisualItem(QQuickItem *parent)
	: QQuickItem(parent)
{

}

QUrl TiledVisualItem::source() const
{
	return m_source;
}

void TiledVisualItem::setSource(const QUrl &newSource)
{
	if (m_source == newSource)
		return;
	m_source = newSource;
	emit sourceChanged();
}

bool TiledVisualItem::glowEnabled() const
{
	return m_glowEnabled;
}

void TiledVisualItem::setGlowEnabled(bool newGlowEnabled)
{
	if (m_glowEnabled == newGlowEnabled)
		return;
	m_glowEnabled = newGlowEnabled;
	emit glowEnabledChanged();
}

QColor TiledVisualItem::glowColor() const
{
	return m_glowColor;
}

void TiledVisualItem::setGlowColor(const QColor &newGlowColor)
{
	if (m_glowColor == newGlowColor)
		return;
	m_glowColor = newGlowColor;
	emit glowColorChanged();
}

bool TiledVisualItem::overlayEnabled() const
{
	return m_overlayEnabled;
}

void TiledVisualItem::setOverlayEnabled(bool newOverlayEnabled)
{
	if (m_overlayEnabled == newOverlayEnabled)
		return;
	m_overlayEnabled = newOverlayEnabled;
	emit overlayEnabledChanged();
}

QColor TiledVisualItem::overlayColor() const
{
	return m_overlayColor;
}

void TiledVisualItem::setOverlayColor(const QColor &newOverlayColor)
{
	if (m_overlayColor == newOverlayColor)
		return;
	m_overlayColor = newOverlayColor;
	emit overlayColorChanged();
}

TiledScene *TiledVisualItem::scene() const
{
	return m_scene;
}

void TiledVisualItem::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();
}

QString TiledVisualItem::name() const
{
	return m_name;
}

void TiledVisualItem::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}



TiledQuick::TileLayerItem *TiledVisualItem::layerItem() const
{
	return m_layerItem;
}

void TiledVisualItem::setLayerItem(TiledQuick::TileLayerItem *newLayerItem)
{
	if (m_layerItem)
		disconnect(m_layerItem);

	m_layerItem = newLayerItem;

	if (m_layerItem) {
		connect(this, &QQuickItem::zChanged, m_layerItem, [this]() {
			m_layerItem->setZ(this->z());
		});
		connect(this, &QQuickItem::visibleChanged, m_layerItem, [this](){
			m_layerItem->setVisible(this->isVisible());
		});
	}
}

QString TiledVisualItem::displayName() const
{
	return m_displayName;
}

void TiledVisualItem::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}
