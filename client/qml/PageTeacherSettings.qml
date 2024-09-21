import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server && Client.server.user ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.logoutVariant
		ToolTip.text: qsTr("Kijelentkezés")
		onClicked: {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  Client.logout()
								  },
								  text: qsTr("Biztosan kijelentkezel?"),
								  iconSource: Qaterial.Icons.logoutVariant,
								  title: Client.server ? Client.server.serverName : ""
							  })
		}
	}


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
				bottomPadding: 30 * Qaterial.Style.pixelSizeRatio
				onMusicVolumeModified: {
					if (!Client.sound.isPlayingMusic())
						Client.sound.playSound("qrc:/sound/menu/bg.mp3", Sound.MusicChannel)
				}
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
