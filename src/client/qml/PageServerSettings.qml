import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	requiredPanelWidth: 700

	title: qsTr("Szerver beállításai")

	mainToolBarComponent: QToolButton { action: actionSave }

	activity: ServerSettings {
		id: serverSettings
	}


	panelComponents: Component {
		QPagePanel {
			id: panel

			maximumWidth: 600

			layoutFillWidth: true
			panelVisible: true

			QAccordion {
				id: accordion

				anchors.fill: parent

				QCollapsible {
					title: qsTr("Általános")

					QGridLayout {
						id: grid1
						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified

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
					title: qsTr("SMTP szerver")

					QGridLayout {
						id: grid2

						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified

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

				QCollapsible {
					title: qsTr("E-mail funkciók")

					QGridLayout {
						id: grid3

						enabled: !serverSettings.isBusy

						onModifiedChanged: actionSave.enabled = grid1.modified || grid2.modified || grid3.modified

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
			}




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
														comboReset
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
										comboReset
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
		}

	}



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
