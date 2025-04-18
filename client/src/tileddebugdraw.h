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
#include <qsggeometry.h>
#include <chipmunk/chipmunk.h>

#include "tiledscene.h"


/**
 * @brief The TiledDebugDraw class
 */

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

	void drawCircle(const cpVect &center, const float &radius, const QColor &color, const float &lineWidth = 1.);
	void drawCircle(const QPointF &center, const float &radius, const QColor &color, const float &lineWidth = 1.) {
		drawCircle(cpVect{.x = center.x(), .y = center.y()}, radius, color, lineWidth);
	}
	void drawSolidCircle(const cpVect &center, const float &radius, const QColor &color);
	void drawSolidCircle(const QPointF &center, const float &radius, const QColor &color) {
		drawSolidCircle(cpVect{.x = center.x(), .y = center.y()}, radius, color);
	}
	void drawSegment(const cpVect &p1, const cpVect &p2, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const QPolygonF &polygon, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const cpShape *polygonShape, const QPointF &center, const QColor &color, const float &lineWidth = 1.);
	void drawPolygon(const cpVect* vertices, const int &vertexCount, const QColor &color, const float &lineWidth = 1.);
	void drawPolyLines(const cpVect* vertices, const int &vertexCount, const QColor &color, const float &lineWidth = 1.);
	void drawSolidPolygon(const QPolygonF &polygon, const QColor &color);
	void drawSolidPolygon(cpShape *polygonShape, const QPointF &center, const QColor &color);
	void drawSolidPolygon(const cpVect* vertices, const int &vertexCount,
						  const float &radius, const QColor &color);


signals:
	void sceneChanged();

private:
	void createNode(std::unique_ptr<QSGGeometry> &geometry, const QColor &color);

	QPointer<TiledScene> m_scene;
	QSGNode *m_parentNode = nullptr;

};

#endif // TILEDDEBUGDRAW_H
