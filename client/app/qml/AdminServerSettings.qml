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
	icon: "qrc:/internal/icon/wrench.svg"
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

				QGridText {
					field: comboRegistration
					text: qsTr("Regisztráció engedélyezése")
				}

				QGridComboBox {
					id: comboRegistration
					sqlField: "registration.enabled"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "0", text: qsTr("Letiltva")},
						{value: "1", text: qsTr("Engedélyezve")},
					]
				}

				QGridText {
					field: comboForcedClass
					text: qsTr("Osztályba sorolás")
				}

				QGridComboBox {
					id: comboForcedClass
					sqlField: "registration.forced"

					valueRole: "value"
					textRole: "text"

					model: []
				}
			}
		}


		QCollapsible {
			title: qsTr("Hitelesítési kódok")

			QGridLayout {
				id: grid6
				enabled: !serverSettings.isBusy

				onModifiedChanged: updateSaveEnabled()

				watchModification: true

				QGridLabel {
					field: textClassCodes
				}

				QGridTextArea {
					id: textClassCodes
					fieldName: qsTr("Osztályok kódjai")
					minimumHeight: CosStyle.baseHeight*3
					readOnly: true
				}

				QGridButton {
					id: buttonRegenerateCodes
					text: qsTr("Újak generálása")
					icon.source: CosStyle.iconRefresh
					display: AbstractButton.TextBesideIcon

					onClicked: {
						buttonRegenerateCodes.enabled = false
						serverSettings.send("classRegistration", {refresh: true})
					}
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
					text: qsTr("Regisztráció Google fiókkal")
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

				QGridText {
					field: comboOAuth2RegistrationForced
					text: qsTr("Google fiókos regisztráció kényszerítése")
				}

				QGridComboBox {
					id: comboOAuth2RegistrationForced
					sqlField: "oauth2.forced"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "0", text: qsTr("Letiltva")},
						{value: "1", text: qsTr("Engedélyezve")},
					]
				}

				QGridLabel {
					field: textOAuth2Domains
				}

				QGridTextField {
					id: textOAuth2Domains
					fieldName: qsTr("Domain korlátozás")
					sqlField: "oauth2.domains"
				}

			}
		}
	}


	Connections {
		target: actionSave
		function onTriggered() {
			var o = JS.getModifiedSqlFields([
												textServerName,
												comboRegistration,
												textOAuth2Id,
												textOAuth2Key,
												comboOAuth2RegistrationAuto,
												comboOAuth2RegistrationForced,
												comboForcedClass,
												textOAuth2Domains
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

			newModel.push({value: String(-1), text: qsTr("-- Hitelesítési kód alapján --")})

			comboForcedClass.model = newModel

			var dc = jsonData["registration.forced"]

			if (!dc || dc < 1)
				jsonData["registration.forced"] = -1

			JS.setSqlFields([
								textServerName,
								comboRegistration,
								textOAuth2Id,
								textOAuth2Key,
								comboOAuth2RegistrationAuto,
								comboOAuth2RegistrationForced,
								comboForcedClass,
								textOAuth2Domains
							], jsonData)


			textClassCodes.clear()

			var t = ""

			if (jsonData.codeList) {
				for (i=0; i<jsonData.codeList.length; i++) {
					c = jsonData.codeList[i]

					if (c.classid > 0)
						t += c.name
					else
						t = qsTr("Osztály nélkül")

					t += ": "+c.code+"\n"
				}
			}

			textClassCodes.text = t

			grid1.modified = false
			grid4.modified = false
			grid5.modified = false
			grid6.modified = false
		}

		function onSetSettings(jsonData, binaryData) {
			if (jsonData.success) {
				//cosClient.sendMessageInfo(qsTr("Szerver beállítások"), qsTr("A szerver beállításai sikeresen módosultak."))
				serverSettings.send("getSettings")
			} else {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Szerver beállítások"), qsTr("Nem sikerült módosítani a szerver beállításait."))
			}
		}


		function onClassRegistration(jsonData, binaryData) {
			buttonRegenerateCodes.enabled = true
			if (jsonData.success) {
				//cosClient.sendMessageInfo(qsTr("Hitelesítési kódok"), qsTr("A hitelesítési kódok sikeresen módosultak."))
				serverSettings.send("getSettings")
			} else {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Hitelesítési kódok"), qsTr("Nem sikerült módosítani a hitelesítési kódokat."))
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
		actionSave.enabled = grid1.modified || grid4.modified || grid5.modified || grid6.modified
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
