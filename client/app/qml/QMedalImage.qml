import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

Image {

	required property bool isDeathmatch
	required property int level
	required property string image

	source: "qrc:/internal/trophy/iconBg"+
			(isDeathmatch ? "d" : "t")+
			level+".png"
	fillMode: Image.PreserveAspectFit

	Image {
		id: srcImg
		source: cosClient.medalIconPath(image)
		anchors.centerIn: parent
		width: parent.width*0.9
		height: parent.height*0.9
		fillMode: Image.PreserveAspectFit
		visible: false
	}

	Colorize {
		source: srcImg
		anchors.fill: srcImg
		hue: switch (level) {
			 case 1:
				 0.4
				 break
			 case 2:
				 0.1
				 break
			 default:
				 0.2
			 }

		lightness: -0.5
		opacity: 0.8
	}
}
