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

	connect(&m_animX, &QVariantAnimation::valueChanged, this, &TiledEffectFog::setCurrentX);
	connect(&m_animY, &QVariantAnimation::valueChanged, this, &TiledEffectFog::setCurrentY);

	connect(&m_animX, &QVariantAnimation::finished, this, &TiledEffectFog::restartX);
	connect(&m_animY, &QVariantAnimation::finished, this, &TiledEffectFog::restartY);
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
		LOG_CERROR("scene") << "Texture error";
		return node;
	}

	QRect rect;
	rect.setSize(m_texture->textureSize());

	if (rect.isNull()) {
		LOG_CERROR("scene") << "Invalid size" << m_texture->textureSize();
		return node;
	}

	QSizeF mySize = size();

	mySize.setWidth(mySize.width() + rect.width());
	mySize.setHeight(mySize.height() + rect.height());

	QPoint offset;

	offset.setX(m_currentX);
	offset.setY(m_currentY);

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

	if (!m_texture) {
		m_texture = m_game->getTexture(":/rpg/ambient/fog.png", window());
	}

	restartX();
	restartY();

	connect(m_game, &TiledGame::gameSynchronized, this, &TiledEffectFog::update);
}



/**
 * @brief TiledEffectFog::stop
 */

void TiledEffectFog::stop()
{
	setVisible(false);
	m_animX.stop();
	m_animY.stop();

	if (m_game)
		disconnect(m_game);

	update();
}



/**
 * @brief TiledEffectFog::setCurrentX
 * @param value
 */

void TiledEffectFog::setCurrentX(const QVariant &value)
{
	m_currentX = value.toInt();
}

void TiledEffectFog::setCurrentY(const QVariant &value)
{
	m_currentY = value.toInt();
}

void TiledEffectFog::restartX()
{
	if (!m_texture) {
		LOG_CERROR("scene") << "Missing texture";
		return;
	}

	m_animX.setStartValue(0);
	m_animX.setEndValue(-m_texture->textureSize().width());
	m_animX.setDuration(38000);
	m_animX.setEasingCurve(QEasingCurve::InOutQuad);
	m_animX.start();
}

void TiledEffectFog::restartY()
{
	if (!m_texture) {
		LOG_CERROR("scene") << "Missing texture";
		return;
	}

	m_animY.setStartValue(-m_texture->textureSize().height());
	m_animY.setEndValue(0);
	m_animY.setDuration(48000);
	m_animY.setEasingCurve(QEasingCurve::OutInBack);
	m_animY.start();
}


