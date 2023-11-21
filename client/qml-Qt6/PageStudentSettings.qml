import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPageGradient {
	id: control

	/*stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}

		return true
	}*/

	title: qsTr("Beállítások")

	appBar.backButtonVisible: true

	progressBarEnabled: true



	QScrollable {
		anchors.fill: parent

		Item {
			id: _item
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.8)
			height: control.paddingTop
		}

		Qaterial.Expandable {
			width: _item.width
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true

			header: QExpandableHeader {
				text: qsTr("Kinézet")
				button.visible: false
			}

			delegate: SettingsAppearance {
				width: _item.width
				bottomPadding: 30 * Qaterial.Style.pixelSizeRatio
			}
		}

		Qaterial.Expandable {
			width: _item.width
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true

			header: QExpandableHeader {
				text: qsTr("Hangok")
				button.visible: false
			}

			delegate: SettingsSound {
				width: _item.width
				onMusicVolumeModified: {
					if (!Client.sound.isPlayingMusic())
						Client.sound.playSound("qrc:/sound/menu/bg.mp3", Sound.MusicChannel)
				}

				bottomPadding: 30 * Qaterial.Style.pixelSizeRatio
			}
		}

		Qaterial.Expandable {
			width: _item.width
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true

			visible: Qt.platform.os != "wasm"

			header: QExpandableHeader {
				text: qsTr("Szoftver")
				button.visible: false
			}

			delegate: SettingsSoftware {
				width: _item.width
			}
		}

	}

	Connections {
		target: Qt.application
		function onStateChanged() {
			if (Qt.platform.os !== "android" && Qt.platform.os !== "ios")
				return

			switch (Qt.application.state) {
			case Qt.ApplicationSuspended:
			case Qt.ApplicationHidden:
				if (control.StackView.isCurrentItem)
					Client.sound.stopSound("qrc:/sound/menu/bg.mp3", Sound.MusicChannel)
				break
			case Qt.ApplicationActive:
				if (control.StackView.isCurrentItem)
					Client.sound.playSound("qrc:/sound/menu/bg.mp3", Sound.MusicChannel)
				break
			}
		}
	}

	Component.onDestruction: {
		Client.sound.stopSound("qrc:/sound/menu/bg.mp3", Sound.MusicChannel)
	}
}
