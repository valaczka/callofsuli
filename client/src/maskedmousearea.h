/*
 * ---- Call of Suli ----
 *
 * maskedmousearea.h
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

#ifndef MASKEDMOUSEAREA_H
#define MASKEDMOUSEAREA_H

#include <QImage>
#include <QQuickItem>

/**
 * @brief The MaskedMouseArea class
 */

class MaskedMouseArea : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(bool pressed READ isPressed NOTIFY pressedChanged)
	Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)
	Q_PROPERTY(QUrl maskSource READ maskSource WRITE setMaskSource NOTIFY maskSourceChanged)
	Q_PROPERTY(qreal alphaThreshold READ alphaThreshold WRITE setAlphaThreshold NOTIFY alphaThresholdChanged)
	Q_PROPERTY(qreal scaleImage READ scaleImage WRITE setScaleImage NOTIFY scaleImageChanged FINAL)

	QML_ELEMENT

public:
	MaskedMouseArea(QQuickItem *parent = 0);

	bool contains(const QPointF &point) const;

	bool isPressed() const { return m_pressed; }
	bool containsMouse() const { return m_containsMouse; }

	QUrl maskSource() const { return m_maskSource; }
	void setMaskSource(const QUrl &source);

	qreal alphaThreshold() const { return m_alphaThreshold; }
	void setAlphaThreshold(qreal threshold);

	qreal scaleImage() const;
	void setScaleImage(qreal newScaleImage);

signals:
	void pressed();
	void released();
	void clicked();
	void canceled();
	void pressedChanged();
	void maskSourceChanged();
	void containsMouseChanged();
	void alphaThresholdChanged();
	void scaleImageChanged();

protected:
	void setPressed(bool pressed);
	void setContainsMouse(bool containsMouse);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void hoverEnterEvent(QHoverEvent *event);
	void hoverLeaveEvent(QHoverEvent *event);
	void mouseUngrabEvent();

private:
	void updateScaledImage();

	bool m_pressed;
	QUrl m_maskSource;
	QImage m_maskImage;
	QImage m_scaledImage;
	QPointF m_pressPoint;
	qreal m_alphaThreshold;
	bool m_containsMouse;
	qreal m_scaleImage = 1.0;
};

#endif // MASKEDMOUSEAREA_H
