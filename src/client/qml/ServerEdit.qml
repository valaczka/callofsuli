import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: p

	property Servers servers: null
	property int serverId: -1

	title: serverId == -1 ? qsTr("Új szerver adatai") : qsTr("Szerver adatai")
	icon: CosStyle.iconUserWhite

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
			text: serverId == -1 ? qsTr("Hozzáadás") : qsTr("Mentés")
			enabled: textName.acceptableInput &&
					 textHostname.acceptableInput &&
					 textPort.acceptableInput

			themeColors: CosStyle.buttonThemeApply
			icon.source: CosStyle.iconSave

			onClicked: {
				var m = JS.getSqlFields([textName, textHostname, textPort, checkSsl])

				if (Object.keys(m).length) {
					servers.serverInfoInsertOrUpdate(serverId, m)
				}

				pageStart.unloadEditor()
			}

		}
	}

	onServerIdChanged: if (serverId==-1) {
						   JS.setSqlFields([
											   textName,
											   textHostname,
											   textPort,
											   checkSsl
										   ], {name:"", host:"", port:0, ssl:false})
					   } else  {
						   if (servers)
							   servers.serverInfoGet(serverId)
					   }

	Connections {
		target: pageStart
		onServerEdit: serverId = id
		onServerCreate: serverId = -1
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
	}


	function populated() {
		console.debug("SERVER EDITPOPULATED")
		servers.serverInfoGet(serverId)
	}

	on_IsCurrentChanged:  if (_isCurrent) textHostname.forceActiveFocus()

}

