/*
 * ---- Call of Suli ----
 *
 * panel.cpp
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Panel
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

#include "panel.h"
#include "Logger.h"
#include "serverservice.h"


/**
 * @brief Panel::Panel
 * @param service
 * @param parent
 */

Panel::Panel(ServerService *service)
	: QObject{service}
	, m_service(service)
{
	Q_ASSERT(service);

	LOG_CTRACE("service") << "Panel created:" << this;

	m_deleteTimer.setInterval(10*60*1000);

	connect(&m_deleteTimer, &QTimer::timeout, this, &Panel::onDeleteTimerTimeout);
}

/**
 * @brief Panel::~Panel
 */

Panel::~Panel()
{
	LOG_CTRACE("service") << "Panel destroyed:" << this;
}




const QString &Panel::owner() const
{
	return m_owner;
}

void Panel::setOwner(const QString &newOwner)
{
	m_lastUpdated = QDateTime::currentDateTime();

	if (m_owner == newOwner)
		return;
	m_owner = newOwner;
	emit ownerChanged();
	sendConfig();
}

HttpEventStream *Panel::stream() const
{
	return m_stream;
}

void Panel::setStream(HttpEventStream *newStream)
{
	if (m_stream == newStream)
		return;
	m_stream = newStream;
	emit streamChanged();

	if (m_stream)
		connect(m_stream, &HttpEventStream::destroyed, &m_deleteTimer, [this]{m_deleteTimer.start();});
	else
		m_deleteTimer.start();
}


int Panel::id() const
{
	return m_id;
}

void Panel::setId(int newId)
{
	m_lastUpdated = QDateTime::currentDateTime();
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QJsonObject &Panel::config() const
{
	return m_config;
}

void Panel::setConfig(const QJsonObject &newConfig)
{
	m_lastUpdated = QDateTime::currentDateTime();
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	emit configChanged();
	sendConfig();
}


/**
 * @brief Panel::onDeleteTimerTimeout
 */

void Panel::onDeleteTimerTimeout()
{
	LOG_CTRACE("service") << "Panel delete timer timeout:" << this;
	m_service->removePanel(this, true);
}


/**
 * @brief Panel::lastUpdated
 * @return
 */

const QDateTime &Panel::lastUpdated() const
{
	return m_lastUpdated;
}


/**
 * @brief Panel::getEventData
 * @return
 */

QJsonObject Panel::getEventData() const
{
	QJsonObject o;
	o.insert(QStringLiteral("owner"), m_owner);
	o.insert(QStringLiteral("config"), m_config);
	return o;
}



/**
 * @brief Panel::sendConfig
 */

void Panel::sendConfig()
{
	if (!m_stream)
		return;

	m_lastUpdated = QDateTime::currentDateTime();

	QJsonObject o = getEventData();

	m_stream->write(QByteArrayLiteral("changed"), QJsonDocument(o).toJson(QJsonDocument::Compact));

}


