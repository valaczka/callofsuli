import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPage {
	id: page

	requiredPanelWidth: 900

	title: qsTr("Regisztráció")

	mainToolBarComponent: QToolBusyIndicator { running: page.isBusy }

	property bool isBusy: false

	onlyPanel: QPagePanel {
		id: panel

		title: page.title
		maximumWidth: 600

		onPopulated: textEmail.forceActiveFocus()

		Connections {
			target: page
			onPageActivated: textEmail.forceActiveFocus()
		}

		QGridLayout {
			id: grid

			enabled: !page.isBusy

			property bool domainMode: cosClient.registrationDomains.length

			anchors.fill: parent

			watchModification: false

			QGridLabel {
				field: textEmail
				visible: !grid.domainMode
			}

			QGridTextField {
				id: textEmail
				fieldName: qsTr("E-mail cím")

				visible: !grid.domainMode

				validator: RegExpValidator { regExp: /^(.+@..+\...+)$/ }
			}

			QGridLabel {
				field: textEmail2
				visible: grid.domainMode
			}

			QGridTextFieldCombo {
				id: textEmail2

				visible: grid.domainMode

				textField.validator: RegExpValidator { regExp: /[^@]+/ }

				comboBox.model: cosClient.registrationDomains
			}

			QGridLabel { field: textFirstname }

			QGridTextField {
				id: textFirstname
				fieldName: qsTr("Vezetéknév")

				validator: RegExpValidator { regExp: /.+/ }
			}

			QGridLabel { field: textLastname }

			QGridTextField {
				id: textLastname
				fieldName: qsTr("Keresztnév")

				onAccepted: buttonRegistration.press()

				validator: RegExpValidator { regExp: /.+/ }
			}


			QGridButton {
				id: buttonRegistration
				text: qsTr("Regisztráció")
				enabled: (grid.domainMode || textEmail.acceptableInput) &&
						 (!grid.domainMode || textEmail2.textField.acceptableInput) &&
						 textFirstname.acceptableInput &&
						 textLastname.acceptableInput

				onClicked: {
					page.isBusy = true
					/*cosClient.socketSend({"class": "userInfo", "func": "registrationRequest",
											 "email": (grid.domainMode ? textEmail2.text : textEmail.text),
											 "firstname": textFirstname.text,
											 "lastname": textLastname.text})*/
				}
			}
		}
	}

	Connections {
		target: cosClient

		onRegistrationRequestSuccess: {
			page.isBusy = false
			cosClient.sendMessageInfo(qsTr("Regisztráció"), qsTr("A regisztráció aktiválásához az ideiglenes jelszót elküldtük a megadott email címre."))
			mainStack.back()
		}

		onRegistrationRequestFailed: page.isBusy = false
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}
}
