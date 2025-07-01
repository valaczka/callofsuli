import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Image {
	id: control

	property bool deathmatch: false			// DEPRECATED
	property int level: -1
	property bool solved: true

	property alias image: srcImg.source
	property alias text: label.text

	opacity: !solved ? 0.6 : 1.0

	source: !solved ? "qrc:/internal/trophy/iconBgEmpty.png"
					  : "qrc:/internal/trophy/iconBgt1.png"
					  /*: "qrc:/internal/trophy/iconBg"+
						(deathmatch ? "d" : "t")+
						level+".png"*/
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
		/*hue: switch (level) {
			 case 1:
				 0.4
				 break
			 case 2:
				 0.1
				 break
			 default:
				 0.2
			 }*/

		hue: solved ? 0.4 : 0.2

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
		visible: !solved && text.length
	}

	Colorize {
		source: label
		anchors.fill: label
		visible: solved
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

	Text {
		id: labelText
		text: level
		color: solved ? Qaterial.Colors.yellow900 : Qaterial.Style.colorTheme.disabledText
		font.family: "Rajdhani"
		font.weight: Font.Black
		font.pixelSize: parent.height * 0.75
		anchors.centerIn: parent
		style: Text.Outline
		styleColor: "black"
		visible: level > 0 && label.text.length === 0
	}
}
