import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Profil")
	icon: "qrc:/internal/icon/account.svg"

	property string username: cosClient.userName
	property bool forcedEdit: false
	property bool menuVisible: false

	property Profile profile: null

	property ListModel modelRankList: ListModel {}

	menu: menuVisible ? menuOwn : null
	action: (!collapsibleBase.modificationEnabled && (username === cosClient.userName || forcedEdit)) || layout1.modified ? actionEdit : null

	QMenu {
		id: menuOwn
		MenuItem {
			action: actionChangePassword
		}

		MenuItem {
			text: qsTr("Kijelentkezés")
			icon.source: CosStyle.iconLogout

			onClicked: cosClient.logout()
		}
	}

	QAccordion {
		id: accordion

		ProfileDetailsUser {
			id: user

			topPadding: control.tabPage.headerPadding
			anchors.horizontalCenter: parent.horizontalCenter
			width: parent.width
		}



		QCollapsible {
			id: collapsibleBase
			title: qsTr("Alapadatok")
			backgroundColor: "transparent"

			visible: username === cosClient.userName || (cosClient.userRoles & Client.RoleTeacher) || forcedEdit

			property bool modificationEnabled: false

			QGridLayout {
				id: layout1

				watchModification: true
				columns: 2
				/*implicitWidth: accordion.width
				width: collapsibleBase.width-50*/

				acceptable: (!textFirstname.enabled || textFirstname.acceptableInput)
							&& (!textLastname.enabled || textLastname.acceptableInput)
							&& textNickname.acceptableInput

				onAccepted: save()

				QGridText {
					text: qsTr("Felhasználó")
				}

				QGridTextField {
					readOnly: true
					text: username
				}

				QGridText {
					text: qsTr("Osztály")
				}

				QGridTextField {
					id: textClassname
					readOnly: true
					sqlField: "classname"
				}

				QGridLabel {
					field: textFirstname
				}

				QGridTextField {
					id: textFirstname
					fieldName: qsTr("Vezetéknév")
					sqlField: "firstname"
					placeholderText: collapsibleBase.modificationEnabled ? qsTr("Adj meg vezetéknevet") : ""
					readOnly: !collapsibleBase.modificationEnabled

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textLastname }

				QGridTextField {
					id: textLastname
					fieldName: qsTr("Keresztnév")
					sqlField: "lastname"
					placeholderText: collapsibleBase.modificationEnabled ? qsTr("Adj meg keresztnevet") : ""
					readOnly: !collapsibleBase.modificationEnabled

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textNickname }

				QGridTextField {
					id: textNickname
					fieldName: qsTr("Becenév")
					sqlField: "nickname"
					placeholderText: collapsibleBase.modificationEnabled ? qsTr("Adj meg becenevet") : ""
					readOnly: !collapsibleBase.modificationEnabled
				}



				QGridText {
					text: qsTr("Karakter")
				}

				QGridImageSpinBox {
					id: spinCharacter
					from: 0
					to: profile.characterList.length-1
					sqlField: "character"
					sqlData: profile.characterList[value].dir

					enabled: collapsibleBase.modificationEnabled

					imageSize: CosStyle.pixelSize*3

					textFromValue: function(value) {
						return profile.characterList[value].name
					}

					imageFromValue: function(value) {
						return "qrc:/character/%1/thumbnail.png".arg(profile.characterList[value].dir)
					}

					function setData(t) {
						value = profile.findCharacter(t)
						modified = false
					}

				}


			}
		}


		QCollapsible {
			id: collapsibleRankList
			title: qsTr("Előléptetések")
			backgroundColor: "transparent"
			collapsed: true

			QObjectListView {
				id: rankList
				width: parent.width

				mouseAreaEnabled: false

				model: SortFilterProxyModel {
					sourceModel: modelRankList

					sorters: [
						StringSorter {
							roleName: "timestamp"
							sortOrder: Qt.DescendingOrder
							priority: 3
						},
						RoleSorter {
							roleName: "rankid"
							sortOrder: Qt.DescendingOrder
							priority: 2
						},
						RoleSorter {
							roleName: "level"
							sortOrder: Qt.DescendingOrder
							priority: 1
						}
					]
				}

				autoSelectorChange: false
				highlightCurrentItem: false

				delegateHeight: CosStyle.halfLineHeight

				leftComponent: Image {
					source: model && model.maxStreak === -1 ? cosClient.rankImageSource(model.rankid, model.level, model.image) : ""
					width: rankList.delegateHeight+10
					height: rankList.delegateHeight*0.8
					fillMode: Image.PreserveAspectFit
				}

				rightComponent: QLabel {
					text: model && model.maxStreak === -1 ? "%1 XP".arg(Number(model.xp).toLocaleString()) : ""
					font.weight: Font.Normal
					font.pixelSize: rankList.delegateHeight*0.8
					color: CosStyle.colorAccentLight
					leftPadding: 5
				}

				contentComponent: Row {
					spacing: 10

					QLabel {
						visible: model && model.maxStreak !== -1
						width: Math.min(implicitWidth, parent.width/2)
						elide: Text.ElideRight
						anchors.verticalCenter: parent.verticalCenter
						font.pixelSize: CosStyle.pixelSize*0.9
						font.weight: Font.DemiBold
						color: CosStyle.colorOKLighter
						text: model ? qsTr("Új széria: %1").arg(model.maxStreak) : ""
					}

					QLabel {
						visible: model && model.maxStreak === -1
						anchors.verticalCenter: parent.verticalCenter
						width: Math.min(implicitWidth, parent.width/2)
						elide: Text.ElideRight
						font.pixelSize: CosStyle.pixelSize*0.9
						font.weight: Font.Medium
						color: CosStyle.colorPrimaryLight
						text: model ? qsTr("%1 (level %2)").arg(model.name).arg(model.level) : ""
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						width: Math.min(implicitWidth, parent.width/2)
						elide: Text.ElideRight
						leftPadding: 5
						font.pixelSize: CosStyle.pixelSize*0.8
						font.weight: Font.Normal
						color: model && model.maxStreak !== -1 ? CosStyle.colorOKLighter : CosStyle.colorPrimaryLight
						text: model ? JS.readableTimestamp(model.timestamp) : ""
					}
				}

			}
		}


		QCollapsible {
			title: qsTr("Megszerzett trófeák (%1 db)").arg(trophies.trophyCount)
			backgroundColor: "transparent"

			ProfileDetailsTrophies {
				id: trophies

				imageSize: CosStyle.pixelSize*3.5

				anchors.horizontalCenter: parent.horizontalCenter
			}
		}
	}


	Action {
		shortcut: "Ctrl+S"
		onTriggered: {
			if (collapsibleBase.modificationEnabled && layout1.acceptable)
				save()
		}
	}


	Action {
		id: actionEdit

		text: qsTr("")

		enabled: !collapsibleBase.modificationEnabled || layout1.acceptable
		icon.source: collapsibleBase.modificationEnabled ? "qrc:/internal/icon/content-save.svg" : "qrc:/internal/icon/account-edit.svg"
		onTriggered: {
			if (collapsibleBase.modificationEnabled) {
				save()
			} else {
				collapsibleBase.modificationEnabled = true
			}
		}
	}

	Action {
		id: actionChangePassword
		text: qsTr("Jelszó változtatás")
		icon.source: "qrc:/internal/icon/shield-edit.svg"

		onTriggered: {
			var d = JS.dialogCreateQml("PasswordChange", { username: cosClient.userName })

			d.accepted.connect(function(data) {
				if (data === 1) {
					var pwd = d.item.password
					var opwd = d.item.oldPassword

					if (cosClient.userRoles & Client.RoleTeacher)
						profile.send(CosMessage.ClassTeacher, "userPasswordChange", {oldPassword: opwd, password: pwd})
					else
						profile.send(CosMessage.ClassStudent, "userPasswordChange", {oldPassword: opwd, password: pwd})
				}
			})
			d.open()
		}
	}


	onPopulated: reloadData()


	Connections {
		target: profile

		function onUserGet(jsonData, binaryData) {
			if (jsonData.character.length === 0)
				jsonData.character = "default"

			user.userName = (jsonData.nickname && jsonData.nickname !== "" ?
								 jsonData.nickname :
								 jsonData.firstname+" "+jsonData.lastname)

			user.picture = jsonData.picture
			user.rankId = Number(jsonData.rankid)
			user.rankLevel = Number(jsonData.ranklevel)
			user.rankImage = jsonData.rankimage
			user.rankName = jsonData.rankname


			if (!forcedEdit && (jsonData.nameModificationDisabled || jsonData.oauth2Account)) {
				textFirstname.readOnly = true
				textLastname.readOnly = true
			}

			if (jsonData.oauth2Account && !forcedEdit)
				actionChangePassword.enabled = false
			else
				actionChangePassword.enabled = true


			JS.setSqlFields([
								textFirstname,
								textLastname,
								textNickname,
								spinCharacter,
								textClassname
							], jsonData)

			trophies.t1 = Number(jsonData.t1)
			trophies.t2 = Number(jsonData.t2)
			trophies.t3 = Number(jsonData.t3)
			trophies.d1 = Number(jsonData.d1)
			trophies.d2 = Number(jsonData.d2)
			trophies.d3 = Number(jsonData.d3)

			if (jsonData.ranklog) {
				JS.listModelReplace(modelRankList, jsonData.ranklog)
				collapsibleRankList.collapsed = false
			} else {
				modelRankList.clear()
				collapsibleRankList.collapsed = true
			}
		}


		function onUserModify(jsonData, binaryData) {
			if (jsonData.error === undefined) {
				reloadData()
				layout1.modified = false
				collapsibleBase.modificationEnabled = false
			}
		}


		function onUserPasswordChange(jsonData, binaryData) {
			if (jsonData.error && jsonData.error.length) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/shield-alert.svg", qsTr("Jelszó változtatás"), qsTr("A jelszó változtatás sikertelen"), jsonData.error)
			} else {
				cosClient.sendMessageInfo(qsTr("Jelszó változtatás"), qsTr("A jelszót sikeresen megváltozott"))
			}
		}
	}


	function save() {
		var o = JS.getModifiedSqlFields([
											textFirstname,
											textLastname,
											textNickname,
											spinCharacter
										])

		if (username === cosClient.userName && Object.keys(o).length) {
			if (cosClient.userRoles & Client.RoleTeacher)
				profile.send(CosMessage.ClassTeacher, "userModify", o)
			else
				profile.send(CosMessage.ClassStudent, "userModify", o)
		}
	}



	function reloadData() {
		if (cosClient.userRoles & Client.RoleTeacher)
			profile.send(CosMessage.ClassTeacher, "userGet", {
							 username: username,
							 withRanklog: true,
							 withTrophy: true
						 })
		else
			profile.send(CosMessage.ClassStudent, "userGet", {
							 username: username,
							 withRanklog: true,
							 withTrophy: true
						 })
	}


	backCallbackFunction: function () {
		if (collapsibleBase.modificationEnabled) {
			collapsibleBase.modificationEnabled = false
			reloadData()
			return true
		}

		return false
	}
}
