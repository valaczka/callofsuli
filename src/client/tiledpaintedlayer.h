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

#include "tiledscene.h"


class TiledPaintedLayer : public QQuickPaintedItem
{
	Q_OBJECT

	Q_PROPERTY(TiledScene* scene READ scene WRITE setScene NOTIFY sceneChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
	TiledPaintedLayer(QQuickItem *parent = Q_NULLPTR);

	Q_INVOKABLE void paintTiles();

	TiledScene* scene() const { return m_scene; }
	QString name() const { return m_name; }

public slots:
	void setScene(TiledScene* scene);
	void setName(QString name);

protected:
	virtual void paint(QPainter *painter) override;

signals:
	void sceneChanged(TiledScene* scene);
	void nameChanged(QString name);

private:
	TiledScene* m_scene;
	QString m_name;
};

#endif // TILEDPAINTEDLAYER_H
