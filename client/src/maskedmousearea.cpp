/*
 * ---- Call of Suli ----
 *
 * maskedmousearea.cpp
 *
 * Created on: 2024. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MaskedMouseArea
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

#include "maskedmousearea.h"
#include <QStyleHints>
#include <QGuiApplication>
#include <qqmlfile.h>


MaskedMouseArea::MaskedMouseArea(QQuickItem *parent)
	: QQuickItem(parent),
	  m_pressed(false),
	  m_alphaThreshold(0.0),
	  m_containsMouse(false)
{
	setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptTouchEvents(true);
}

void MaskedMouseArea::setPressed(bool pressed)
{
	if (m_pressed != pressed) {
		m_pressed = pressed;
		emit pressedChanged();
	}
}

void MaskedMouseArea::setContainsMouse(bool containsMouse)
{
	if (m_containsMouse != containsMouse) {
		m_containsMouse = containsMouse;
		emit containsMouseChanged();
	}
}

void MaskedMouseArea::setMaskSource(const QUrl &source)
{
	if (m_maskSource != source) {
		m_maskSource = source;
		m_maskImage = QImage(QQmlFile::urlToLocalFileOrQrc(source));

		emit maskSourceChanged();

		updateScaledImage();
	}
}

void MaskedMouseArea::setAlphaThreshold(qreal threshold)
{
	if (m_alphaThreshold != threshold) {
		m_alphaThreshold = threshold;
		emit alphaThresholdChanged();
	}
}

bool MaskedMouseArea::contains(const QPointF &point) const
{
	if (!QQuickItem::contains(point) || m_scaledImage.isNull())
		return false;

	QPoint p = point.toPoint();
	if (p.x() < 0 || p.x() >= m_scaledImage.width() ||
		p.y() < 0 || p.y() >= m_scaledImage.height())
		return false;

	qreal r = qBound<int>(0, m_alphaThreshold * 255, 255);
	return qAlpha(m_scaledImage.pixel(p)) > r;
}

void MaskedMouseArea::mousePressEvent(QMouseEvent *event)
{
	setPressed(true);
	m_pressPoint = event->pos();
	emit pressed();
}

void MaskedMouseArea::mouseReleaseEvent(QMouseEvent *event)
{
	setPressed(false);
	emit released();

	QPointF pos = event->position();
	pos -= m_pressPoint;

	const int threshold = qApp->styleHints()->startDragDistance();
	if (pos.manhattanLength() < 2*threshold)
		emit clicked();
}

void MaskedMouseArea::mouseUngrabEvent()
{
	setPressed(false);
	emit canceled();
}


/**
 * @brief MaskedMouseArea::touchEvent
 * @param event
 */

void MaskedMouseArea::touchEvent(QTouchEvent *event)
{
	QPointF pos;

	if (event->pointCount() != 1)
		return;

	if (event->isBeginEvent()) {
		m_touchPoint = event->point(0).position();
		return;
	} else if (event->isEndEvent()) {
		pos = event->point(0).position();
	} else
		return;

	pos -= m_touchPoint;

	const int threshold = qApp->styleHints()->startDragDistance();
	if (pos.manhattanLength() < 2*threshold) {
		emit clicked();

		if (event->timestamp() - m_lastTouch < (quint64) qApp->styleHints()->mouseDoubleClickInterval())
			emit doubleClicked();
	}

	m_lastTouch = event->timestamp();
}


/**
 * @brief MaskedMouseArea::updateScaledImage
 */

void MaskedMouseArea::updateScaledImage()
{
	if (m_maskImage.isNull()) {
		m_scaledImage = {};
		return;
	}

	m_scaledImage = m_maskImage.scaled(m_maskImage.width()*m_scaleImage, m_maskImage.height()*m_scaleImage, Qt::KeepAspectRatio);
}

qreal MaskedMouseArea::scaleImage() const
{
	return m_scaleImage;
}

void MaskedMouseArea::setScaleImage(qreal newScaleImage)
{
	if (qFuzzyCompare(m_scaleImage, newScaleImage))
		return;
	m_scaleImage = newScaleImage;
	emit scaleImageChanged();
	updateScaledImage();
}

void MaskedMouseArea::hoverEnterEvent(QHoverEvent *event)
{
	Q_UNUSED(event);
	setContainsMouse(true);
}

void MaskedMouseArea::hoverLeaveEvent(QHoverEvent *event)
{
	Q_UNUSED(event);
	setContainsMouse(false);
}


/**
 * @brief MaskedMouseArea::mouseDoubleClickEvent
 * @param event
 */

void MaskedMouseArea::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	emit doubleClicked();
}
