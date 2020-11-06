/*
 * ---- Call of Suli ----
 *
 * cosdb.h
 *
 * Created on: 2020. 03. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * COSdb
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

#ifndef COSDB_H
#define COSDB_H

#include <QObject>
#include "cossql.h"

class COSdb : public CosSql
{
	Q_OBJECT

public:
	Q_PROPERTY(QString databaseFile READ databaseFile WRITE setDatabaseFile NOTIFY databaseFileChanged)

	explicit COSdb(const QString &connectionName = QString(), QObject *parent = nullptr);
	virtual ~COSdb();

	QString databaseFile() const { return m_databaseFile; }
	bool databaseExists() const { return (QFile::exists(m_databaseFile)); }

public slots:
	bool databaseOpen();
	void setDatabaseFile(QString databaseFile);

protected slots:
	virtual bool databaseInit() = 0;

signals:
	void databaseError(const QString &text);
	void databaseFileChanged(QString databaseFile);

protected:
	QString m_databaseFile;
	bool m_isOwnCreated;

};

#endif // COSDB_H
