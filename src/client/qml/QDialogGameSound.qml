import QtQuick 2.12
import QtQuick.Controls 2.12
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias volumeMusic: sliderMusic.value
	property alias volumeSfx: sliderSfx.value
	property alias volumeVoiceover: sliderVoiceover.value

	signal volumeMusicModified(real volume)
	signal volumeSfxModified(real volume)
	signal volumeVoiceoverModified(real volume)

	maximumHeight: 500
	maximumWidth: 700

	title: qsTr("Hangerő beállítása")

	icon: CosStyle.iconSetup

	titleColor: CosStyle.colorPrimary

	Column {
		anchors.centerIn: parent
		width: parent.width-50

		spacing: 0

		QLabel {
			font.pixelSize: CosStyle.pixelSize*0.8
			font.weight: Font.Medium
			anchors.left: parent.left
			text: qsTr("Háttérzene")
		}

		Slider {
			id: sliderMusic
			from: 0
			to: 100
			width: parent.width
			onMoved: volumeMusicModified(value)
		}

		QLabel {
			font.pixelSize: CosStyle.pixelSize*0.8
			font.weight: Font.Medium
			anchors.left: parent.left
			text: qsTr("Effektek")
		}

		Slider {
			id: sliderSfx
			from: 0
			to: 100
			width: parent.width
			onMoved: volumeSfxModified(value)
		}

		QLabel {
			font.pixelSize: CosStyle.pixelSize*0.8
			font.weight: Font.Medium
			anchors.left: parent.left
			text: qsTr("Szöveg")
		}

		Slider {
			id: sliderVoiceover
			from: 0
			to: 100
			width: parent.width
			onMoved: volumeVoiceoverModified(value)
		}

	}

	buttons:  QButton {
		id: buttonNo
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Bezárás")
		icon.source: CosStyle.iconClose

		onClicked: dlgClose()
	}


	function populated() {
		buttonNo.forceActiveFocus()
	}

}
