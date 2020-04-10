/*
 * ---- Call of Suli ----
 *
 * sqlimage.cpp
 *
 * Created on: 2020. 04. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * SqlImage
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

#include "sqlimage.h"

#include "../common/cossql.h"

SqlImage::SqlImage(Client *client)
	: QQuickImageProvider(QQuickImageProvider::Pixmap)
	, m_client(client)
{

}



/**
 * @brief SqlImage::requestPixmap
 * @param id
 * @param size
 * @param requestedSize
 * @return
 */

QPixmap SqlImage::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
	CosSql *db = new CosSql("sqlimageprovider");

	if (m_client)
		db->open(m_client->serverDataDir()+"/resources.db", false);


	if (!db->isValid()) {
		int width = 100;
		int height = 50;

		if (size)
			*size = QSize(width, height);
		QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
					   requestedSize.height() > 0 ? requestedSize.height() : height);
		pixmap.fill(QColor("red").rgba());
		return pixmap;
	}

	QStringList path = id.split("/");

	QVariantList l;
	l << path.value(0, "");
	l << path.value(1, "");

	QVariantMap m;

	bool success = db->execSelectQueryOneRow("SELECT content FROM resource WHERE folder=? AND file=?", l, &m);

	db->close();
	delete db;

	if (success) {
		QByteArray outByteArray = m.value("content").toByteArray();
		QPixmap outPixmap = QPixmap();
		outPixmap.loadFromData(outByteArray);
		if (!outPixmap.isNull())
			return outPixmap;
	} else {
		int width = 100;
		int height = 50;

		if (size)
			*size = QSize(width, height);
		QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
					   requestedSize.height() > 0 ? requestedSize.height() : height);
		pixmap.fill(QColor("orange").rgba());
		return pixmap;
	}

	int width = 100;
	int height = 50;
	QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
				   requestedSize.height() > 0 ? requestedSize.height() : height);
	pixmap.fill(QColor("yellow").rgba());
	return pixmap;
}
