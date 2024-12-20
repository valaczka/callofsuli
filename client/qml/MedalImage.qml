import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Image {
	id: control

	property bool deathmatch: false
	property int level: -1

	property alias image: srcImg.source
	property alias text: label.text

	opacity: level < 1 ? 0.6 : 1.0

	source: level < 1 ? "qrc:/internal/trophy/iconBgEmpty.png"
					  : "qrc:/internal/trophy/iconBg"+
						(deathmatch ? "d" : "t")+
						level+".png"
	fillMode: Image.PreserveAspectFit

	Image {
		id: srcImg
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

	Text {
		id: label
		color: "white"
		font.family: "Rajdhani"
		font.weight: Font.Bold
		font.pixelSize: parent.height * 0.75
		anchors.centerIn: parent
		visible: level <= 0 && text.length
	}

	Colorize {
		source: label
		anchors.fill: label
		visible: level > 0
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
	}
}
