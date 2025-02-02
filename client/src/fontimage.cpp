/*
 * ---- Call of Suli ----
 *
 * fontimage.cpp
 *
 * Created on: 2020. 05. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * FontImage
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

#include <QFont>
#include <QPainter>

#include "fontimage.h"

FontImage::FontImage()
	: QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}


/**
 * @brief FontImage::requestPixmap
 * @param id
 * @param size
 * @param requestedSize
 * @return
 */

QPixmap FontImage::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
	QStringList path = id.split(QStringLiteral("/"));

	QString family = path.value(0, QStringLiteral(""));
	QString letter = path.value(1, QStringLiteral(""));

	QSize mSize = QSize(requestedSize.width() > 0 ? requestedSize.width() : 24,
						requestedSize.height() > 0 ? requestedSize.height() : 24);

	if (size)
		*size = mSize;

	QPixmap pix(mSize);

	pix.fill(Qt::transparent);

	QPainter painter(&pix);

	QFont font = QFont(family);
	int drawSize = std::max(qRound(requestedSize.height() * 0.9), 1);
	font.setPixelSize(drawSize);

	QColor penColor(Qt::black);


	painter.save();
	painter.setPen(QPen(penColor));
	painter.setFont(font);
	painter.drawText(QRect(QPoint(0,0), mSize), Qt::AlignCenter|Qt::AlignVCenter, letter);
	painter.restore();

	return pix;
}
