/*
 * ---- Call of Suli ----
 *
 * game.h
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Game
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

#ifndef ABSTRACTGAME_H
#define ABSTRACTGAME_H

#include "qloggingcategory.h"
#include "qquickitem.h"
#include "gamemap.h"
#include <QObject>

class Client;


/**
 * @brief The Game class
 */

class AbstractGame : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem *pageItem READ pageItem WRITE setPageItem NOTIFY pageItemChanged)
	Q_PROPERTY(GameMap* map READ map WRITE setMap NOTIFY mapChanged)

public:

	// Játékmód

	enum Mode {
		Invalid,
		Action,
		Lite,
		Exam,
		Quiz
	};

	Q_ENUM(Mode);

	explicit AbstractGame(const Mode &mode, Client *client);
	virtual ~AbstractGame();

	QQuickItem *pageItem() const;
	void setPageItem(QQuickItem *newPageItem);

	const Mode &mode() const;

	GameMap *map() const;
	void setMap(GameMap *newMap);

	Q_INVOKABLE void unloadPageItem();

public slots:
	bool load();
	void finishGame();

protected:
	virtual QQuickItem *loadPage() = 0;

private slots:
	void onPageItemDestroyed();

signals:
	void pageItemChanged();
	void mapChanged();

protected:
	Client *m_client = nullptr;
	QQuickItem *m_pageItem = nullptr;
	const Mode m_mode;
	GameMap *m_map = nullptr;
};

Q_DECLARE_LOGGING_CATEGORY(lcGame)

#endif // ABSTRACTGAME_H
