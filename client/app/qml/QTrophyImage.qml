import QtQuick 2.15
import QtQuick.Controls 2.15

Image {
	required property bool isDeathmatch
	required property int level

	source: level < 0 ? "qrc:/internal/trophy/trophy0.png"
					  : "qrc:/internal/trophy/trophy"+
						(isDeathmatch ? "d" : "t")+
						level+".png"

	fillMode: Image.PreserveAspectFit
}
