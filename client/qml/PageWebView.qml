import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QPage {
	id: control

	property url url

	appBar.backButtonVisible: true

	Item {
		id: connectingItem
		anchors.fill: parent

		Column {
			anchors.centerIn: parent
			spacing: 20

			Qaterial.LabelBody1 {
				id: _info
				anchors.horizontalCenter: parent.horizontalCenter
				width: Math.min(implicitWidth, Qaterial.Style.maxContainerSize*0.85)
				wrapMode: Text.Wrap
				horizontalAlignment: Text.AlignHCenter

				property int sec: 5

				text: qsTr("A kért weboldal az eszköz alapértelmezett böngészőjében nyílik meg %1 másodperc múlva").arg(sec)

				onSecChanged: {
					if (sec == 0)
						_btn.clicked()
				}

				Timer {
					running: parent.sec > 0 && parent.visible
					interval: 1000
					triggeredOnStart: false
					onTriggered: parent.sec--
				}
			}

			QLabelInformative {
				visible: !(Client.Server && Client.server.config.noCertificateWarning !== undefined)

				text: qsTr("A Call of Suli szerver saját aláírású tanúsítványt használ, amit a böngésző biztonsági kockázatnak fog jelezni. A sikeres azonosításhoz engedélyezni kell majd az oldalra lépést.")
			}

			QButton {
				id: _btn
				anchors.horizontalCenter: parent.horizontalCenter
				highlighted: true
				text: qsTr("Megnyitás böngészőben")
				icon.source: Qaterial.Icons.web
				onClicked: {
					Client.Utils.openUrl(control.url)
					_info.visible = false
				}
			}

			Qaterial.OutlineButton {
				anchors.horizontalCenter: parent.horizontalCenter
				highlighted: false
				text: qsTr("Link másolása")
				icon.source: Qaterial.Icons.contentCopy
				onClicked: {
					Client.Utils.setClipboardText(control.url.toString())
					Client.snack(qsTr("Link vágólapra másolva"))
				}
			}
		}

	}

}
