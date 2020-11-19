import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: p

	title: servers.serverKey == -1 ? qsTr("Új szerver adatai") : qsTr("Szerver adatai")
	icon: CosStyle.iconUserWhite
	layoutFillWidth: true

	contextMenuFunc: servers.serverKey == -1 ? null : function (m) {
		m.addAction(actionRemove)
	}

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
			text: servers.serverKey == -1 ? qsTr("Hozzáadás") : qsTr("Mentés")
			enabled: textName.acceptableInput &&
					 textHostname.acceptableInput &&
					 textPort.acceptableInput

			themeColors: CosStyle.buttonThemeApply
			icon.source: CosStyle.iconSave

			onClicked: {
				var m = JS.getSqlFields([textName, textHostname, textPort, checkSsl])

				if (Object.keys(m).length) {
					var nextK = servers.serverInsertOrUpdate(servers.serverKey, m)
					if (swipeMode)
						servers.editing = false
					else if (servers.serverKey == -1)
						servers.serverKey = nextK
				}
			}

		}
	}


	Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Biztosan törlöd a szervert?"),
										   text: textName.text
									   })
			d.accepted.connect(function () {
				servers.serverDeleteKey(servers.serverKey)
				servers.editing = false
				servers.serverKey = -1
			})
			d.open()
		}
	}

	Connections {
		target: servers

		onEditingChanged: loadData()
		onServerKeyChanged: loadData()
	}

	onPanelActivated: textHostname.forceActiveFocus()

	Component.onCompleted: loadData()

	function loadData() {
		if (!servers.editing)
			return

		if (servers.serverKey==-1) {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], {name:"", host:"", port:10101, ssl:false})
		} else  {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], servers.serversModel.getByKey(servers.serverKey))
		}

		grid.modified = false

		if (swipeMode)
			parent.parentPage.swipeToPage(1)

	}

}

