/*
 * ---- Call of Suli ----
 *
 * tiledpaintedlayer.h
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

#ifndef TILEDPAINTEDLAYER_H
#define TILEDPAINTEDLAYER_H

#include <QQuickPaintedItem>

#include "tmxmap.h"


class TiledPaintedLayer : public QQuickPaintedItem
{
	Q_OBJECT

	Q_PROPERTY(Tiled::Map * map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(Tiled::TileLayer * layer READ layer WRITE setLayer NOTIFY layerChanged)

public:
	TiledPaintedLayer(QQuickItem *parent = Q_NULLPTR);

	Tiled::Map * map() const { return m_map; }
	Tiled::TileLayer * layer() const { return m_layer; }

public slots:
	void onParentChanged(QQuickItem *);
	void setMap(Tiled::Map * map);
	void setLayer(Tiled::TileLayer * layer);

protected:
	virtual void paint(QPainter *painter) override;

private slots:
	void onParentSizeChanged();

signals:
	void mapChanged(Tiled::Map * map);
	void layerChanged(Tiled::TileLayer * layer);

private:
	Tiled::Map * m_map;
	Tiled::TileLayer * m_layer;
};

#endif // TILEDPAINTEDLAYER_H
