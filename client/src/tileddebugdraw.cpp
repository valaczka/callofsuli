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

#include "tileddebugdraw.h"
#include <qsgnode.h>
#include <qsggeometry.h>
#include <qsgflatcolormaterial.h>


#define CIRCLE_SEGMENTS_COUNT 32


TiledDebugDraw::TiledDebugDraw(QQuickItem *parent)
	: QQuickItem(parent)
{
	setFlag(ItemHasContents);

	m_callbacks = b2DefaultDebugDraw();
	m_callbacks.context = this;

	m_callbacks.drawShapes = true;
	//m_callbacks.drawAABBs = true;


	m_callbacks.DrawPoint = [](b2Vec2 center, float size, b2HexColor color, void *context) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawCircle(center, size, box2dColorToQColor(color));
	};

	m_callbacks.DrawCircle = [](b2Vec2 center, float radius, b2HexColor color, void *context) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawCircle(center, radius, box2dColorToQColor(color));
	};

	m_callbacks.DrawSolidCircle = [](b2Transform transform, float radius, b2HexColor color, void* context ) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawSolidCircle(transform, radius, box2dColorToQColor(color));
	};

	m_callbacks.DrawSegment = [](b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawSegment(p1, p2, box2dColorToQColor(color));
	};

	m_callbacks.DrawPolygon = [](const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context ) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawPolygon(vertices, vertexCount, box2dColorToQColor(color));
	};

	m_callbacks.DrawSolidPolygon = [](b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius,
			b2HexColor color, void* context ) {
		TiledDebugDraw &self = *static_cast<TiledDebugDraw*>(context);
		self.drawSolidPolygon(transform, vertices, vertexCount, radius, box2dColorToQColor(color));
	};
}



/**
 * @brief TiledDebugDraw::updatePaintNode
 * @param node
 * @return
 */

QSGNode *TiledDebugDraw::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
	if (!node) {
		node = new QSGNode;
	}

	const bool isActive = m_scene && m_scene->world() && isVisible() && opacity() > 0.f;

	if (node->childCount() > 0)
		node->removeAllChildNodes();

	if (!isActive)
		return node;

	m_parentNode = node;
	///m_scene->world()->Draw(m_callbacks);			// Replace with own implementation
	m_scene->debugDrawEvent(this);
	m_parentNode = nullptr;

	return node;
}


/**
 * @brief TiledDebugDraw::box2dColorToQColor
 * @param color
 * @param fill
 * @return
 */

QColor TiledDebugDraw::box2dColorToQColor(const b2HexColor &color, const float &alpha)
{
	const unsigned int a = std::lerp(0x00u, 0xFFu, std::max(std::min(1.0f, alpha), 0.0f));

	const QRgb rgb = ((a & 0xffu) << 24) | color;

	return QColor::fromRgb(rgb);
}



/**
 * @brief TiledDebugDraw::createNode
 * @param geometry
 * @param color
 * @param parent
 * @return
 */

QSGNode *TiledDebugDraw::createNode(QSGGeometry *geometry, const QColor &color)
{
	QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
	material->setColor(color);

	QSGGeometryNode *node = new QSGGeometryNode;
	node->setGeometry(geometry);
	node->setFlag(QSGNode::OwnsGeometry);
	node->setMaterial(material);
	node->setFlag(QSGNode::OwnsMaterial);

	if (m_parentNode) {
		m_parentNode->appendChildNode(node);
	} else {
		LOG_CERROR("scene") << "Missing parent node";
	}

	return node;
}




/**
 * @brief TiledDebugDraw::drawCircle
 */

void TiledDebugDraw::drawCircle(const b2Vec2 &center, const float &radius, const QColor &color, const float &lineWidth)
{
	// We'd use QSGGeometry::DrawLineLoop, but it's not supported in Qt 6
	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											CIRCLE_SEGMENTS_COUNT + 1);
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
 * @param transform
 * @param radius
 * @param color
 */

void TiledDebugDraw::drawSolidCircle(const b2Transform &transform, const float &radius, const QColor &color)
{
	// We'd use QSGGeometry::DrawTriangleFan, but it's not supported in Qt 6
	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											CIRCLE_SEGMENTS_COUNT * 3);
	geometry->setDrawingMode(QSGGeometry::DrawTriangles);
	geometry->setLineWidth(1.0);

	QPointF centerInPixels(transform.p.x, transform.p.y);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	QSGGeometry::Point2D lastPoint;
	lastPoint.set(centerInPixels.x() + radius,
				  centerInPixels.y());

	for (int i = 1; i <= CIRCLE_SEGMENTS_COUNT; ++i) {
		const float theta = i * 2 * M_PI / CIRCLE_SEGMENTS_COUNT;
		QSGGeometry::Point2D currentPoint;
		currentPoint.set(centerInPixels.x() + radius * qCos(theta),
						 centerInPixels.y() + radius * qSin(theta));

		const int triangleStart = (i - 1) * 3;
		points[triangleStart].set(centerInPixels.x(), centerInPixels.y());
		points[triangleStart + 1] = lastPoint;
		points[triangleStart + 2] = currentPoint;
		lastPoint = currentPoint;
	}

	createNode(geometry, color);

	/*drawSegment(transform.p,
				b2Vec2(transform.p.x + transform.q.c * radius,
					   transform.p.y + transform.q.s * radius),
				qRgb(200, 40, 0));*/
}


