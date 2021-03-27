import QtQuick 2.15
import QtQuick.Controls 2.15

Image {

	required property bool isDeathmatch
	required property int level
	required property string image

	source: "qrc:/internal/trophy/iconBg"+
			(isDeathmatch ? "d" : "t")+
			level+".png"
	fillMode: Image.PreserveAspectFit

	Image {
		source: cosClient.medalIconPath(image)
		anchors.centerIn: parent
		width: parent.width*0.75
		height: parent.height*0.75
		fillMode: Image.PreserveAspectFit
	}
}
