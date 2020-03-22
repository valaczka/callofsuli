import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	title: qsTr("Szerver beállításai")
	icon: CosStyle.iconSetup
	action: actionSave

	property bool _closeEnabled: false

	QAccordion {
		QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		QCollapsible {
			title: qsTr("Általános")

			QGridLayout {
				id: grid1
				enabled: !serverSettings.isBusy

				onModifiedChanged: updateSaveEnabled()

				watchModification: true

				QGridLabel {
					field: textServerName
				}

				QGridTextField {
					id: textServerName
					fieldName: qsTr("Szerver neve")
					sqlField: "serverName"

					validator: RegExpValidator { regExp: /.+/ }
				}

			}
		}

		QCollapsible {
			title: qsTr("Regisztráció")

			QGridLayout {
				id: grid4

				enabled: !serverSettings.isBusy

				onModifiedChanged: updateSaveEnabled()

				watchModification: true

				/*QGridText {
								field: comboRegistrationAuto
								text: qsTr("Automatikus regisztráció:")
							}

							QGridComboBox {
								id: comboRegistrationAuto
								sqlField: "registration.auto"

								valueRole: "value"
								textRole: "text"

								model: [
									{value: "0", text: qsTr("Letiltva")},
									{value: "1", text: qsTr("Engedélyezve")},
								]
							}*/


				QGridText {
					field: comboRegistrationClass
					text: qsTr("Osztály kiválasztása:")
				}

				QGridComboBox {
					id: comboRegistrationClass
					sqlField: "registration.class"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "0", text: qsTr("Letiltva")},
						{value: "1", text: qsTr("Engedélyezve")},
					]
				}

				QGridText {
					field: comboDefaultClass
					text: qsTr("Alapértelmezett osztály:")
				}

				QGridComboBox {
					id: comboDefaultClass
					sqlField: "registration.defaultClass"

					valueRole: "value"
					textRole: "text"

					model: []
				}
			}
		}



		QCollapsible {
			title: qsTr("OAuth2")

			QGridLayout {
				id: grid5

				enabled: !serverSettings.isBusy

				onModifiedChanged: updateSaveEnabled()

				watchModification: true

				QGridLabel {
					field: textOAuth2Id
				}

				QGridTextField {
					id: textOAuth2Id
					fieldName: qsTr("Google Client ID")
					sqlField: "oauth2.googleID"
				}

				QGridLabel {
					field: textOAuth2Key
				}

				QGridTextField {
					id: textOAuth2Key
					fieldName: qsTr("Google Client Secret")
					sqlField: "oauth2.googleKey"
				}

				QGridText {
					field: comboOAuth2RegistrationAuto
					text: qsTr("Automatikus regisztráció:")
				}

				QGridComboBox {
					id: comboOAuth2RegistrationAuto
					sqlField: "oauth2.registration"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "0", text: qsTr("Letiltva")},
						{value: "1", text: qsTr("Engedélyezve")},
					]
				}

			}
		}
	}


	Connections {
		target: actionSave
		function onTriggered() {
			var o = JS.getModifiedSqlFields([
												textServerName,
												comboRegistrationClass,
												textOAuth2Id,
												textOAuth2Key,
												comboOAuth2RegistrationAuto,
												comboDefaultClass
											])

			if (Object.keys(o).length) {
				serverSettings.send("setSettings", o)
			}
		}
	}

	Connections {
		target: serverSettings

		function onGetSettings(jsonData, binaryData) {
			var newModel = []

			if (jsonData.classList) {
				for (var i=0; i<jsonData.classList.length; i++) {
					var c = jsonData.classList[i]
					newModel.push({value: String(c.id), text: c.name})
				}
			}

			newModel.push({value: String(-1), text: qsTr("-- Osztály nélkül --")})

			comboDefaultClass.model = newModel

			var dc = jsonData["registration.defaultClass"]

			if (!dc || dc < 1)
				jsonData["registration.defaultClass"] = -1

			JS.setSqlFields([
								textServerName,
								comboRegistrationClass,
								textOAuth2Id,
								textOAuth2Key,
								comboOAuth2RegistrationAuto,
								comboDefaultClass
							], jsonData)

			grid1.modified = false
			grid4.modified = false
			grid5.modified = false
		}

		function onSetSettings(jsonData, binaryData) {
			if (jsonData.success) {
				cosClient.sendMessageInfo(qsTr("Szerver beállítások"), qsTr("A szerver beállításai sikeresen módosultak."))
				serverSettings.send("getSettings")
			} else {
				cosClient.sendMessageWarning(qsTr("Szerver beállítások"), qsTr("Nem sikerült módosítani a szerver beállításait."))
			}
		}
	}

	Component.onCompleted: serverSettings.send("getSettings")


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		//text: qsTr("Mentés")
		enabled: false
		shortcut: "Ctrl+S"
	}


	function updateSaveEnabled() {
		actionSave.enabled = grid1.modified || grid4.modified || grid5.modified
	}


	backCallbackFunction: function () {
		if (_closeEnabled)
			return false

		if (actionSave.enabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				_closeEnabled = true
				mainStack.back()
			})
			d.open()
			return true
		}

		return false
	}


	closeCallbackFunction: function () {
		if (_closeEnabled)
			return false

		if (actionSave.enabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				_closeEnabled = true
				mainWindow.close()
			})
			d.open()
			return true
		}

		return false
	}
}
