/*
 * ---- Call of Suli ----
 *
 * tiledvisualitem.h
 *
 * Created on: 2024. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledVisualItem
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

#ifndef TILEDVISUALITEM_H
#define TILEDVISUALITEM_H

#include <libtiledquick/tilelayeritem.h>
#include <QQuickItem>

class TiledScene;

#ifndef OPAQUE_PTR_TiledScene
#define OPAQUE_PTR_TiledScene
Q_DECLARE_OPAQUE_POINTER(TiledScene*)
#endif


/**
 * @brief The TiledVisualItem class
 */

class TiledVisualItem : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

	Q_PROPERTY(bool glowEnabled READ glowEnabled WRITE setGlowEnabled NOTIFY glowEnabledChanged FINAL)
	Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor NOTIFY glowColorChanged FINAL)
	Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged FINAL)
	Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged FINAL)

	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)

public:
	TiledVisualItem(QQuickItem *parent = nullptr);

	QUrl source() const;
	void setSource(const QUrl &newSource);

	bool glowEnabled() const;
	void setGlowEnabled(bool newGlowEnabled);

	QColor glowColor() const;
	void setGlowColor(const QColor &newGlowColor);

	bool overlayEnabled() const;
	void setOverlayEnabled(bool newOverlayEnabled);

	QColor overlayColor() const;
	void setOverlayColor(const QColor &newOverlayColor);

	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

	QString name() const;
	void setName(const QString &newName);

	TiledQuick::TileLayerItem *layerItem() const;
	void setLayerItem(TiledQuick::TileLayerItem *newLayerItem);

signals:
	void sourceChanged();
	void glowEnabledChanged();
	void glowColorChanged();
	void overlayEnabledChanged();
	void overlayColorChanged();
	void sceneChanged();
	void nameChanged();

private:
	QUrl m_source;
	bool m_glowEnabled = false;
	QColor m_glowColor;
	bool m_overlayEnabled = false;
	QColor m_overlayColor;
	TiledScene *m_scene = nullptr;
	QString m_name;
	TiledQuick::TileLayerItem *m_layerItem = nullptr;
};

#endif // TILEDVISUALITEM_H
