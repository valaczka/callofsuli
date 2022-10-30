import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property Servers servers: null
	property alias profile: profile

	property bool loginTried: false

	property bool _closeEnabled: false

	buttonModel: modelGuest

	activity: Profile {
		id: profile
	}


	ListModel {
		id: modelGuest

		ListElement {
			title: qsTr("Regisztráció")
			icon: "image://font/Academic/\uf184"
			iconColor: "darkorange"
			func: function() { replaceContent(componentRegistration) }
		}
		ListElement {
			title: qsTr("Bejelentkezés")
			icon: "image://font/Academic/\uf207"
			iconColor: "chartreuse"
			func: function() { replaceContent(componentLogin) }
			checked: true
		}
		ListElement {
			title: qsTr("Rangsor")
			icon: "qrc:/internal/icon/podium.svg"
			iconColor: "royalblue"
			func: function() { replaceContent(componentScore) }
		}
	}



	ListModel {
		id: modelStudent

		ListElement {
			title: qsTr("Profil")
			icon: "qrc:/internal/icon/account.svg"
			iconColor: "bisque"
			func: function() { replaceContent(componentProfile, { menuVisible: true }) }
		}
		ListElement {
			title: qsTr("Áttekintés")
			icon: "qrc:/internal/icon/speedometer.svg"
			iconColor: "gold"
			func: function() { replaceContent(componentGroups) }
			checked: true
			raised: true
		}
		ListElement {
			title: qsTr("Rangsor")
			icon: "qrc:/internal/icon/podium.svg"
			iconColor: "royalblue"
			func: function() { replaceContent(componentScore) }
		}
	}




	ListModel {
		id: modelTeacher

		ListElement {
			title: qsTr("Profil")
			icon: "qrc:/internal/icon/account-tie.svg"
			iconColor: "bisque"
			func: function() { replaceContent(componentProfile, { menuVisible: true }) }
		}
		ListElement {
			title: qsTr("Áttekintés")
			icon: "qrc:/internal/icon/speedometer.svg"
			iconColor: "gold"
			func: function() { replaceContent(componentGroups) }
			checked: true
			raised: true
		}
		ListElement {
			title: qsTr("Rangsor")
			icon: "qrc:/internal/icon/podium.svg"
			iconColor: "royalblue"
			func: function() { replaceContent(componentScore) }
		}
	}


	ListModel {
		id: modelAdmin

		ListElement {
			title: qsTr("Profil")
			icon: "qrc:/internal/icon/account-hard-hat.svg"
			iconColor: "bisque"
			func: function() { replaceContent(componentProfile, { menuVisible: true }) }
		}
		ListElement {
			title: qsTr("Áttekintés")
			icon: "qrc:/internal/icon/speedometer.svg"
			iconColor: "gold"
			func: function() { replaceContent(componentGroups) }
			checked: true
		}
		ListElement {
			title: qsTr("Rangsor")
			icon: "qrc:/internal/icon/podium.svg"
			iconColor: "royalblue"
			func: function() { replaceContent(componentScore) }
		}
		ListElement {
			title: qsTr("Beállítások")
			icon: "qrc:/internal/icon/cog-outline.svg"
			iconColor: "#ff4f38"
			func: function() { replaceContent(componentSettings) }
		}
	}


	Component {
		id: componentGroups
		Groups {
			profile: control.profile
		}
	}

	Component {
		id: componentRegistration
		Registration {
			servers: control.servers
		}
	}

	Component {
		id: componentLogin
		Login {
			servers: control.servers
		}
	}


	Component {
		id: componentScore
		ScoreList {
			onUserSelected: {
				if (!(cosClient.userRoles & Client.RoleGuest))
					pushContent(componentProfile, { username: user })
			}
		}
	}

	Component {
		id: componentProfile
		ProfileEditor {
			profile: control.profile
		}
	}

	Component {
		id: componentSettings
		Settings { }
	}

	Connections {
		target: cosClient
		function onUserRolesChanged(userRole) {
			if (control.StackView.view)
				mainStack.pop(control)

			checkRoles()
		}

		function onRegistrationRequest(oauth2, code) {
			if (control.StackView.view)
				mainStack.pop(control)

			control.replaceContent(componentRegistration, {
									   autoRegisterGoogle: oauth2,
									   code: code
								   })
		}
	}

	StackView.onRemoved: {
		cosClient.closeConnection()
	}



	Component.onCompleted: checkRoles()


	function checkRoles() {
		if (cosClient.userRoles & Client.RoleAdmin) {
			if (buttonModel !== modelAdmin)
				replaceContent()
			buttonModel = modelAdmin
			title = ""
			//buttonColor = CosStyle.colorPrimaryLight
			buttonBackgroundColor = "#5e0000"
		} else if (cosClient.userRoles & Client.RoleTeacher) {
			if (buttonModel !== modelTeacher)
				replaceContent()
			buttonModel = modelTeacher
			title = ""
			//buttonColor = CosStyle.colorPrimaryLighter
			buttonBackgroundColor = "#33220c"
		} else if (cosClient.userRoles & Client.RoleStudent) {
			if (buttonModel !== modelStudent)
				replaceContent()
			buttonModel = modelStudent
			title = ""
			//buttonColor = CosStyle.colorPrimary
			buttonBackgroundColor = "#111147"
		} else {
			if (buttonModel !== modelGuest)
				replaceContent()
			buttonModel = modelGuest
			title = cosClient.serverName
			//buttonColor = CosStyle.colorPrimaryDark
			buttonBackgroundColor = "black"
		}
	}


	pageBackCallbackFunction: function () {
		if (_closeEnabled)
			return false


		for (var i=0; i<buttonModel.count; i++) {
			var o = buttonModel.get(i)
			var b = buttons[i]

			if (o.checked && b && !b.checked) {
				b.checked = true
				return true
			}
		}

		var d = JS.dialogCreateQml("YesNo", {
									   text: qsTr("Biztosan lezárod a szerverrel a kapcsolatot?"),
									   image: "qrc:/internal/icon/lan-disconnect.svg"
								   })
		d.accepted.connect(function() {
			_closeEnabled = true
			mainStack.back()
		})
		d.open()
		return true

	}
}
