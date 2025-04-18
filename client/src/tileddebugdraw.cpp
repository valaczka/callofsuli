/*
 * ---- Call of Suli ----
 *
 * tileddebugdraw.cpp
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

#include <qsgnode.h>
#include <qsgflatcolormaterial.h>

#include "tileddebugdraw.h"
#include <chipmunk/chipmunk_structs.h>


#define CIRCLE_SEGMENTS_COUNT 32




/**
 * @brief The DebugNode class
 */

class DebugNode : public QSGGeometryNode
{
public:
	DebugNode(std::unique_ptr<QSGGeometry> &geometry, const QColor &color)
		: QSGGeometryNode()
		, m_geometry(std::move(geometry))
	{
		setFlag(QSGNode::OwnedByParent);

		m_material.setColor(color);

		setGeometry(m_geometry.get());
		setMaterial(&m_material);
	}

private:
	std::unique_ptr<QSGGeometry> m_geometry;
	QSGFlatColorMaterial m_material;
};





/**
 * @brief TiledDebugDraw::TiledDebugDraw
 * @param parent
 */


TiledDebugDraw::TiledDebugDraw(QQuickItem *parent)
	: QQuickItem(parent)
{
	setFlag(ItemHasContents);
}





/**
 * @brief TiledDebugDraw::updatePaintNode
 * @param node
 * @return
 */

QSGNode *TiledDebugDraw::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
	if (node)
		delete node;

	node = new QSGNode;
	node->setFlag(QSGNode::OwnedByParent);

	const bool isActive = m_scene && /*m_scene() &&*/ isVisible() && opacity() > 0.f;

	if (!isActive)
		return node;

	m_parentNode = node;
	m_scene->debugDrawEvent(this);
	m_parentNode = nullptr;

	return node;
}




/**
 * @brief TiledDebugDraw::createNode
 * @param geometry
 * @param color
 * @param parent
 * @return
 */

void TiledDebugDraw::createNode(std::unique_ptr<QSGGeometry> &geometry, const QColor &color)
{
	if (m_parentNode) {
		DebugNode *node = new DebugNode(geometry, color);
		m_parentNode->appendChildNode(node);
	} else {
		LOG_CERROR("scene") << "Missing parent node";
	}
}




/**
 * @brief TiledDebugDraw::drawCircle
 */

void TiledDebugDraw::drawCircle(const cpVect &center, const float &radius, const QColor &color, const float &lineWidth)
{
	// We'd use QSGGeometry::DrawLineLoop, but it's not supported in Qt 6
std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											CIRCLE_SEGMENTS_COUNT + 1));
	geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(lineWidth);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	for (int i = 0; i <= CIRCLE_SEGMENTS_COUNT; ++i) {
		const float theta = i * 2 * M_PI / CIRCLE_SEGMENTS_COUNT;
		points[i].set(center.x + radius * qCos(theta),
					  center.y + radius * qSin(theta));
	}

	createNode(geometry, color);
}


/**
 * @brief TiledDebugDraw::drawSolidCircle
 * @param center
 * @param radius
 * @param color
 */

void TiledDebugDraw::drawSolidCircle(const cpVect &center, const float &radius, const QColor &color)
{
	// We'd use QSGGeometry::DrawTriangleFan, but it's not supported in Qt 6
	std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											CIRCLE_SEGMENTS_COUNT * 3));
	geometry->setDrawingMode(QSGGeometry::DrawTriangles);
	geometry->setLineWidth(1.0);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	QSGGeometry::Point2D lastPoint;
	lastPoint.set(center.x + radius,
				  center.y);

	for (int i = 1; i <= CIRCLE_SEGMENTS_COUNT; ++i) {
		const float theta = i * 2 * M_PI / CIRCLE_SEGMENTS_COUNT;
		QSGGeometry::Point2D currentPoint;
		currentPoint.set(center.x + radius * qCos(theta),
						 center.y + radius * qSin(theta));

		const int triangleStart = (i - 1) * 3;
		points[triangleStart].set(center.x, center.y);
		points[triangleStart + 1] = lastPoint;
		points[triangleStart + 2] = currentPoint;
		lastPoint = currentPoint;
	}

	createNode(geometry, color);
}





/**
 * @brief TiledDebugDraw::drawSegment
 * @param p1
 * @param p2
 * @param color
 */

void TiledDebugDraw::drawSegment(const cpVect &p1, const cpVect &p2, const QColor &color, const float &lineWidth)
{
	std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2));

	geometry->setDrawingMode(QSGGeometry::DrawLines);
	geometry->setLineWidth(lineWidth);

	geometry->vertexDataAsPoint2D()[0].set(p1.x, p1.y);
	geometry->vertexDataAsPoint2D()[1].set(p2.x, p2.y);

	createNode(geometry, color);
}



/**
 * @brief TiledDebugDraw::drawPolygon
 * @param polygon
 * @param color
 * @param lineWidth
 */

void TiledDebugDraw::drawPolygon(const QPolygonF &polygon, const QColor &color, const float &lineWidth)
{
	if (polygon.size() < 2)
		return;

	std::vector<cpVect> vertices;
	vertices.reserve(polygon.size());

	for (const auto &p : polygon)
		vertices.emplace_back(p.x(), p.y());

	drawPolyLines(vertices.data(), vertices.size(), color, lineWidth);
}


