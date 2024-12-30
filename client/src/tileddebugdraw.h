/*
 * ---- Call of Suli ----
 *
 * tileddebugdraw.h
 *
 * Created on: 2024. 12. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledDebugDraw
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

#ifndef TILEDDEBUGDRAW_H
#define TILEDDEBUGDRAW_H

#include <QQuickItem>
#include <box2cpp/box2cpp.h>
#include <qsggeometry.h>
#include "tiledscene.h"

class TiledDebugDraw : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)

public:
	explicit TiledDebugDraw(QQuickItem *parent = nullptr);

	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

	void drawCircle(const b2Vec2 &center, const float &radius, const QColor &color, const float &lineWidth = 1.);
	void drawCircle(const QPointF &center, const float &radius, const QColor &color, const float &lineWidth = 1.) {
		drawCircle(b2Vec2{(float) center.x(), (float) center.y()}, radius, color, lineWidth);
	}
	void drawSolidCircle(const b2Transform &transform, const float &radius, const QColor &color);
	void drawSolidCircle(const QPointF &center, const float &radius, const QColor &color);
	void drawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const QPolygonF &polygon, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const b2Vec2* vertices, const int &vertexCount, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const b2Transform &transform, const b2Vec2* vertices, const int &vertexCount, const QColor &color,
					 const float &lineWidth = 1.);
	void drawPolyLines(const b2Vec2* vertices, const int &vertexCount, const QColor &color, const float &lineWidth = 1.);
	void drawSolidPolygon(const b2Transform &transform, const b2Vec2* vertices, const int &vertexCount,
						  const float &radius, const QColor &color);

	static QColor box2dColorToQColor(const b2HexColor &color, const float &alpha = 1.0f);

signals:
	void sceneChanged();

private:
	QSGNode *createNode(QSGGeometry *geometry, const QColor &color);

	static QPointF getPolygonVertex(const b2Vec2* vertices, const int num, const b2Transform &transform);

	b2DebugDraw m_callbacks;
	QPointer<TiledScene> m_scene;
	QSGNode *m_parentNode = nullptr;

};

#endif // TILEDDEBUGDRAW_H
