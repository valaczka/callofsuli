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

#include "cosdb.h"

#include <QPainter>
#include <QPainterPath>

SqlImage::SqlImage(Client *client, const QString &connectionName, const QString &databaseName, const QString &table)
	: QQuickImageProvider(QQuickImageProvider::Pixmap)
	, m_db(new CosDb(connectionName, client))
	, m_table(table)
{
	m_db->setDatabaseName(databaseName);
	m_db->open();
	m_deleteRequest = true;
}

/**
 * @brief SqlImage::SqlImage
 * @param client
 * @param db
 */

SqlImage::SqlImage(CosDb *db, const QString &table)
	: QQuickImageProvider(QQuickImageProvider::Pixmap)
	, m_db(db)
	, m_deleteRequest(false)
	, m_table(table)
{

}


/**
 * @brief SqlImage::~SqlImage
 */

SqlImage::~SqlImage()
{
	if (m_deleteRequest) {
		m_db->close();
		delete m_db;
		m_db = nullptr;
	}
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
	if (!m_db->isValid() || !m_db->isOpen()) {
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

	QString overlayText = path.value(2, "");

	QVariantMap m = m_db->execSelectQueryOneRow("SELECT content FROM "+m_table+" WHERE folder=? AND file=?", l);

	bool success = !m.isEmpty();

	if (success) {
		QByteArray outByteArray = m.value("content").toByteArray();
		QPixmap outPixmap = QPixmap();
		outPixmap.loadFromData(outByteArray);

		if (!outPixmap.isNull()) {
			if (requestedSize.isValid())
				outPixmap = outPixmap.scaled(requestedSize, Qt::KeepAspectRatio);

			if (!overlayText.isEmpty()) {
				QPainter painter(&outPixmap);

				QFont font = QFont("Arial Black");
				int drawSize = qMax(qRound(outPixmap.height() * 0.38), 5);
				font.setPixelSize(drawSize);

				painter.save();

				QPainterPath path;

				path.addText(outPixmap.width()*0.75, outPixmap.height()*0.95, font, overlayText);
				painter.strokePath(path, QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
				painter.fillPath(path, Qt::cyan);

				painter.restore();
			}

			if (size)
				*size = outPixmap.size();
			return outPixmap;
		}
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
	pixmap.fill(QColor("green").rgba());
	return pixmap;
}



