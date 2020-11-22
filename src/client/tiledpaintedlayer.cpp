/*
 * ---- Call of Suli ----
 *
 * tiledpaintedlayer.cpp
 *
 * Created on: 2020. 10. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledPaintedLayer
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

#include <QPainter>

#include "tiledpaintedlayer.h"

/**
 * @brief TiledPaintedLayer::TiledPaintedLayer
 * @param parent
 */

TiledPaintedLayer::TiledPaintedLayer(QQuickItem *parent)
	: QQuickPaintedItem(parent)
	, m_map(nullptr)
	, m_layer(nullptr)
{

	connect(this, &TiledPaintedLayer::parentChanged, this, &TiledPaintedLayer::onParentChanged);

	if (parent)
		onParentChanged(parent);

}


/**
 * @brief TiledPaintedLayer::setLayer
 * @param layer
 */

void TiledPaintedLayer::setLayer(Tiled::TileLayer *layer)
{
	if (m_layer == layer)
		return;

	m_layer = layer;
	emit layerChanged(m_layer);
}



void TiledPaintedLayer::onParentChanged(QQuickItem *)
{
	QQuickItem *p = qobject_cast<QQuickItem *>(this->parent());

	if (p) {
		connect(p, &QQuickItem::widthChanged, this, &TiledPaintedLayer::onParentSizeChanged);
		connect(p, &QQuickItem::heightChanged, this, &TiledPaintedLayer::onParentSizeChanged);

		setWidth(p->width());
		setHeight(p->height());
	}
}

void TiledPaintedLayer::setMap(Tiled::Map *map)
{
	if (m_map == map)
		return;

	m_map = map;
	emit mapChanged(m_map);
}



/**
 * @brief TiledPaintedLayer::paint
 * @param painter
 */

void TiledPaintedLayer::paint(QPainter *painter)
{
	if (!m_map || !m_layer)
		return;


	setOpacity(m_layer->opacity());
	setVisible(m_layer->isVisible());

	int cellX, cellY;
	cellX = cellY = 0;

	TMXTileLayer layer = TMXTileLayer(m_layer);

	foreach(const TMXCell &cell, layer.cells())
	{
		// Store tiles that are used from the tileset
		if(!cell.isEmpty()) {
			TMXTile tile = cell.tile();
			const QPoint &pos = QPoint(cellX * m_map->tileWidth(), cellY * m_map->tileHeight() - tile.height() + m_map->tileHeight());

			QPainter::PixmapFragment fragment;
			fragment.x = pos.x();
			fragment.y = pos.y();
			fragment.sourceLeft = 0;
			fragment.sourceTop = 0;
			fragment.width = tile.width();
			fragment.height = tile.height();
			fragment.scaleX = cell.flippedHorizontally() ? -1 : 1;
			fragment.scaleY = cell.flippedVertically() ? -1 : 1;
			fragment.rotation = 0;
			fragment.opacity = 1;

			if (cell.flippedAntiDiagonally())
				fragment.rotation = 90;

			QTransform transform;
			transform.translate(pos.x() + tile.width() * .5, pos.y() + tile.height() * .5);
			transform.rotate(fragment.rotation);
			transform.scale(fragment.scaleX, fragment.scaleY);

			QRect target = QRect(pos.x(), pos.y(), tile.width(), tile.height());
			QRect source = QRect(tile.image().rect());
			const QPixmap &tileImage = tile.image().transformed(transform);
			painter->drawPixmap(target, tileImage, source);
		}

		cellY++;
		if(((cellY * m_map->tileHeight()) % (m_map->height() * m_map->tileHeight())) == 0) {
			cellY = 0;
			cellX++;
		}
	}

}


/**
 * @brief TiledPaintedLayer::onParentSizeChanged
 */

void TiledPaintedLayer::onParentSizeChanged()
{
	QQuickItem *p = qobject_cast<QQuickItem *>(this->parent());
	if (!p)
		return;

	setWidth(p->width());
	setHeight(p->height());

}
