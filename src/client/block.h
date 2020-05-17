/*
 * ---- Call of Suli ----
 *
 * block.h
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

#ifndef BLOCK_H
#define BLOCK_H

#include "game.h"
#include "abstractstorage.h"

class Game;

class Block
{
public:

	explicit Block(Game *game);
	~Block();

	void setGame(Game *game);
	void addStorage(const QVariantMap &storage);
	int targetCount() const { return m_targets.count(); }
	int currentIndex() const;
	void setCurrentIndex(int currentIndex);
	AbstractStorage::Target currentTarget();

	void resetTargets();


private:
	Game* m_game;
	QList<AbstractStorage *> m_storages;

	QList<AbstractStorage::Target> m_targets;
	int m_currentIndex;
};

#endif // BLOCK_H
