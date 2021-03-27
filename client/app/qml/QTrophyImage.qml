import QtQuick 2.15
import QtQuick.Controls 2.15

Image {
	required property bool isDeathmatch
	required property int level

	source: "qrc:/internal/trophy/trophy"+
			(isDeathmatch ? "d" : "t")+
			level+".png"

	fillMode: Image.PreserveAspectFit
}
