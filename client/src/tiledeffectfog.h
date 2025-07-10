/*
 * ---- Call of Suli ----
 *
 * tiledeffectfog.h
 *
 * Created on: 2025. 07. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledEffectFog
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

#ifndef TILEDEFFECTFOG_H
#define TILEDEFFECTFOG_H

#include "qvariantanimation.h"
#include "tiledgame.h"
#include <QQuickItem>

class TiledEffectFog : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)

public:
	TiledEffectFog(QQuickItem *parent = nullptr);
	virtual ~TiledEffectFog();

	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

	TiledGame *game() const;
	void setGame(TiledGame *newGame);

signals:
	void gameChanged();

private:
	TiledGame *m_game = nullptr;
	QSGTexture *m_texture = nullptr;
	QVariantAnimation m_animX;
	QVariantAnimation m_animY;
};

#endif // TILEDEFFECTFOG_H
