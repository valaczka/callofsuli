/*
 * ---- Call of Suli ----
 *
 * contexthelper.cpp
 *
 * Created on: 2025. 07. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ContextHelper
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

#include "contexthelper.h"
#include "client.h"
#include "utils_.h"
#include <QTemporaryFile>


#define TIMER_TIMEOUT	3000
#define JSON_URL "https://valaczka.github.io/callofsuli/context/helper.json"




/**
 * @brief The ContextHelperStorageItem class
 */

class ContextHelperStorageItem
{
public:
	ContextHelperStorageItem() = default;
	~ContextHelperStorageItem() = default;

	int lastId = 0;
	QMap<int, ContextHelperData> list;

	std::optional<ContextHelperData> takeNext();
	bool hasNextImage() const;
	bool isNextImageReady() const;
	QUrl getCurrentImageUrl() const;

	void tmpImageNextClear();
	void tmpImageNextSet(const int &id, const QByteArray &data);

	bool updateImages();

private:
	mutable QRecursiveMutex m_mutex;
	std::unique_ptr<QTemporaryFile> m_tmpImage;
	std::unique_ptr<QTemporaryFile> m_tmpImageNext;
	int m_currentId = -1;
	int m_nextId = -1;
	bool m_used = false;
};




/**
 * @brief The ContextHelperStorage class
 */

class ContextHelperStorage
{
public:
	ContextHelperStorage(Client *client)
		: m_client(client)
	{}

	void setLastId(const ContextHelperData::Context &context, const int &id);
	void addData(const ContextHelperData::Context &context, const ContextHelperData &data);
	void updateNextImage(const ContextHelperData::Context &context);
	void updateNextImages();

	void fromJson(const QJsonObject &data);
	void clear();

	QVariantList toSettingsList() const;

	bool isNextDownloading(const ContextHelperData::Context &context) const;
	std::optional<ContextHelperData> takeNext(const ContextHelperData::Context &context);
	QUrl getCurrentImageUrl(const ContextHelperData::Context &context) const;

private:
	std::unordered_map<ContextHelperData::Context, ContextHelperStorageItem> m_data;
	Client *const m_client;
};





/**
 * @brief ContextHelper::ContextHelper
 * @param client
 */


ContextHelper::ContextHelper(Client *client)
	: QObject(client)
	, m_client(client)
	, m_storage(new ContextHelperStorage(client))
{
	Q_ASSERT(client);

	loadSettings();

	connect(&m_timer, &QTimer::timeout, this, &ContextHelper::onTimerTimeout);
}


/**
 * @brief ContextHelper::~ContextHelper
 */

ContextHelper::~ContextHelper()
{
	saveSettings();
	delete m_storage;
	m_storage = nullptr;
}



/**
 * @brief ContextHelper::download
 */

void ContextHelper::download()
{
	LOG_CDEBUG("client") << "Download ContextHelper data";

	HttpConnection *HttpConnection = m_client->httpConnection();

	if (!HttpConnection) {
		LOG_CERROR("client") << "HttpConnection missing";
		return;
	}

	QNetworkRequest r{QUrl(JSON_URL)};

	QNetworkReply *reply = HttpConnection->networkManager()->get(r);

	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		const QByteArray &data = reply->readAll();

		m_storage->fromJson(Utils::byteArrayToJsonObject(data).value_or(QJsonObject{}));

		reply->deleteLater();
	});
}



/**
 * @brief ContextHelper::setCurrentContext
 * @param context
 */

void ContextHelper::setCurrentContext(const ContextHelperData::Context &context)
{
	m_currentContext = context;

	if (!m_enabled) {
		m_timer.stop();
		return;
	}

	if (m_currentContext == ContextHelperData::ContextInvalid)
		m_timer.stop();
	else
		m_timer.start(TIMER_TIMEOUT);
}



/**
 * @brief ContextHelper::unsetContext
 * @param context
 */

void ContextHelper::unsetContext(const ContextHelperData::Context &context)
{
	if (m_currentContext == context)
		setCurrentContext(ContextHelperData::ContextInvalid);
}


/**
 * @brief ContextHelper::loadSettings
 */

void ContextHelper::loadSettings()
{
	setEnabled(Utils::settingsGet(QStringLiteral("contextHelper/enabled"), true).toBool());

	QVariantList list = Utils::settingsGet(QStringLiteral("contextHelper/last")).toList();

	if (list.size() % 2 != 0) {
		LOG_CERROR("client") << "Invalid ContextHelper list";
		Utils::settingsClear(QStringLiteral("contextHelper/last"));
		return;
	}


	for (auto it = list.cbegin(); it != list.cend(); ++it) {
		ContextHelperData::Context ctx = it->value<ContextHelperData::Context>();

		if (ctx == ContextHelperData::ContextInvalid) {
			LOG_CWARNING("client") << "Invalid ContextHelper context" << *it;
		}

		++it;

		if (it == list.cend())
			break;


		if (ctx == ContextHelperData::ContextInvalid)
			continue;

		m_storage->setLastId(ctx, it->toInt());

	}
}



