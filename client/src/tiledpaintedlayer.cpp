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
#include <QSGTexture>
#include <QSGSimpleMaterialShader>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>

#include "Logger.h"
#include "libtiled/tilelayer.h"
#include "tiledpaintedlayer.h"
#include "gamescene.h"

/**
 * @brief TiledPaintedLayer::TiledPaintedLayer
 * @param parent
 */

TiledPaintedLayer::TiledPaintedLayer(QQuickItem *parent, Tiled::Map *map, Tiled::TileLayer *layer)
	: QQuickPaintedItem(parent)
	, m_map(map)
	, m_layer(layer)
{
	if (!m_map) {
		LOG_CWARNING("scene") << "Missing map on TiledPaintedLayer";
	}

	if (!m_layer) {
		LOG_CWARNING("scene") << "Missing layer on TiledPaintedLayer";
	}


	if (m_map && m_layer) {
		setImplicitWidth(m_layer->width() * m_map->tileSize().width());
		setImplicitHeight(m_layer->height() * m_map->tileSize().height());
	}
}

/**
 * @brief TiledPaintedLayer::~TiledPaintedLayer
 */

TiledPaintedLayer::~TiledPaintedLayer()
{

}



/**
 * @brief TiledPaintedLayer::paint
 * @param painter
 */

void TiledPaintedLayer::paint(QPainter *painter)
{
	if (!m_map) {
		LOG_CWARNING("scene") << "Missing map on TiledPaintedLayer";
		return;
	}

	if (!m_layer) {
		LOG_CWARNING("scene") << "Missing layer on TiledPaintedLayer";
		return;
	}

	int cellX, cellY;
	cellX = cellY = 0;

	for (int x = 0; x < m_layer->width(); ++x) {
		for (int y = 0; y < m_layer->height(); ++y) {
			const Tiled::Cell &cell = m_layer->cellAt(x, y);

			if (!cell.isEmpty()) {
				Tiled::Tile *tile = cell.tile();
				const QPoint &pos = QPoint(cellX * m_map->tileWidth(), cellY * m_map->tileHeight() - tile->height() + m_map->tileHeight());

				QPainter::PixmapFragment fragment;
				fragment.x = pos.x();
				fragment.y = pos.y();
				fragment.sourceLeft = 0;
				fragment.sourceTop = 0;
				fragment.width = tile->width();
				fragment.height = tile->height();
				fragment.scaleX = cell.flippedHorizontally() ? -1 : 1;
				fragment.scaleY = cell.flippedVertically() ? -1 : 1;
				fragment.rotation = 0;
				fragment.opacity = 1;

				if (cell.flippedAntiDiagonally())
					fragment.rotation = 90;

				QTransform transform;
				transform.translate(pos.x() + tile->width() * .5, pos.y() + tile->height() * .5);
				transform.rotate(fragment.rotation);
				transform.scale(fragment.scaleX, fragment.scaleY);

				QRect target = QRect(pos.x(), pos.y(), tile->width(), tile->height());
				const QRect &source = tile->imageRect();

				const QPixmap &tileImage = tile->image().transformed(transform);
				painter->drawPixmap(target, tileImage, source);


			}

			cellY++;
			if(((cellY * m_map->tileHeight()) % (m_map->height() * m_map->tileHeight())) == 0) {
				cellY = 0;
				cellX++;
			}
		}
	}
}



