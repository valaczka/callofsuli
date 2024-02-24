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
#include <QtGui/private/qwasmlocalfileaccess_p.h>
#include <QObject>

class OnlineClient : public Client
{
	Q_OBJECT

public:
	explicit OnlineClient(Application *app);
	virtual ~OnlineClient();

	void wasmSaveContent(const QByteArray &data, const QString &fileNameHint);
	void wasmLoadFileToFileSystem(const QString &accept, std::function<void (const QString &, const QByteArray &)> saveFunc);

	template <typename T>
	void wasmLoadFileToFileSystem(const QString &accept, T *inst, void (T::*func)(const QString &name, const QByteArray &content));

	void enableTabCloseConfirmation(bool enable);

	virtual bool fullScreenHelper() const override;
	virtual void setFullScreenHelper(bool newFullScreenHelper) override;

protected:
	virtual void fullScreenHelperConnect(QQuickWindow *window) override;
	virtual void fullScreenHelperDisconnect(QQuickWindow *window) override;
	virtual bool saveDynamicResource(const QString &name, const QByteArray &data) override;

protected slots:
	virtual void onApplicationStarted() override;
	virtual void onUserLoggedIn() override;
	virtual void onUserLoggedOut() override;

private slots:
	void onResourceDownloaded();
	void onAllResourceReady();

signals:
	void allResourceReady();

private:
	QStringList m_resourceList;
	bool m_demoMode = false;
};



/**
 * @brief OnlineClient::wasmLoadFileToFileSystem
 * @param accept
 * @param inst
 * @return
 */

template<typename T>
void OnlineClient::wasmLoadFileToFileSystem(const QString &accept, T *inst, void (T::*func)(const QString &, const QByteArray &))
{
	struct LoadFileData {
		QString name;
		QByteArray buffer;
	};

	LoadFileData *fileData = new LoadFileData();

	QWasmLocalFileAccess::openFile(accept.toStdString(),
								   [](bool fileSelected) {
		LOG_CDEBUG("client") << "File selected" << fileSelected;
	},
	[fileData](uint64_t size, const std::string name) -> char* {
		fileData->name = QString::fromStdString(name);
		fileData->buffer.resize(size);
		return fileData->buffer.data();
	},
	[fileData, inst, func](){
		QByteArray content = fileData->buffer;
		QString name = fileData->name;
		if (inst && func)
			std::invoke(func, inst, name, content);
		delete fileData;
	});
}



#endif // ONLINECLIENT_H
