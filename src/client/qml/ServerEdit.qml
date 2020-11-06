import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: p

	title: servers.serverId == -1 ? qsTr("Új szerver adatai") : qsTr("Szerver adatai")
	icon: CosStyle.iconUserWhite
	layoutFillWidth: true

	QGridLayout {
		id: grid

		anchors.fill: parent

		watchModification: true

		onAccepted: buttonSave.press()

		QGridLabel { field: textHostname }

		QGridTextField {
			id: textHostname
			fieldName: qsTr("Cím (host)")
			sqlField: "host"

			onTextModified: {
				if (textName.text.length === 0)
					textName.text = textHostname.text
			}

			validator: RegExpValidator { regExp: /.+/ }
		}

		QGridLabel { field: textPort }

		QGridTextField {
			id: textPort
			fieldName: qsTr("Port")
			sqlField: "port"
			inputMethodHints: Qt.ImhDigitsOnly

			validator: IntValidator {bottom: 1; top: 65534}
		}

		QGridLabel { field: textName }

		QGridTextField {
			id: textName
			fieldName: qsTr("Szerver elnevezése")
			sqlField: "name"

			validator: RegExpValidator { regExp: /.+/ }
		}

		QGridCheckBox {
			id: checkSsl
			text: qsTr("SSL kapcsolat")
			sqlField: "ssl"
		}

		QGridButton {
			id: buttonSave
			text: servers.serverId == -1 ? qsTr("Hozzáadás") : qsTr("Mentés")
			enabled: textName.acceptableInput &&
					 textHostname.acceptableInput &&
					 textPort.acceptableInput

			themeColors: CosStyle.buttonThemeApply
			icon.source: CosStyle.iconSave

			onClicked: {
				var m = JS.getSqlFields([textName, textHostname, textPort, checkSsl])

				if (Object.keys(m).length) {
					servers.serverInfoInsertOrUpdate(servers.serverId, m)
					if (servers.serverId == -1)
						servers.editing = false
				}
			}

		}
	}

	Connections {
		target: servers

		onServerInfoLoaded: {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], server)

			grid.modified = false
		}


		onEditingChanged: loadData()
		onServerIdChanged: loadData()
	}

	onPanelActivated: textHostname.forceActiveFocus()

	Component.onCompleted: loadData()

	function loadData() {
		if (!servers.editing)
			return

		if (servers.serverId==-1) {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], {name:"", host:"", port:0, ssl:false})
		} else  {
			servers.serverInfoGet(servers.serverId)
		}
	}

}

