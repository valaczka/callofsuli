import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSwipePage {
	id: page

	defaultTitle: qsTr("Szerver beállításai")

	mainToolBarComponent: QToolButton { action: actionSave }

	activity: ServerSettings {
		id: serverSettings
	}

	content: [
		QSwipeContainer {
			id: container1
			reparented: swipeMode
			reparentedParent: placeholder1
			title: qsTr("Általános")
			icon: CosStyle.iconSetup

			QAccordion {
				QCollapsible {
					title: qsTr("Általános")

					QGridLayout {
						id: grid1
						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified || grid4.modified

						width: parent.width
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

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified || grid4.modified

						width: parent.width
						watchModification: true

						QGridText {
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
						}


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
					}
				}

			}

		}
		,
		QSwipeContainer {
			id: container2
			reparented: swipeMode
			reparentedParent: placeholder2
			title: qsTr("E-mail")
			icon: CosStyle.iconEmail

			QAccordion {
				QCollapsible {
					title: qsTr("E-mail funkciók")

					QGridLayout {
						id: grid3

						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified || grid4.modified

						width: parent.width
						watchModification: true

						QGridText {
							field: comboRegistration
							text: qsTr("Emailes regisztráció:")
						}

						QGridComboBox {
							id: comboRegistration
							sqlField: "email.registration"

							valueRole: "value"
							textRole: "text"

							model: [
								{value: "0", text: qsTr("Letiltva")},
								{value: "1", text: qsTr("Engedélyezve")},
							]
						}


						QGridLabel { field: textDomain }

						QGridTextField {
							id: textDomain

							enabled: comboRegistration.currentValue === "1"
							fieldName: qsTr("Domain korlátozás")
							placeholderText: qsTr("Domain korlátozás vesszővel elválasztva (pl: gmail.com, freemail.com)")
							sqlField: "email.registrationDomains"
						}


						QGridText {
							field: comboReset
							text: qsTr("Jelszó helyreállítás:")
						}

						QGridComboBox {
							id: comboReset
							sqlField: "email.passwordReset"

							valueRole: "value"
							textRole: "text"

							model: [
								{value: "0", text: qsTr("Letiltva")},
								{value: "1", text: qsTr("Engedélyezve")},
							]
						}
					}
				}


				QCollapsible {
					title: qsTr("SMTP szerver")

					QGridLayout {
						id: grid2

						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified || grid4.modified

						width: parent.width
						watchModification: true


						QGridLabel { field: textHost }

						QGridTextField {
							id: textHost
							fieldName: qsTr("Host")
							sqlField: "smtp.server"

						}

						QGridLabel { field: textPort }

						QGridTextField {
							id: textPort
							fieldName: qsTr("Port")
							sqlField: "smtp.port"

							validator: IntValidator { bottom: 0; top: 65535 }

						}

						QGridText {
							field: comboType
							text: qsTr("Biztonság típusa:")
						}

						QGridComboBox {
							id: comboType
							sqlField: "smtp.type"

							valueRole: "value"
							textRole: "text"

							model: [
								{value: "0", text: qsTr("Nincs titkosítás")},
								{value: "1", text: qsTr("SSL titkosítás")},
								{value: "2", text: qsTr("TLS titkosítás")}
							]
						}

						QGridLabel { field: textEmail }

						QGridTextField {
							id: textEmail
							fieldName: qsTr("Email cím")
							sqlField: "smtp.email"

							validator: RegExpValidator { regExp: /^((.+@..+\...+)|)$/ }

						}


						QGridLabel { field: textUser }

						QGridTextField {
							id: textUser
							fieldName: qsTr("Felhasználó")
							sqlField: "smtp.user"

						}


						QGridLabel { field: textPassword }

						QGridTextField {
							id: textPassword
							fieldName: qsTr("Jelszó")
							sqlField: "smtp.password"

							echoMode: TextInput.Password

						}

					}

				}

			}
		}
	]

	swipeContent: [
		Item { id: placeholder1 },
		Item { id: placeholder2 }
	]

	tabBarContent: [
		QSwipeButton { swipeContainer: container1 },
		QSwipeButton { swipeContainer: container2 }
	]


	Connections {
		target: actionSave
		function onTriggered() {
			var o = JS.getModifiedSqlFields([
												textServerName,
												textHost,
												textPort,
												comboType,
												textEmail,
												textUser,
												textPassword,
												comboRegistration,
												textDomain,
												comboReset,
												comboRegistrationAuto,
												comboRegistrationClass
											])

			if (Object.keys(o).length) {
				serverSettings.send("setSettings", o)
			}
		}
	}

	Connections {
		target: serverSettings

		function onGetSettings(jsonData, binaryData) {
			JS.setSqlFields([
								textServerName,
								textHost,
								textPort,
								comboType,
								textEmail,
								textUser,
								textPassword,
								comboRegistration,
								textDomain,
								comboReset,
								comboRegistrationAuto,
								comboRegistrationClass
							], jsonData)

			grid1.modified = false
			grid2.modified = false
			grid3.modified = false
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
		text: qsTr("Mentés")
		enabled: false
		shortcut: "Ctrl+S"
	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}

}
