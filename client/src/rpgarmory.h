/*
 * ---- Call of Suli ----
 *
 * rpgarmory.h
 *
 * Created on: 2024. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgArmory
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

#ifndef RPGARMORY_H
#define RPGARMORY_H

#include "tiledweapon.h"
#include <QObject>

class RpgMageStaff;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_RpgMageStaff
#define OPAQUE_PTR_RpgMageStaff
  Q_DECLARE_OPAQUE_POINTER(RpgMageStaff*)
#endif

#endif

class RpgArmory : public QObject
{
	Q_OBJECT

	Q_PROPERTY(TiledWeaponList *weaponList READ weaponList CONSTANT FINAL)
	Q_PROPERTY(TiledWeapon *currentWeapon READ currentWeapon WRITE setCurrentWeapon NOTIFY currentWeaponChanged FINAL)
	Q_PROPERTY(TiledWeapon *nextWeapon READ nextWeapon WRITE setNextWeapon NOTIFY nextWeaponChanged FINAL)
	Q_PROPERTY(QStringList baseLayers READ baseLayers WRITE setBaseLayers NOTIFY baseLayersChanged FINAL)
	Q_PROPERTY(RpgMageStaff *mageStaff READ mageStaff NOTIFY mageStaffChanged FINAL)

public:
	explicit RpgArmory(TiledObject *parentObject, QObject *parent = nullptr);
	virtual ~RpgArmory();

	static void fillAvailableLayers(IsometricObjectLayeredSprite *dest, const int &spriteNum);
	static void fillLayer(IsometricObjectLayeredSprite *dest, const QString &layer, const int &spriteNum);
	static void fillLayer(IsometricObjectLayeredSprite *dest, const QString &layer, const QString &subdir, const int &spriteNum);
	static const QHash<TiledWeapon::WeaponType, QString> &weaponHash() { return m_layerInfoHash; }

	Q_INVOKABLE bool changeToNextWeapon();

	void updateLayers();

	int getShieldCount() const;

	TiledWeaponList *weaponList() const;
	TiledWeapon *weaponFind(const TiledWeapon::WeaponType &type) const;

	TiledWeapon *weaponAdd(TiledWeapon *weapon);
	void weaponRemove(TiledWeapon *weapon);

	TiledWeapon *currentWeapon() const;
	void setCurrentWeapon(TiledWeapon *newCurrentWeapon);

	QStringList baseLayers() const;
	void setBaseLayers(const QStringList &newBaseLayers);

	void updateNextWeapon();
	void setCurrentWeaponIf(TiledWeapon *newCurrentWeapon, const TiledWeapon::WeaponType &currentType);

	TiledWeapon *nextWeapon() const;
	void setNextWeapon(TiledWeapon *newNextWeapon);

	RpgMageStaff *mageStaff() const;

signals:
	void currentWeaponChanged();
	void baseLayersChanged();
	void nextWeaponChanged();
	void mageStaffChanged();

private:
	TiledWeapon *getNextWeapon() const;

	TiledObject *m_parentObject = nullptr;
	std::unique_ptr<TiledWeaponList> m_weaponList;
	TiledWeapon *m_currentWeapon = nullptr;
	TiledWeapon *m_nextWeapon = nullptr;
	QStringList m_baseLayers = { QStringLiteral("default") };

	static const QHash<TiledWeapon::WeaponType, QString> m_layerInfoHash;

};

#endif // RPGARMORY_H
