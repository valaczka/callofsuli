/*
 * ---- Call of Suli ----
 *
 * clientcache.cpp
 *
 * Created on: 2023. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ClientCache
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

#include "clientcache.h"
#include "Logger.h"

ClientCache::ClientCache(QObject *parent)
	: QObject(parent)
{
	LOG_CTRACE("client") << "Client cache created" << this;
}


/**
 * @brief ClientCache::~ClientCache
 */

ClientCache::~ClientCache()
{
	removeAll();

	LOG_CTRACE("client") << "Client cache destroyed" << this;
}




/**
 * @brief ClientCache::add
 * @param key
 * @param list
 * @param jsonField
 * @param property
 * @param api
 * @param path
 */




/**
 * @brief ClientCache::contains
 * @param key
 * @return
 */

bool ClientCache::contains(const QString &key) const
{
	return m_list.contains(key);
}


/**
 * @brief ClientCache::remove
 * @param key
 */

void ClientCache::remove(const QString &key)
{
	if (m_list.contains(key)) {
		qolm::QOlmBase *l = m_list.value(key).list;
		if (l)
			l->deleteLater();
	}
	m_list.remove(key);
}


/**
 * @brief ClientCache::removeAll
 * @param olm
 */

void ClientCache::remove(qolm::QOlmBase *olm)
{
	LOG_CTRACE("client") << "Cache remove" << olm << this;

	for (auto i = m_list.begin(); i != m_list.end(); ) {
		if (i->list == olm) {
			i->list->deleteLater();
			i = m_list.erase(i);
		} else {
			++i;
		}
	}
}


/**
 * @brief ClientCache::removeAll
 */

void ClientCache::removeAll()
{
	LOG_CTRACE("client") << "Cache remove all" << this;

	foreach (const CacheItem &it, m_list) {
		if (it.list)
			it.list->deleteLater();
	}

	m_list.clear();
}

/**
 * @brief ClientCache::get
 * @param key
 * @return
 */

qolm::QOlmBase *ClientCache::get(const QString &key) const
{
	if (m_list.contains(key))
		return m_list.value(key).list;
	else
		return nullptr;
}


/**
 * @brief ClientCache::set
 * @param key
 * @param list
 * @return
 */

bool ClientCache::set(const QString &key, const QJsonArray &list)
{
	if (!m_list.contains(key)) {
		LOG_CWARNING("client") << "Key not found:" << key;
		return false;
	}

	const CacheItem &it = m_list.value(key);

	OlmLoader::call(it.func, it.list, list, it.jsonField, it.property, it.allFieldOverride);

	return true;
}


/**
 * @brief ClientCache::reload
 * @param websocket
 * @param key
 * @return
 */


bool ClientCache::reload(WebSocket *websocket, const QString &key)
{
	if (!websocket) {
		LOG_CERROR("client") << "Websocket not set";
		return false;
	}

	if (!m_list.contains(key)) {
		LOG_CWARNING("client") << "Key not found:" << key;
		return false;
	}

	const CacheItem &it = m_list.value(key);

	if (it.api == WebSocket::ApiInvalid) {
		LOG_CWARNING("client") << "API not set for key:" << key;
		return false;
	}


	websocket->send(it.api, it.path)->done(this, [this, key](const QJsonObject &data){
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		set(key, list);
	});
	return true;
}


/**
 * @brief ClientCache::reload
 * @param websocket
 * @param key
 * @param func
 * @return
 */

bool ClientCache::reload(WebSocket *websocket, const QString &key, QObject *inst, QJSValue func)
{
	if (!websocket) {
		LOG_CERROR("client") << "Websocket not set";
		return false;
	}

	if (!m_list.contains(key)) {
		LOG_CWARNING("client") << "Key not found:" << key;
		return false;
	}

	const CacheItem &it = m_list.value(key);

	if (it.api == WebSocket::ApiInvalid) {
		LOG_CWARNING("client") << "API not set for key:" << key;
		return false;
	}


	websocket->send(it.api, it.path)->done(inst, [this, key, func](const QJsonObject &data) mutable {
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		set(key, list);
		if (func.isCallable())
			func.call();
	});
	return true;
}



/**
 * @brief ClientCache::find
 * @param key
 * @param value
 * @return
 */

QObject *ClientCache::find(const QString &key, const QVariant &value)
{
	if (!m_list.contains(key)) {
		LOG_CWARNING("client") << "Key not found:" << key;
		return nullptr;
	}

	const CacheItem &it = m_list.value(key);

	return std::invoke(it.finder, it.list, it.property, value);
}




/**
 * @brief ClientCache::clear
 */

void ClientCache::clear()
{
	foreach (const CacheItem &it, m_list)
		if (it.list)
			it.list->clear();

}




/**
 * @brief ClientCache::callHandler
 * @param key
 * @param list
 * @param array
 * @return
 */

bool ClientCache::callReloadHandler(const QString &key, qolm::QOlmBase *list, const QJsonArray &array) const
{
	if (!m_handlers.contains(key)) {
		LOG_CWARNING("client") << "Handlery key not found:" << key;
		return false;
	}

	if (!list)
		return false;

	const CacheItem &it = m_handlers.value(key);

	OlmLoader::call(it.func, list, array, it.jsonField, it.property, it.allFieldOverride);

	return true;
}



/**
 * @brief ClientCache::callFinderHandler
 * @param key
 * @param list
 * @param value
 * @return
 */

QObject *ClientCache::callFinderHandler(const QString &key, qolm::QOlmBase *list, const QVariant &value) const
{
	if (!m_handlers.contains(key)) {
		LOG_CWARNING("client") << "Handlery key not found:" << key;
		return nullptr;
	}

	if (!list)
		return nullptr;

	const CacheItem &it = m_handlers.value(key);

	return std::invoke(it.finder, list, it.property, value);
}







/**
 * @brief OlmLoader::call
 * @param map
 * @param key
 * @param list
 * @param array
 * @param jsonField
 * @param property
 * @return
 */

bool OlmLoader::call(const QMap<QString, OlmLoaderFunc> &map, const QString &key, qolm::QOlmBase *list, const QJsonArray &array,
					 const char *jsonField, const char *property, const bool &allFieldOverride)
{
	if (map.contains(key)) {
		call(map.value(key), list, array, jsonField, property, allFieldOverride);
		return true;
	}

	return false;
}



/**
 * @brief OlmLoader::call
 * @param func
 * @param list
 * @param array
 * @param jsonField
 * @param property
 * @return
 */

void OlmLoader::call(const OlmLoaderFunc &func, qolm::QOlmBase *list, const QJsonArray &array,
					 const char *jsonField, const char *property, const bool &allFieldOverride)
{
	std::invoke(func, list, array, jsonField, property, allFieldOverride);
}





/**
 * @brief OlmLoader::find
 * @param list
 * @param property
 * @param value
 * @return
 */

QObject *OlmLoader::find(qolm::QOlmBase *list, const char *property, const QVariant &value)
{
	Q_ASSERT(list);

	for (int i=0; i<list->size(); ++i) {
		QObject *o = list->get(i);
		if (o && o->property(property) == value)
			return o;
	}

	return nullptr;
}



/**
 * @brief OlmLoader::selectAll
 * @param list
 * @param selected
 * @param property
 * @return
 */

void OlmLoader::selectAll(qolm::QOlmBase *list, const bool &selected, const char *property)
{
	Q_ASSERT(list);

	for (int i=0; i<list->size(); ++i) {
		QObject *o = list->get(i);
		o->setProperty(property, selected);
	}
}
