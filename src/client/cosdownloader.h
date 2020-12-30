/*
 * ---- Call of Suli ----
 *
 * cosdownloader.h
 *
 * Created on: 2020. 12. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosDownloader
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

#ifndef COSDOWNLOADER_H
#define COSDOWNLOADER_H

#include <QObject>

#include "../../common/cosmessage.h"

struct CosDownloaderItem;

class AbstractActivity;

class CosDownloader : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<CosDownloaderItem> list READ list NOTIFY listChanged)
	Q_PROPERTY(int fullSize READ fullSize NOTIFY fullSizeChanged)
	Q_PROPERTY(int downloadedSize READ downloadedSize NOTIFY downloadedSizeChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	Q_PROPERTY(QString activeDownload READ activeDownload WRITE setActiveDownload NOTIFY activeDownloadChanged)
	Q_PROPERTY(bool hasDownloadable READ hasDownloadable NOTIFY hasDownloadableChanged)
	Q_PROPERTY(qreal downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

public:
	explicit CosDownloader(AbstractActivity *activity = nullptr, const CosMessage::CosClass &cosClass = CosMessage::ClassInvalid,
						   const QString &func = QString(), QObject *parent = nullptr);
	~CosDownloader();

	void append(const CosDownloaderItem &item);
	void append(const QString &remote, const QString &local = QString(), const int &size = 0, const QString &md5 = QString(),
				const bool &downloaded = false, const qreal &progress = 0.0);
	void append(const QVariantMap &map);
	void append(const QVariantList &list);
	void clear();

	int get(const QString &remote);

	QList<CosDownloaderItem> list() const { return m_list; }
	int fullSize() const;
	int downloadedSize() const;
	int count() const { return m_list.count(); }
	QString activeDownload() const { return m_activeDownload; }
	bool hasDownloadable() const;
	qreal downloadProgress() const;

	CosMessage::CosClass cosClass() const { return m_class; }
	void setCosClass(const CosMessage::CosClass &cosClass) { m_class = cosClass; }

	QString func() const { return m_func; }
	void setFunc(const QString &func) { m_func = func; }

	QString jsonKeyFileName() const { return m_jsonKeyFileName; }
	void setJsonKeyFileName(const QString &jsonKeyFileName) { m_jsonKeyFileName = jsonKeyFileName; }

	QString jsonKeyMd5() const { return m_jsonKeyMd5; }
	void setJsonKeyMd5(const QString &jsonKeyMd5) { m_jsonKeyMd5 = jsonKeyMd5; }

	bool running() const { return m_running; }

public slots:
	void start();
	void abort();
	void setActiveDownload(QString activeDownload);

signals:
	void downloadStarted();
	void downloadFinished();
	void downloadFailed();
	void oneDownloadFinished(CosDownloaderItem item, const QByteArray &data, const QJsonObject &jsonData);

	void fullSizeChanged(qreal fullSize);
	void downloadedSizeChanged(qreal downloadedSize);
	void countChanged(int count);
	void activeDownloadChanged(QString activeDownload);
	void hasDownloadableChanged(bool hasDownloadable);
	void listChanged(QList<CosDownloaderItem> list);
	void downloadProgressChanged(qreal downloadProgress);

protected slots:
	void onMessageFrameReceived(const CosMessage &message);
	void onMessageReceived(const CosMessage &message);

private:
	void downloadNext();

	AbstractActivity *m_activity;
	QList<CosDownloaderItem> m_list;
	QString m_activeDownload;
	CosMessage::CosClass m_class;
	QString m_func;
	QString m_jsonKeyFileName;
	QString m_jsonKeyMd5;
	bool m_running;
};


/**
 * @brief The CosDownloaderItem struct
 */

struct CosDownloaderItem
{
	Q_GADGET

	Q_PROPERTY(QString remoteFile MEMBER remoteFile)
	Q_PROPERTY(QString localFile MEMBER localFile)
	Q_PROPERTY(bool isDownloaded MEMBER isDownloaded)
	Q_PROPERTY(qreal progress MEMBER progress)
	Q_PROPERTY(int size MEMBER size)
	Q_PROPERTY(QString md5 MEMBER md5)

public:
	CosDownloaderItem(const QString &remote, const QString &local = QString(), const int &ssize = 0, const QString &mmd5 = QString(),
					  const bool &downloaded = false, const qreal &prg = 0.0)
		: remoteFile(remote)
		, localFile(local)
		, isDownloaded(downloaded)
		, progress(prg)
		, size(ssize)
		, md5(mmd5)
	{}

	friend inline bool operator== (const CosDownloaderItem &b1, const CosDownloaderItem &b2) {
		return b1.remoteFile == b2.remoteFile &&
				b1.localFile == b2.localFile &&
				b1.isDownloaded == b2.isDownloaded &&
				b1.progress == b2.progress &&
				b1.size == b2.size &&
				b1.md5 == b2.md5;
	}

	QString remoteFile;
	QString localFile;
	bool isDownloaded;
	qreal progress;
	int size;
	QString md5;
};

#endif // COSDOWNLOADER_H
