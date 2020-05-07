import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property bool isBusy: false

	header: QToolBar {
		id: toolbar

		title: qsTr("Szerver beállításai")

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		Row {
			Layout.fillHeight: true
			QToolBusyIndicator { running: page.isBusy }
			QToolButton { action: actionSave }
		}
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPagePanel {
		id: p

		anchors.fill: parent
		maximumWidth: 800

		blurSource: bgImage

		QAccordion {
			id: accordion

			anchors.fill: parent

			QCollapsible {
				title: qsTr("Általános")

				QGridLayout {
					id: grid1
					enabled: !page.isBusy

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

					enabled: !page.isBusy

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
						fieldName: qsTr("port")
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

					enabled: !page.isBusy

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

	}


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		text: qsTr("Mentés")
		enabled: grid1.modified || grid2.modified || grid3.modified
		shortcut: "Ctrl+S"
		onTriggered: {
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
				isBusy = true
				o["class"] = "userInfo"
				o["func"] = "setSettings"
				cosClient.socketSend(o)
			}
		}
	}

	Connections {
		target: cosClient

		onSettingsLoaded: {
			isBusy = false
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
							], data)

			grid1.modified = false
			grid2.modified = false
			grid3.modified = false
		}

		onSettingsSuccess: {
			isBusy = false
			cosClient.sendMessageInfo(qsTr("Szerver beállítások"), qsTr("A szerver beállításai sikeresen módosultak."))
			getSettings()
		}

		onSettingsError: {
			isBusy = false
			cosClient.sendMessageWarning(qsTr("Szerver beállítások"), qsTr("Nem sikerült módosítani a szerver beállításait."))
		}
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		getSettings()
	}

	function getSettings() {
		isBusy = true
		cosClient.socketSend({"class": "userInfo", "func": "getSettings"})
	}


	function windowClose() {
		return true
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

		return false
	}
}
