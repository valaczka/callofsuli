/*
 * ---- Call of Suli ----
 *
 * onlineclient.h
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OnlineClient
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

#ifndef ONLINECLIENT_H
#define ONLINECLIENT_H

#include "client.h"
#include <QObject>

class OnlineClient : public Client
{
	Q_OBJECT

public:
	explicit OnlineClient(Application *app, QObject *parent = nullptr);
	virtual ~OnlineClient();

	void wasmLoadFileToFileSystem(const QString &accept, std::function<void (const QString &, const QByteArray &)> saveFunc);
	void wasmSaveContent(const QByteArray &data, const QString &fileNameHint);

	/*void toggleFullscreen(); */
	void enableTabCloseConfirmation(bool enable);

protected slots:
	virtual void onApplicationStarted() override;

private slots:
	void onResourceDownloaded();
	void onAllResourceReady();

signals:
	void allResourceReady();

private:
	QStringList m_resourceList;

};

#endif // ONLINECLIENT_H
