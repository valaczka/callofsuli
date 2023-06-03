/*
 * ---- Call of Suli ----
 *
 * onlineclient.cpp
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

#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <QtGui/private/qwasmlocalfileaccess_p.h>


#include "Logger.h"
#include "abstractlevelgame.h"
#include "onlineclient.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include "QResource"
#include "application.h"
#include "gameterrain.h"
#include "websocket.h"

OnlineClient::OnlineClient(Application *app, QObject *parent)
	: Client{app, parent}
{
	connect(this, &OnlineClient::allResourceReady, this, &OnlineClient::onAllResourceReady);
}


/**
 * @brief OnlineClient::~OnlineClient
 */

OnlineClient::~OnlineClient()
{

}



/**
 * @brief OnlineClient::wasmLoadFileToFileSystem
 * @param accept
 * @param saveFunc
 * @return
 */

void OnlineClient::wasmLoadFileToFileSystem(const QString &accept, std::function<void (const QString &, const QByteArray &)> saveFunc)
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
	[fileData, saveFunc](){
		QByteArray content = fileData->buffer;
		QString name = fileData->name;
		if (saveFunc)
			saveFunc(name, content);
		delete fileData;
	});
}



/**
 * @brief OnlineClient::wasmSaveContent
 * @param data
 * @param fileNameHint
 */

void OnlineClient::wasmSaveContent(const QByteArray &data, const QString &fileNameHint)
{
	QWasmLocalFileAccess::saveFile(data.constData(), size_t(data.size()), fileNameHint.toStdString());
}





/**
 * @brief OnlineClient::onApplicationStarted
 */

void OnlineClient::onApplicationStarted()
{
	LOG_CDEBUG("client") << "Download resources";

	QNetworkRequest request(QUrl(QStringLiteral("wasm_resources.json")));
	QNetworkReply *reply = m_webSocket->networkManager()->get(request);
	connect(reply, &QNetworkReply::finished, this, &OnlineClient::onResourceDownloaded);

	enableTabCloseConfirmation(true);

	emscripten::val location = emscripten::val::global("location");
	m_parseUrl = {QString::fromStdString(location["href"].as<std::string>())};
}





/**
 * @brief OnlineClient::onResourceDownloaded
 */

void OnlineClient::onResourceDownloaded()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	const QUrl &url = reply->url();

	if (reply->error()) {
		LOG_CWARNING("client") << "Nem sikerült a letöltés:" << url << qPrintable(reply->errorString());
	} else {
		LOG_CDEBUG("client") << "Resource downloaded:" << url;
		const QByteArray &payload = reply->readAll();
		const QString &filename = url.path();

		if (filename == "wasm_resources.json") {
			const QJsonObject &o = Utils::byteArrayToJsonObject(payload);
			const QJsonArray &resources = o.value(QStringLiteral("resources")).toArray();

			QUrl url;
			url.setScheme(m_parseUrl.scheme());
			url.setHost(m_parseUrl.host());
			url.setPort(m_parseUrl.port());

			Server *s = new Server(this);
			m_webSocket->setServer(s);
			s->setUrl(url);


			if (resources.isEmpty()) {
				LOG_CERROR("client") << "Az erőforráslista üres";
			} else {
				m_resourceList.clear();

				foreach (const QJsonValue &v, resources)
					m_resourceList.append(v.toString());

				LOG_CDEBUG("client") << "Letöltendő erőforrások:" << m_resourceList;

				foreach (const QString &s, m_resourceList) {
					QNetworkReply *r = m_webSocket->networkManager()->get(QNetworkRequest(QUrl(s)));
					connect(r, &QNetworkReply::finished, this, &OnlineClient::onResourceDownloaded);
				}
			}
		} else {
			if (!m_resourceList.contains(filename) || payload.isEmpty())	{
				LOG_CWARNING("client") << "Érvénytelen erőforrás:" << qPrintable(filename);
			} else {
				const std::size_t count = payload.size();
				unsigned char* resourceData = new unsigned char[count];
				std::memcpy(resourceData, payload.constData(), count);

				QResource::registerResource(resourceData);

				LOG_CINFO("client") << "Register resource:" << qPrintable(filename);

				m_resourceList.removeAll(filename);

				if (m_resourceList.isEmpty())
					emit allResourceReady();
			}
		}
	}

	reply->deleteLater();
}



/**
 * @brief OnlineClient::onAllResourceReady
 */

void OnlineClient::onAllResourceReady()
{
	m_application->loadFonts();

	LOG_CINFO("client") << "Az alkalmazás sikeresen elindult";

	GameTerrain::reloadAvailableTerrains();
	AbstractLevelGame::reloadAvailableMedal();

	m_startPage = stackPushPage("PageStart.qml");
}


/**
 * @brief OnlineClient::enableTabCloseConfirmation
 * @param enable
 */

void OnlineClient::enableTabCloseConfirmation(bool enable)
{
	LOG_CDEBUG("client") << "Enable tab close confirmation" << enable;

	using emscripten::val;
	const val window = val::global("window");
	const bool capture = true;
	const val eventHandler = val::module_property("app_beforeUnloadHandler");
	if (enable) {
		window.call<void>("addEventListener", std::string("beforeunload"), eventHandler, capture);
	} else {
		window.call<void>("removeEventListener", std::string("beforeunload"), eventHandler, capture);
	}
}




namespace {
void beforeUnloadhandler(emscripten::val event) {
	LOG_CWARNING("client") << "Unload handler";
	// Adding this event handler is sufficent to make the browsers display
	// the confirmation dialog, provided the calls below are also made:
	event.call<void>("preventDefault"); // call preventDefault as required by standard
	event.set("returnValue", std::string(" ")); // set returnValue to something, as required by Chrome
	//return emscripten::val("Sure");
}
}

EMSCRIPTEN_BINDINGS(app) {
	function("app_beforeUnloadHandler", &beforeUnloadhandler);
}
