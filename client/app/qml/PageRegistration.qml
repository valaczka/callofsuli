import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Regisztráció")

	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: QSimpleContainer {
			id: panel

			title: qsTr("Regisztráció")
			icon: CosStyle.iconRegistration
			maximumWidth: 600

			QGridLayoutFlickable {
				id: grid

				property bool domainMode: cosClient.registrationDomains.length

				watchModification: false

				onAccepted: buttonRegistration.press()

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
					forcedText: qsTr("E-mail cím")
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

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridText {
					field: comboClass
					text: qsTr("Osztály:")
					visible: comboClass.visible
				}

				QGridComboBox {
					id: comboClass
					sqlField: "classid"

					visible: cosClient.registrationClasses.length

					valueRole: "classid"
					textRole: "classname"

					model: cosClient.registrationClasses
				}

				QGridButton {
					id: buttonRegistration
					text: qsTr("Regisztráció")
					enabled: (grid.domainMode || textEmail.acceptableInput) &&
							 (!grid.domainMode || textEmail2.textField.acceptableInput) &&
							 textFirstname.acceptableInput &&
							 textLastname.acceptableInput

					onClicked: {
						grid.enabled = false
						cosClient.socketSend(CosMessage.ClassUserInfo, "registrationRequest",
											 {
												 "email": (grid.domainMode ? textEmail2.text : textEmail.text),
												 "firstname": textFirstname.text,
												 "lastname": textLastname.text,
												 "classid": comboClass.visible ? comboClass.sqlData : -1
											 })
					}
				}
			}


			Connections {
				target: cosClient

				function onRegistrationRequestSuccess() {
					cosClient.sendMessageInfo(qsTr("Regisztráció"), qsTr("A regisztráció aktiválásához az ideiglenes jelszót elküldtük a megadott email címre."))
					mainStack.back()
				}

				function onRegistrationRequestFailed(errorString) {
					grid.enabled = true
					cosClient.sendMessageError(qsTr("Sikertelen regisztráció"), errorString)
				}
			}

			onPopulated: grid.domainMode ? textEmail2.forceActiveFocus() : textEmail.forceActiveFocus()
		}


	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}



