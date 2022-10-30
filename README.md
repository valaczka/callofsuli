# Call of Suli
Call of Suli - (c) 2012-2022 Valaczka János Pál

A Call of Suli egy olyan alkalmazás, amellyel a tanárok feladatlapokat tudnak egyszerűen összeállítani, melyeket a Call of Suli akciójátékká alakít át. A tanulók (játékosok) tehát a feladatokat egy akciójáték keretében tudják megoldani.

A teljes funkcionalitáshoz egy létező Call of Suli szerverhez kell csatlakozni, enélkül csak egy demo pályát lehet végigjátszani.

A tanárok a feladatlapokat (küldetések) pályákba, a tanulókat pedig csoportokba tudják szervezni, a csoportokhoz pályákat tudnak rendelni. A tanulók feladata, hogy a csoportjaik pályáit teljesítsék. A tanárok a csoportjaik haladását és eredményeit nyomon tudják követni.

A pályák összeállításától függően a küldetések egymásra épülhetnek, illetve több nehézségi szinten is megoldhatóak. A küldetések teljesítésért a játékosok pontokat (XP) szereznek, melyekkel a ranglistán előbbre lépnek.

A csoportokon belül időtartamhoz kötött hadjáratokat lehet kitűzni, melyekhez értékelési feltételeket lehet szabni. A hadjárat végén a megoldott feladatok alapján automatikus értékelést (jegy, XP) kapnak a tanulók.

A küldetések során egy karaktert kell irányítaniuk, akinek az ellenfeleit kell időre megölnie. Néhány ellenfelet csak akkor tud legyőzni, ha az adott feladatot helyesen oldja meg. A játékos akkor győz, ha az adott szintet befejezi.

Lehetőség van arra is, hogy akciójáték nélkül csak a kitűzött feladatokat oldják meg a játékosok, ha úgy szeretnék.

### Feladattípusok:
- Egyszerű választás
- Többszörös választás
- Igaz/hamis
- Párosítás
- Szövegkitöltés
- Összeadás-kivonás
- Numerikus válasz
- Sorbarendezés
- Képválasztás

## Kliens letöltés

https://github.com/valaczka/callofsuli/releases

[<img src="https://play.google.com/intl/en_us/badges/static/images/badges/hu_badge_web_generic.png" width=250>](https://play.google.com/store/apps/details?id=hu.piarista.vjp.callofsuli)

## Build

```
git clone --recurse-submodules https://github.com/valaczka/callofsuli.git
cd callofsuli
qmake client.pro
make
make install
```

## Build (iOS)

```
git clone https://github.com/valaczka/callofsuli.git
cd callofsuli
qmake -config release -spec macx-ios-clang client.pro
make
```

## [Köszönet (Credits)](CREDITS.md)
