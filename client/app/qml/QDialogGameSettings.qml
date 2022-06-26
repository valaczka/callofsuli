import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias volumeMusic: sliderMusic.value
	property alias volumeSfx: sliderSfx.value
	property alias volumeVoiceover: sliderVoiceover.value
	property alias joystickSize: sliderJoystick.value

	signal volumeMusicModified(real volume)
	signal volumeSfxModified(real volume)
	signal volumeVoiceoverModified(real volume)
	signal joystickSizeModified(real size)

	maximumHeight: 500
	maximumWidth: 700

	title: qsTr("Beállítások")

	icon: "qrc:/internal/icon/message-cog-outline.svg"

	titleColor: CosStyle.colorPrimary


	Flickable {
		width: parent.width-50
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col
			width: parent.width

			spacing: 0

			QLabel {
				font.pixelSize: CosStyle.pixelSize*0.8
				font.weight: Font.Medium
				anchors.left: parent.left
				text: qsTr("Joystick mérete")
			}

			Row {
				spacing: 10
				Slider {
					id: sliderJoystick
					anchors.verticalCenter: parent.verticalCenter
					from: 100
					to: 600
					width: col.width-resetJoystick.width-parent.spacing
					onMoved: joystickSizeModified(value)
				}

				QToolButton {
					id: resetJoystick
					anchors.verticalCenter: parent.verticalCenter
					icon.source: "qrc:/internal/icon/restore.svg"
					display: AbstractButton.IconOnly
					text: qsTr("Visszaállítás")
					onClicked: {
						sliderJoystick.value = 175
						joystickSizeModified(sliderJoystick.value)
					}
				}
			}

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

			QCheckBox {
				id: checkLandscape
				font.pixelSize: CosStyle.pixelSize*0.8
				font.weight: Font.Medium
				anchors.left: parent.left
				text: qsTr("Fekvő orientáció")
				checked: cosClient.forcedLandscape

				visible: Qt.platform.os == "android"

				onToggled: if (checked)
							   cosClient.forceLandscape()
						   else
							   cosClient.resetLandscape()
			}

		}
	}

	buttons:  QButton {
		id: buttonNo
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Bezárás")
		icon.source: "qrc:/internal/icon/close.svg"

		onClicked: dlgClose()
	}


	function populated() {
		buttonNo.forceActiveFocus()
	}

}
