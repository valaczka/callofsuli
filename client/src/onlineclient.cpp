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


#include "abstractlevelgame.h"
#include "onlineclient.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include "QResource"
#include "application.h"
#include "gameterrain.h"

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
 * @brief OnlineClient::onApplicationStarted
 */

void OnlineClient::onApplicationStarted()
{
	qCDebug(lcClient).noquote() << tr("Download resources");

	QUrl url("wasm_resources.json");
	QNetworkRequest request(url);

	QNetworkReply *reply = m_networkManager->get(request);

	connect(reply, &QNetworkReply::finished, this, &OnlineClient::onResourceDownloaded);

	enableTabCloseConfirmation(true);
}





/**
 * @brief OnlineClient::onResourceDownloaded
 */

void OnlineClient::onResourceDownloaded()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	const QUrl &url = reply->url();

	if (reply->error()) {
		qCWarning(lcClient).noquote() << tr("Nem sikerült a letöltés:") << url << reply->errorString();
	} else {
		qCDebug(lcClient).noquote() << tr("Resource downloaded:") << url;
		const QByteArray &payload = reply->readAll();
		const QString &filename = url.path();

		if (filename == "wasm_resources.json") {
			const QJsonObject &o = Utils::byteArrayToJsonObject(payload);
			const QJsonArray &resources = o.value("resources").toArray();


			if (resources.isEmpty()) {
				qCCritical(lcClient).noquote() << tr("Az erőforráslista üres");
			} else {
				m_resourceList.clear();

				foreach (const QJsonValue &v, resources)
					m_resourceList.append(v.toString());

				qCDebug(lcClient).noquote() << tr("Letöltendő erőforrások:") << m_resourceList;

				foreach (const QString &s, m_resourceList) {
					QNetworkReply *r = m_networkManager->get(QNetworkRequest(QUrl(s)));
					connect(r, &QNetworkReply::finished, this, &OnlineClient::onResourceDownloaded);
				}
			}
		} else {
			if (!m_resourceList.contains(filename) || payload.isEmpty())	{
				qCWarning(lcClient).noquote() << tr("Érvénytelen erőforrás:") << filename;
			} else {
				const std::size_t count = payload.size();
				unsigned char* resourceData = new unsigned char[count];
				std::memcpy(resourceData, payload.constData(), count);

				QResource::registerResource(resourceData);

				qCInfo(lcClient).noquote() << QObject::tr("Register resource: %1").arg(filename);

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

	qCInfo(lcClient).noquote() << tr("Az alkalmazás sikeresen elindult");

	GameTerrain::reloadAvailableTerrains();
	AbstractLevelGame::reloadAvailableMedal();

	stackPushPage("PageStart.qml");
}


/**
 * @brief OnlineClient::enableTabCloseConfirmation
 * @param enable
 */

void OnlineClient::enableTabCloseConfirmation(bool enable)
{
	qCDebug(lcClient).noquote() << tr("ENABLE TAB CLOSE CONFIRMATION") << enable;

	using emscripten::val;
	const val window = val::global("window");
	const bool capture = true;
	const val eventHandler = val::module_property("beforeUnloadHandler");
	if (enable) {
		window.call<void>("addEventListener", std::string("beforeunload"), eventHandler, capture);
		qCDebug(lcClient).noquote() << "CALL";
	} else {
		window.call<void>("removeEventListener", std::string("beforeunload"), eventHandler, capture);
		qCDebug(lcClient).noquote() << "UNLOAD";
	}

}



namespace {
static emscripten::val beforeUnloadhandler(emscripten::val event) {
	qCDebug(lcClient).noquote() << QObject::tr("Unload handler");
	// Adding this event handler is sufficent to make the browsers display
	// the confirmation dialog, provided the calls below are also made:
	event.call<void>("preventDefault"); // call preventDefault as required by standard
	event.set("returnValue", std::string(" ")); // set returnValue to something, as required by Chrome
	return emscripten::val("Sure");
}
}

EMSCRIPTEN_BINDINGS(app) {
	function("beforeUnloadHandler", &beforeUnloadhandler);
}
