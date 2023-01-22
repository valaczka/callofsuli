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

ClientCache::ClientCache()
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
 * @brief ClientCache::contains
 * @param id
 * @return
 */

bool ClientCache::contains(const CacheItemBase &item) const
{
	foreach (CacheItemBase *c, m_list) {
		if (c == item)
			return true;
	}

	return false;
}


/**
 * @brief ClientCache::remove
 * @param item
 * @return
 */

bool ClientCache::remove(const CacheItemBase &item)
{
	LOG_CTRACE("client") << "Cache remove id" << this << item;

	bool r = false;

	for (auto i = m_list.begin(); i != m_list.end(); ) {
		if (*i == item) {
			delete *i;
			i = m_list.erase(i);
			r = true;
		} else {
			++i;
		}
	}

	return r;
}



/**
 * @brief ClientCache::add
 * @param id
 * @return
 */


bool ClientCache::add(CacheItemBase *item)
{
	if (contains(*item)) {
		LOG_CWARNING("client") << "Cache already exists" << this << item;
		return false;
	}

	m_list.append(item);

	LOG_CDEBUG("client") << "Cache added" << this << item;

	return true;
}



/**
 * @brief ClientCache::remove
 * @param olm
 * @return
 */

bool ClientCache::removeAll(qolm::QOlmBase *olm)
{
	if (!olm)
		return false;

	LOG_CTRACE("client") << "Cache remove all" << this << olm;

	bool r = false;

	for (auto i = m_list.begin(); i != m_list.end(); ) {
		CacheItemBase *c = *i;
		if (c->olmBase() == olm) {
			i = m_list.erase(i);
			r = true;
		} else {
			++i;
		}
	}

	return r;
}


/**
 * @brief ClientCache::removeAll
 * @return
 */

void ClientCache::removeAll()
{
	LOG_CTRACE("client") << "Cache remove all" << this;

	qDeleteAll(m_list);
	m_list.clear();
}



/**
 * @brief ClientCache::set
 * @param id
 * @param list
 * @return
 */

bool ClientCache::set(const CacheItemBase &id, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Cache set list" << this << id;

	foreach (CacheItemBase *c, m_list) {
		if (c == id) {
			return c->load(list);
		}
	}

	LOG_CWARNING("client") << "Cache id olm not found" << this << id;
	return false;
}


/**
 * @brief ClientCache::get
 * @param id
 * @return
 */

qolm::QOlmBase *ClientCache::get(const CacheItemBase &id) const
{
	foreach (CacheItemBase *c, m_list) {
		if (id == c) {
			return c->olmBase();
		}
	}

	return nullptr;
}



