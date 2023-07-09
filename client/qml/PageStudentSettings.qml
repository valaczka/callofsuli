import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
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
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
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

			delegate: Column{
				width: _item.width
				spacing: 10 * Qaterial.Style.pixelSizeRatio
				SettingsAppearance {
					width: parent.width
				}

				QButton {
					icon.source: Qaterial.Icons.notificationClearAll
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Felugró üzenetek olvasatlanná tevése")
					wrapMode: implicitWidth > parent.width ? Text.Wrap : Text.NoWrap
					onClicked: {
						JS.questionDialog({
											  onAccepted: function()
											  {
												  Client.Utils.settingsClear("notification")
												  Client.snack("Felugró üzenetek olvasatlanok")
											  },
											  text: qsTr("Biztosan olvasatlanná teszel minden felugró üzenetet?"),
											  iconSource: Qaterial.Icons.notificationClearAll,
											  title: qsTr("Felugró üzenetek")
										  })
					}
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
				text: qsTr("Hangok")
				button.visible: false
			}

			delegate: SettingsSound {
				width: _item.width
				onMusicVolumeModified: {
					if (!Client.isPlayingMusic())
						Client.playSound("qrc:/sound/menu/bg.mp3", Sound.Music)
				}
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
					Client.stopSound("qrc:/sound/menu/bg.mp3", Sound.Music)
				break
			case Qt.ApplicationActive:
				if (control.StackView.isCurrentItem)
					Client.playSound("qrc:/sound/menu/bg.mp3", Sound.Music)
				break
			}
		}
	}

	Component.onDestruction: {
		Client.stopSound("qrc:/sound/menu/bg.mp3", Sound.Music)
	}
}