/**
 * @brief ContextHelper::saveSettings
 */

void ContextHelper::saveSettings()
{
	Utils::settingsSet(QStringLiteral("contextHelper/enabled"), m_enabled);
	Utils::settingsSet(QStringLiteral("contextHelper/last"), m_storage->toSettingsList());
}





/**
 * @brief ContextHelperStorageItem::takeNext
 * @return
 */

std::optional<ContextHelperData> ContextHelperStorageItem::takeNext()
{
	if (m_used)
		return std::nullopt;

	const auto &it = list.upperBound(lastId);

	if (it == list.constEnd())
		return std::nullopt;

	lastId = it.key();

	updateImages();

	m_used = true;

	return it.value();
}


/**
 * @brief ContextHelperStorageItem::hasNextImage
 * @return
 */

bool ContextHelperStorageItem::hasNextImage() const
{
	if (m_used)
		return false;

	const auto &it = list.upperBound(lastId);

	if (it == list.constEnd())
		return false;

	return !it->image.isEmpty();
}


/**
 * @brief ContextHelperStorageItem::isNextImageReady
 * @return
 */

bool ContextHelperStorageItem::isNextImageReady() const
{
	QMutexLocker locker(&m_mutex);
	return (m_nextId != -1 && m_tmpImageNext);
}




/**
 * @brief ContextHelperStorageItem::getCurrentImageUrl
 * @return
 */

QUrl ContextHelperStorageItem::getCurrentImageUrl() const
{
	QMutexLocker locker(&m_mutex);

	if (lastId != m_currentId || !m_tmpImage)
		return QUrl{};

	return QUrl::fromLocalFile(m_tmpImage->fileName());
}



/**
 * @brief ContextHelperStorageItem::tmpImageNextClear
 */

void ContextHelperStorageItem::tmpImageNextClear()
{
	QMutexLocker locker(&m_mutex);

	m_tmpImageNext.reset();
	m_nextId = -1;
}



/**
 * @brief ContextHelperStorageItem::tmpImageNextSet
 * @param id
 * @param data
 */

void ContextHelperStorageItem::tmpImageNextSet(const int &id, const QByteArray &data)
{
	QMutexLocker locker(&m_mutex);

	std::unique_ptr<QTemporaryFile> file = std::make_unique<QTemporaryFile>();

	if (file->open()) {
		file->write(data);
		file->close();

		m_nextId = id;

		m_tmpImageNext.swap(file);
	} else {
		LOG_CERROR("client") << "Error";
		tmpImageNextClear();
	}

}



/**
 * @brief ContextHelperStorageItem::updateImages
 */

bool ContextHelperStorageItem::updateImages()
{
	QMutexLocker locker(&m_mutex);

	if (lastId == m_currentId)
		return false;

	if (lastId == m_nextId && m_nextId != -1) {
		if (m_tmpImageNext)
			m_tmpImage.swap(m_tmpImageNext);
		else
			m_tmpImage.reset();

		m_tmpImageNext.reset();

		m_currentId = m_nextId;
		m_nextId = -1;
		return true;
	}

	m_currentId = -1;
	m_tmpImage.reset();

	return true;
}





/**
 * @brief ContextHelper::enabled
 * @return
 */

bool ContextHelper::enabled() const
{
	return m_enabled;
}

void ContextHelper::setEnabled(bool newEnabled)
{
	if (m_enabled == newEnabled)
		return;
	m_enabled = newEnabled;
	emit enabledChanged();
}



/**
 * @brief ContextHelper::onTimerTimeout
 */

void ContextHelper::onTimerTimeout()
{
	if (!m_enabled) {
		m_timer.stop();
		return;
	}

	if (m_storage->isNextDownloading(m_currentContext))
		return;

	m_timer.stop();

	const std::optional<ContextHelperData> &ptr = takeCurrentData();

	if (!ptr)
		return;

	QQuickWindow *window = m_client->mainWindow();

	if (!window)
		return;

	ContextHelperData data = ptr.value();

	if (!data.image.isEmpty())
		data.image = m_storage->getCurrentImageUrl(m_currentContext).toString();

	QMetaObject::invokeMethod(window, "contextHelperDialog", Qt::AutoConnection,
							  Q_ARG(QVariant, QVariant::fromValue(data))
							  );

}