/**
 * @brief TiledDebugDraw::drawPolygon
 * @param polygonShape
 * @param color
 * @param lineWidth
 */

void TiledDebugDraw::drawPolygon(const cpShape *polygonShape, const QPointF &center, const QColor &color, const float &lineWidth)
{
	const int count = cpPolyShapeGetCount(polygonShape);

	if (count < 2)
		return;

	std::vector<cpVect> vertices;
	vertices.reserve(count);

	for (int i=0; i<count; ++i) {
		cpVect v = cpPolyShapeGetVert(polygonShape, i);

		if (cpBody *body = polygonShape->body)
			v = cpTransformVect(body->transform, v);

		v.x += center.x();
		v.y += center.y();
		vertices.push_back(v);
	}

	drawPolygon(vertices.data(), vertices.size(), color, lineWidth);
}



/**
 * @brief TiledDebugDraw::drawPolygon
 * @param vertices
 * @param vertexCount
 * @param color
 */

void TiledDebugDraw::drawPolygon(const cpVect *vertices, const int &vertexCount, const QColor &color, const float &lineWidth)
{
	Q_ASSERT(vertexCount > 1);

	// We'd use QSGGeometry::DrawLineLoop, but it's not supported in Qt 6
	std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											vertexCount + 1));
	geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(lineWidth);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	for (int i = 0; i < vertexCount; ++i) {
		points[i].set(vertices[i].x, vertices[i].y);
	}
	points[vertexCount] = points[0];

	createNode(geometry, color);
}




/**
 * @brief TiledDebugDraw::drawPolyLines
 * @param vertices
 * @param vertexCount
 * @param color
 * @param lineWidth
 */

void TiledDebugDraw::drawPolyLines(const cpVect *vertices, const int &vertexCount, const QColor &color, const float &lineWidth)
{
	Q_ASSERT(vertexCount > 1);

	std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											vertexCount));

	geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(lineWidth);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	for (int i = 0; i < vertexCount; ++i) {
		points[i].set(vertices[i].x, vertices[i].y);
	}

	createNode(geometry, color);
}




/**
 * @brief TiledDebugDraw::drawSolidPolygon
 * @param polygon
 * @param color
 */

void TiledDebugDraw::drawSolidPolygon(const QPolygonF &polygon, const QColor &color)
{
	if (polygon.size() < 2)
		return;

	std::vector<cpVect> vertices;
	vertices.reserve(polygon.size());

	for (const auto &p : polygon)
		vertices.emplace_back(p.x(), p.y());

	drawSolidPolygon(vertices.data(), vertices.size(), 0., color);
}



/**
 * @brief TiledDebugDraw::drawSolidPolygon
 * @param polygonShape
 * @param color
 */

void TiledDebugDraw::drawSolidPolygon(cpShape *polygonShape, const QPointF &center, const QColor &color)
{
	const int count = cpPolyShapeGetCount(polygonShape);

	if (count < 2)
		return;

	std::vector<cpVect> vertices;
	vertices.reserve(count);

	for (int i=0; i<count; ++i) {
		cpVect v = cpPolyShapeGetVert(polygonShape, i);

		if (cpBody *body = polygonShape->body)
			v = cpTransformVect(body->transform, v);

		v.x += center.x();
		v.y += center.y();
		vertices.push_back(v);
	}

	drawSolidPolygon(vertices.data(), vertices.size(), 0., color);
}




/**
 * @brief TiledDebugDraw::drawSolidPolygon
 * @param vertices
 * @param vertexCount
 * @param radius
 * @param color
 */

void TiledDebugDraw::drawSolidPolygon(const cpVect *vertices, const int &vertexCount, const float &radius, const QColor &color)
{
	Q_ASSERT(vertexCount > 2);

	if (radius > 0) {
		LOG_CTRACE("scene") << "Polygon radius in DebugDraw not supported";
	}

	// We'd use QSGGeometry::DrawTriangleFan, but it's not supported in Qt 6
	std::unique_ptr<QSGGeometry> geometry(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											(vertexCount - 2) * 3));
	geometry->setDrawingMode(QSGGeometry::DrawTriangles);
	geometry->setLineWidth(1.0);

	const QPointF origin(vertices[0].x, vertices[0].y);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	QPointF prev(vertices[1].x, vertices[1].y);
	for (int i = 2; i < vertexCount; ++i) {
		const QPointF cur(vertices[i].x, vertices[i].y);

		const int triangleStart = (i - 2) * 3;
		points[triangleStart].set(origin.x(), origin.y());
		points[triangleStart + 1].set(prev.x(), prev.y());
		points[triangleStart + 2].set(cur.x(), cur.y());
		prev = cur;
	}

	createNode(geometry, color);
}



/**
 * @brief TiledDebugDraw::scene
 * @return
 */

TiledScene *TiledDebugDraw::scene() const
{
	return m_scene;
}


/**
 * @brief TiledDebugDraw::setScene
 * @param newScene
 */

void TiledDebugDraw::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;

	if (m_scene)
		m_scene->m_debugDraw = nullptr;

	m_scene = newScene;

	if (m_scene)
		m_scene->m_debugDraw = this;

	emit sceneChanged();
}



/**
 * @brief DebugNode::DebugNode
 * @param color
 */


