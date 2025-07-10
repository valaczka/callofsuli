/*
 * ---- Call of Suli ----
 *
 * tiledeffectfog.cpp
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

#include "tiledeffectfog.h"
#include "qsgsimpletexturenode.h"
#include <QSGNode>


TiledEffectFog::TiledEffectFog(QQuickItem *parent)
	: QQuickItem(parent)
{
	setFlag(ItemHasContents);
}


/**
 * @brief TiledEffectFog::~TiledEffectFog
 */

TiledEffectFog::~TiledEffectFog()
{

}


/**
 * @brief TiledEffectFog::updatePaintNode
 * @param node
 * @return
 */

QSGNode *TiledEffectFog::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
	if (node)
		delete node;

	node = new QSGNode;
	node->setFlag(QSGNode::OwnedByParent);

	if (!m_game || !isVisible())
		return node;

	if (!m_texture) {
		m_texture = m_game->getTexture(":/rpg/ambient/fog.png", window());
	}

	if (!m_texture) {
		LOG_CERROR("scene") << "Texture error";
		return node;
	}

	QRect rect;
	rect.setSize(m_texture->textureSize());

	if (rect.isNull()) {
		LOG_CERROR("scene") << "Invalid size" << m_texture->textureSize();
		return node;
	}

	if (m_animX.state() != QAbstractAnimation::Running) {
		m_animX.setStartValue(0);
		m_animX.setEndValue(-rect.width());
		m_animX.setDuration(38000);
		m_animX.setEasingCurve(QEasingCurve::InOutQuad);
		m_animX.start();
	}

	if (m_animY.state() != QAbstractAnimation::Running) {
		m_animY.setStartValue(-rect.height());
		m_animY.setEndValue(0);
		m_animY.setDuration(48000);
		m_animY.setEasingCurve(QEasingCurve::OutInBack);
		m_animY.start();
	}

	QSizeF mySize = size();

	mySize.setWidth(mySize.width() + rect.width());
	mySize.setHeight(mySize.height() + rect.height());

	QPoint offset;

	if (m_animX.state() == QAbstractAnimation::Running)
		offset.setX(m_animX.currentValue().toInt());

	if (m_animY.state() == QAbstractAnimation::Running)
		offset.setY(m_animY.currentValue().toInt());

	for (int i=0; i<std::ceil((float) mySize.width()/rect.width()); ++i) {
		for (int j=0; j<std::ceil((float) mySize.height()/rect.height()); ++j) {
			QSGSimpleTextureNode *imgNode = new QSGSimpleTextureNode();
			imgNode->setOwnsTexture(false);
			imgNode->setFlag(QSGNode::OwnedByParent);

			QRect imgRect = rect.translated(i*rect.width(), j*rect.height()).translated(offset);

			imgNode->setRect(imgRect);
			imgNode->setSourceRect(rect);
			imgNode->setTexture(m_texture);

			node->appendChildNode(imgNode);
		}
	}

	return node;
}


/**
 * @brief TiledEffectFog::game
 * @return
 */

TiledGame *TiledEffectFog::game() const
{
	return m_game;
}

void TiledEffectFog::setGame(TiledGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();

	connect(m_game, &TiledGame::gameSynchronized, this, &TiledEffectFog::update);
}


