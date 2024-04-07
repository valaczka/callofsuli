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

#include "Logger.h"
#include "abstractlevelgame.h"
#include "onlineclient.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include "QResource"
#include "application.h"
#include "gameterrain.h"
#include "actiongame.h"
#include "server.h"
#include "utils_.h"
#include "httpconnection.h"
#include "rpgplayer.h"
#include "rpggame.h"


OnlineClient::OnlineClient(Application *app)
	: Client{app}
{
	connect(this, &OnlineClient::allResourceReady, this, &OnlineClient::onAllResourceReady);

}


/**
 * @brief OnlineClient::~OnlineClient
 */

OnlineClient::~OnlineClient()
{
	enableTabCloseConfirmation(false);
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
	QNetworkReply *reply = m_httpConnection->networkManager()->get(request);
	connect(reply, &QNetworkReply::finished, this, &OnlineClient::onResourceDownloaded);

	enableTabCloseConfirmation(true);

	emscripten::val location = emscripten::val::global("location");
	m_parseUrl = {QString::fromStdString(location["href"].as<std::string>())};
}



/**
 * @brief OnlineClient::onUserLoggedIn
 */

void OnlineClient::onUserLoggedIn()
{
	Client::onUserLoggedIn();

	QSettings s;
	s.setValue(QStringLiteral("usertoken"), server()->token());
	s.sync();
}


/**
 * @brief OnlineClient::onUserLoggedOut
 */

void OnlineClient::onUserLoggedOut()
{
	QSettings s;
	s.setValue(QStringLiteral("usertoken"), QStringLiteral(""));
	s.sync();

	Client::onUserLoggedOut();
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
			const QJsonObject &o = Utils::byteArrayToJsonObject(payload).value_or(QJsonObject{});
			const QJsonArray &resources = o.value(QStringLiteral("resources")).toArray();

			m_demoMode = o.value(QStringLiteral("demo")).toBool(false);

			QUrl url;
			url.setScheme(m_parseUrl.scheme());
			url.setHost(m_parseUrl.host());
			url.setPort(m_parseUrl.port());

			Server *s = new Server(this);
			m_httpConnection->setServer(s);
			s->setUrl(url);

			QSettings settings;

			const QString &token = settings.value(QStringLiteral("usertoken")).toString();

			if (!token.isEmpty())
				s->setToken(token);


			if (resources.isEmpty()) {
				LOG_CERROR("client") << "Az erőforráslista üres";
			} else {
				m_resourceList.clear();

				for (const QJsonValue &v : std::as_const(resources))
					m_resourceList.append(v.toString());

				LOG_CDEBUG("client") << "Letöltendő erőforrások:" << m_resourceList;

				foreach (const QString &s, m_resourceList) {
					QNetworkReply *r = m_httpConnection->networkManager()->get(QNetworkRequest(QUrl(s)));
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
	ActionGame::reloadAvailableCharacters();
	RpgPlayer::reloadAvailableCharacters();
	RpgGame::reloadAvailableTerrains();


	if (m_demoMode)
		m_startPage = loadDemoMap();
	else
		loadDynamicResources();
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


/**
 * @brief OnlineClient::fullScreenHelper
 * @return
 */

bool OnlineClient::fullScreenHelper() const
{
	using emscripten::val;
	const val document = val::global("document");
	const val fullscreenElement = document["fullscreenElement"];
	if (fullscreenElement.isUndefined() || fullscreenElement.isNull())
		return false;
	else
		return true;
}


/**
 * @brief OnlineClient::setFullScreenHelper
 * @param newFullScreenHelper
 */

void OnlineClient::setFullScreenHelper(bool newFullScreenHelper)
{
	using emscripten::val;
	const val document = val::global("document");
	const val fullscreenElement = document["fullscreenElement"];
	if (newFullScreenHelper && (fullscreenElement.isUndefined() || fullscreenElement.isNull()))
		document["documentElement"].call<void>("requestFullscreen");
	else if (!newFullScreenHelper && (!fullscreenElement.isUndefined() || !fullscreenElement.isNull()))
		document.call<void>("exitFullscreen");
}


/**
 * @brief OnlineClient::fullScreenHelperConnect
 * @param window
 */

void OnlineClient::fullScreenHelperConnect(QQuickWindow *window)
{
	Q_UNUSED(window);
}


/**
 * @brief OnlineClient::fullScreenHelperDisconnect
 * @param window
 */

void OnlineClient::fullScreenHelperDisconnect(QQuickWindow *window)
{
	Q_UNUSED(window);
}


/**
 * @brief OnlineClient::saveDynamicResource
 * @param name
 * @param data
 * @return
 */

bool OnlineClient::saveDynamicResource(const QString &name, const QByteArray &data)
{
	if (!server())
		return false;

	if (!server()->dynamicContentRemove(name, data)) {
		messageError(tr("Fájl mentése sikertelen: %1").arg(name));
		return false;
	}

	const std::size_t count = data.size();
	unsigned char* resourceData = new unsigned char[count];
	std::memcpy(resourceData, data.constData(), count);

	QResource::registerResource(resourceData, QStringLiteral("/content"));

	LOG_CINFO("client") << "Register resource:" << qPrintable(name);

	if (server()->dynamicContentList().isEmpty()) {
		server()->setDynamicContentReady(true);
		m_startPage = stackPushPage("PageStart.qml");
	}

	return true;
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


