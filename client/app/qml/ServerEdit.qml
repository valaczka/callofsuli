import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: p

	property ServerObject server: null

	title: server ? qsTr("Szerver adatai") : qsTr("Új szerver adatai")
	icon: server ? "qrc:/internal/icon/access-point.svg" : "qrc:/internal/icon/access-point-plus.svg"
	menu: QMenu {
		//MenuItem { action: actionQRread }
		MenuItem { action: actionRemove }
	}


	QGridLayout {
		id: grid

		watchModification: true

		onAccepted: buttonSave.press()

		QTabHeader {
			tabContainer: p
			Layout.columnSpan: parent.columns
			Layout.fillWidth: true
			isPlaceholder: true
		}

		QGridLabel { field: textHostname }

		QGridTextField {
			id: textHostname
			fieldName: qsTr("Cím (host)")
			sqlField: "host"
			inputMethodHints: Qt.ImhUrlCharactersOnly

			validator: RegExpValidator { regExp: /.+/ }
		}

		QGridText {
			text: qsTr("Port")
			field: spinPort
		}

		QGridSpinBox {
			id: spinPort
			sqlField: "port"

			from: 1
			to: 65535
			editable: true
		}


		QGridCheckBox {
			id: checkSsl
			text: qsTr("SSL kapcsolat")
			sqlField: "ssl"
		}

		QGridButton {
			id: buttonSave
			text: server ? qsTr("Mentés") : qsTr("Hozzáadás")
			enabled: textHostname.acceptableInput

			themeColors: CosStyle.buttonThemeGreen
			icon.source: server ? "qrc:/internal/icon/content-save.svg" : "qrc:/internal/icon/content-save-plus.svg"

			onClicked: {
				if (server) {
					JS.updateByModifiedSqlFields(server, [textHostname, spinPort, checkSsl])
				} else {
					var m = JS.getSqlFields([textHostname, spinPort, checkSsl])

					if (Object.keys(m).length) {
						servers.serverCreate(m)
					}
				}
				servers.uiBack()
			}

		}
	}


	Action {
		id: actionRemove
		icon.source: "qrc:/internal/icon/access-point-remove.svg"
		text: qsTr("Törlés")

		enabled: server

		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Szerver törlése"),
										   text: qsTr("Biztosan törlöd a szervert?\n%1:%2").arg(textHostname.text).arg(spinPort.value),
										   image: "qrc:/internal/icon/access-point-remove.svg"
									   })
			d.accepted.connect(function () {
				servers.serverDelete(server)
				servers.uiBack()
			})
			d.open()
		}
	}


	Action {
		id: actionQRread
		text: qsTr("QR beolvasás")
	}


	Component.onCompleted: loadData()

	onPopulated: textHostname.forceActiveFocus()

	function loadData() {
		if (!server) {
			JS.setSqlFields([
								textHostname,
								spinPort,
								checkSsl
							], {host:"", port:10101, ssl:false})
		} else  {
			JS.setSqlFields([
								textHostname,
								spinPort,
								checkSsl
							], server)
		}

		grid.modified = false
	}

}

