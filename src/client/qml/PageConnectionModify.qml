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

	property ListModel connectionModel: null
	property int connectionModelIndex: -1

	signal saveData(var fields)

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

			QGridLabel { field: textDb }

			QGridTextField {
				id: textDb
				fieldName: qsTr("Adatbázis")
				sqlField: "db"

				validator: RegExpValidator { regExp: /.+/ }
			}

			QGridButton {
				id: buttonSave
				label: qsTr("Mentés")
				disabled: !textName.acceptableInput ||
						  !textHostname.acceptableInput ||
						  !textPort.acceptableInput ||
						  !textDb.acceptableInput

				onClicked: saveData(JS.getSqlFields([textName, textHostname, textPort, textDb],
										connectionModelIndex === -1))

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


	Component.onCompleted: {
		if (connectionModelIndex === -1)
			return

		var m = connectionModel.get(connectionModelIndex)

		textName.setText(m.label)
		textHostname.setText(m.serverHost)
		textPort.setText(m.serverPort)
		textDb.setText(m.database)
		grid.modified = false
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = connectionModelIndex === -1 ?
					qsTr("Új szerver hozzáadása") :
					qsTr("Szerver szerkesztése")

		servers.sendMessageWarning("SIKER", "Úgy tűnik, sikerült!", "Remélem...")

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