/**
 * @brief ContextHelper::takeCurrentData
 * @return
 */

std::optional<ContextHelperData> ContextHelper::takeCurrentData()
{
	return m_storage->takeNext(m_currentContext);
}


/**
 * @brief ContextHelperStorage::setLastId
 * @param context
 * @param id
 */

void ContextHelperStorage::setLastId(const ContextHelperData::Context &context, const int &id)
{
	m_data[context].lastId = id;
}



/**
 * @brief ContextHelperStorage::toSettingsList
 * @return
 */

QVariantList ContextHelperStorage::toSettingsList() const
{
	QVariantList list;

	for (const auto &[ctx, d] : m_data) {
		list << ctx;
		list << d.lastId;
	}

	return list;
}



/**
 * @brief ContextHelperStorage::isNextDownloading
 * @param context
 * @return
 */

bool ContextHelperStorage::isNextDownloading(const ContextHelperData::Context &context) const
{
	const auto &it = m_data.find(context);

	if (it == m_data.cend())
		return false;

	if (it->second.hasNextImage() && !it->second.isNextImageReady())
		return true;

	return false;
}



/**
 * @brief ContextHelperStorage::takeNext
 * @param context
 * @return
 */

std::optional<ContextHelperData> ContextHelperStorage::takeNext(const ContextHelperData::Context &context)
{
	if (!m_data.contains(context))
		return std::nullopt;
	else {
		const std::optional<ContextHelperData> &ptr = m_data[context].takeNext();

		updateNextImage(context);

		return ptr;
	}
}



/**
 * @brief ContextHelperStorage::getCurrentImageUrl
 * @param context
 * @return
 */

QUrl ContextHelperStorage::getCurrentImageUrl(const ContextHelperData::Context &context) const
{
	if (!m_data.contains(context))
		return QUrl();
	else
		return m_data.at(context).getCurrentImageUrl();
}



/**
 * @brief ContextHelperStorage::addData
 * @param context
 * @param data
 */

void ContextHelperStorage::addData(const ContextHelperData::Context &context, const ContextHelperData &data)
{
	m_data[context].list[data.id] = data;
}


/**
 * @brief ContextHelperStorage::updateNextImage
 * @param context
 */

void ContextHelperStorage::updateNextImage(const ContextHelperData::Context &context)
{
	auto it = m_data.find(context);

	if (it == m_data.end())
		return;

	const auto &itemIt = it->second.list.upperBound(it->second.lastId);

	it->second.tmpImageNextClear();

	if (itemIt == it->second.list.constEnd())
		return;

	if (itemIt->image.isEmpty())
		return;

	HttpConnection *HttpConnection = m_client->httpConnection();

	if (!HttpConnection) {
		LOG_CERROR("client") << "HttpConnection missing";
		return;
	}

	QNetworkRequest r{QUrl(itemIt->image)};

	QNetworkReply *reply = HttpConnection->networkManager()->get(r);

	QObject::connect(reply, &QNetworkReply::finished, m_client, [this, reply, context, id = itemIt->id]() {
		const QByteArray &data = reply->readAll();

		auto it = m_data.find(context);

		if (it == m_data.end()) {
			LOG_CERROR("client") << "Missing context" << context;
			return;
		}

		it->second.tmpImageNextSet(id, data);

		reply->deleteLater();
	});
}


/**
 * @brief ContextHelperStorage::updateNextImages
 */

void ContextHelperStorage::updateNextImages()
{
	for (const auto &it : m_data) {
		updateNextImage(it.first);
	}
}



/**
 * @brief ContextHelperStorage::fromJson
 * @param data
 */

void ContextHelperStorage::fromJson(const QJsonObject &data)
{
	clear();

	static const QUrl base(JSON_URL);

	for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
		const QString &keyStr = it.key();
		const ContextHelperData::Context ctx = ContextHelperData::Context(keyStr.toInt());

		if (ctx == ContextHelperData::ContextInvalid) {
			LOG_CWARNING("client") << "Invalid context" << keyStr;
			continue;
		}

		const QJsonArray list = it.value().toArray();

		for (const QJsonValue &v : list) {
			ContextHelperData data;
			data.iconColor = QStringLiteral("#26C6DA");
			data.fromJson(v.toObject());

			if (data.id <= 0) {
				LOG_CWARNING("client") << "Invalid data" << v;
				continue;
			}

			if (!data.image.isEmpty()) {
				QUrl relative(data.image);
				data.image = base.resolved(relative).toString();
			}

			addData(ctx, data);
		}
	}

	updateNextImages();
}



/**
 * @brief ContextHelperStorage::clear
 */

void ContextHelperStorage::clear()
{
	for (auto &it : m_data)
		it.second.list.clear();
}