/**
 * @brief TiledDebugDraw::drawSolidCircle
 * @param center
 * @param radius
 * @param color
 */

void TiledDebugDraw::drawSolidCircle(const QPointF &center, const float &radius, const QColor &color)
{
	b2Transform t;
	t.p = b2Vec2{(float) center.x(), (float) center.y()};
	t.q = b2MakeRot(0.);

	drawSolidCircle(t, radius, color);
}





/**
 * @brief TiledDebugDraw::drawSegment
 * @param p1
 * @param p2
 * @param color
 */

void TiledDebugDraw::drawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const QColor &color, const float &lineWidth)
{
	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2);
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

	std::vector<b2Vec2> vertices;
	vertices.reserve(polygon.size());

	for (const auto &p : polygon)
		vertices.emplace_back(p.x(), p.y());

	drawPolyLines(vertices.data(), vertices.size(), color, lineWidth);
}



/**
 * @brief TiledDebugDraw::drawPolygon
 * @param vertices
 * @param vertexCount
 * @param color
 */

void TiledDebugDraw::drawPolygon(const b2Vec2 *vertices, const int &vertexCount, const QColor &color, const float &lineWidth)
{
	Q_ASSERT(vertexCount > 1);

	// We'd use QSGGeometry::DrawLineLoop, but it's not supported in Qt 6
	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											vertexCount + 1);
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
 * @brief TiledDebugDraw::drawPolygon
 * @param transform
 * @param vertices
 * @param vertexCount
 * @param color
 * @param lineWidth
 */

void TiledDebugDraw::drawPolygon(const b2Transform &transform, const b2Vec2 *vertices, const int &vertexCount,
								 const QColor &color, const float &lineWidth)
{
	Q_ASSERT(vertexCount > 1);

	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											vertexCount+1);

	geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(lineWidth);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	for (int i = 0; i < vertexCount; ++i) {
		b2Vec2 trp = b2TransformPoint(transform, vertices[i]);
		points[i].set(trp.x, trp.y);
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

void TiledDebugDraw::drawPolyLines(const b2Vec2 *vertices, const int &vertexCount, const QColor &color, const float &lineWidth)
{
	Q_ASSERT(vertexCount > 1);

	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											vertexCount);
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
 * @param transform
 * @param vertices
 * @param vertexCount
 * @param radius
 * @param color
 */

void TiledDebugDraw::drawSolidPolygon(const b2Transform &transform, const b2Vec2 *vertices, const int &vertexCount,
									  const float &radius, const QColor &color)
{
	Q_ASSERT(vertexCount > 2);

	if (radius > 0) {
		LOG_CTRACE("scene") << "Polygon radius in DebugDraw not supported";
	}

	// We'd use QSGGeometry::DrawTriangleFan, but it's not supported in Qt 6
	QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
											(vertexCount - 2) * 3);
	geometry->setDrawingMode(QSGGeometry::DrawTriangles);
	geometry->setLineWidth(1.0);

	const QPointF origin = getPolygonVertex(vertices, 0, transform);

	QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
	QPointF prev = getPolygonVertex(vertices, 1, transform);
	for (int i = 2; i < vertexCount; ++i) {
		const QPointF cur = getPolygonVertex(vertices, i, transform);

		const int triangleStart = (i - 2) * 3;
		points[triangleStart].set(origin.x(), origin.y());
		points[triangleStart + 1].set(prev.x(), prev.y());
		points[triangleStart + 2].set(cur.x(), cur.y());
		prev = cur;
	}

	createNode(geometry, color);
}


/**
 * @brief TiledDebugDraw::getPolygonVertex
 * @param vertices
 * @param vertexCount
 * @param num
 * @param transform
 * @return
 */

QPointF TiledDebugDraw::getPolygonVertex(const b2Vec2 *vertices, const int num, const b2Transform &transform)
{
	b2Vec2 trp = b2TransformPoint(transform, vertices[num]);
	return QPointF{trp.x, trp.y};
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
		m_scene->disconnect(this);

	m_scene = newScene;

	if (m_scene)
		connect(m_scene, &TiledScene::worldStepped, this, &TiledDebugDraw::update);

	emit sceneChanged();
}
