import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property Servers servers: null
	property int serverId: -1

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPagePanel {
		id: p

		maximumWidth: 600

		blurSource: bgImage

		QGridLayout {
			id: grid

			anchors.fill: parent

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

				onEditingFinished: {
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

			QGridCheckBox {
				id: checkSsl
				text: qsTr("SSL kapcsolat")
				sqlField: "ssl"
			}

			QGridButton {
				id: buttonSave
				label: qsTr("Mentés")
				disabled: !textName.acceptableInput ||
						  !textHostname.acceptableInput ||
						  !textPort.acceptableInput

				onClicked: {
					var m = JS.getSqlFields([textName, textHostname, textPort, checkSsl],
											serverId === -1)

					if (Object.keys(m).length) {
						servers.serverInfoInsertOrUpdate(serverId, m)
					}
				}

			}
		}
	}

	Component {
		id: dlgModify

		QDialogYesNo {
			id: dlgYesNo

			title: qsTr("Biztosan eldobod a változtatásokat?")

			onDlgAccept: {
				grid.modified = false
				mainStack.back()
			}

		}
	}


	Connections {
		target: servers
		onServerInfoLoaded: {
			textName.setText(server.name)
			textHostname.setText(server.host)
			textPort.setText(server.port)
			checkSsl.setChecked(server.ssl)
			grid.modified = false
			busy.running = false
		}

		onServerInfoUpdated: {
			busy.running = false
			grid.modified = false
			mainStack.back()
		}
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = serverId === -1 ?
					qsTr("Új szerver hozzáadása") :
					qsTr("Szerver szerkesztése")

		if (serverId !== -1) {
			busy.running = true
			servers.serverInfoGet(serverId)
		}
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}


		if (grid.modified) {
			var d = JS.dialogCreate(dlgModify)
			d.open()
			return true
		}

		return false
	}
}
