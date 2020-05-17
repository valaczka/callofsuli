/*
 * ---- Call of Suli ----
 *
 * block.cpp
 *
 * Created on: 2020. 05. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Block
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

#include "block.h"
#include "questionpair.h"


Block::Block(Game *game)
	: m_game(game)
{
	m_currentIndex = -1;
}


/**
 * @brief Block::~Block
 */

Block::~Block()
{
	qDeleteAll(m_storages.begin(), m_storages.end());
	m_storages.clear();
}


/**
 * @brief Block::addStorage
 * @param storage
 */

void Block::addStorage(const QVariantMap &storage)
{
	QString module = storage.value("module").toString();

	QString data = storage.value("data").toString();
	QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
	QJsonObject d = doc.object();

	QVariantList objectives = storage.value("objectives").toList();

	if (objectives.isEmpty()) {
		qDebug() << "Empty storage skipped" << module;
		return;
	}

	AbstractStorage *AS = nullptr;

	if (module == "questionpair")
		AS = new Questionpair(d);
	else {
		qWarning() << "INVALID MODULE" << module;
	}


	if (AS) {
		AS->fillContainers();
		AS->setObjectives(objectives);

		m_storages << AS;
	}

}



/**
 * @brief Block::resetTargets
 */

void Block::resetTargets()
{
	m_targets.clear();

	QList<AbstractStorage::Target> list;

	foreach (AbstractStorage *AS, m_storages) {
		list << AS->createTargets();
	}

	while (list.count()) {
		int idx = random() % list.count();
		m_targets.append(list.takeAt(idx));
	}

	qDebug() << "CREATE TARGET" << m_targets.count();
	//qDebug() << m_targets;
}


int Block::currentIndex() const
{
	return m_currentIndex;
}

void Block::setCurrentIndex(int currentIndex)
{
	m_currentIndex = currentIndex;
}


/**
 * @brief Block::currentTarget
 * @return
 */

AbstractStorage::Target Block::currentTarget()
{
	if (m_currentIndex >= 0 && m_currentIndex < m_targets.count()) {
		return m_targets.value(m_currentIndex);
	}

	return AbstractStorage::Target(nullptr);
}

