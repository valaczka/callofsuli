import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSimpleContainer {
	id: p

	property int serverKey: -1

	title: serverKey == -1 ? qsTr("Új szerver adatai") : qsTr("Szerver adatai")
	icon: CosStyle.iconComputerData

	property var contextMenuFunc: serverKey == -1 ? null : function (m) {
		m.addAction(actionRemove)
	}


	QGridLayout {
		id: grid

		watchModification: true

		onAccepted: buttonSave.press()

		QGridLabel { field: textName }

		QGridTextField {
			id: textName
			fieldName: qsTr("Szerver elnevezése")
			sqlField: "name"

			validator: RegExpValidator { regExp: /.+/ }
		}

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

			validator: IntValidator {bottom: 1; top: 65535}
		}


		QGridCheckBox {
			id: checkSsl
			text: qsTr("SSL kapcsolat")
			sqlField: "ssl"
		}

		QGridButton {
			id: buttonSave
			text: serverKey == -1 ? qsTr("Hozzáadás") : qsTr("Mentés")
			enabled: textName.acceptableInput &&
					 textHostname.acceptableInput &&
					 textPort.acceptableInput

			themeColors: CosStyle.buttonThemeGreen
			icon.source: CosStyle.iconSave

			onClicked: {
				var m = JS.getSqlFields([textName, textHostname, textPort, checkSsl])

				if (Object.keys(m).length) {
					servers.serverInsertOrUpdate(serverKey, m)
					servers.uiBack()
				}
			}

		}
	}


	Action {
		id: actionRemove
		icon.source: CosStyle.iconDelete
		text: qsTr("Törlés")

		enabled: serverId != -1

		property int serverId: -1

		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Szerver törlése"),
										   text: qsTr("Biztosan törlöd a szervert?\n%1").arg(textName.text)
									   })
			d.accepted.connect(function () {
				servers.serverDelete({id: serverId})
				servers.uiBack()
			})
			d.open()
		}
	}


	Component.onCompleted: loadData()

	onPopulated: textName.forceActiveFocus()

	function loadData() {
		if (serverKey == -1) {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], {name:"", host:"", port:10101, ssl:false})
		} else  {
			var o = servers.serversModel.getByKey(serverKey)
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], o)
			actionRemove.serverId = o.id
		}

		grid.modified = false
	}

}

