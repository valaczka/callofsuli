import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: control
	title: qsTr("Profil")
	icon: CosStyle.iconUser

	property bool forceFullName: false
	property bool canEdit: true
	property color detailsColor: CosStyle.colorAccentLighter
	property real detailsSize: CosStyle.pixelSize*2.2
	property int detailsFontWeight: Font.Light

	property int fieldNameWidth: Math.min(width*0.4, 350)

	property bool modificationEnabled: false
	readonly property bool acceptable: layout1.acceptable && layout2.acceptable && layout3.acceptable
	readonly property bool modified: layout1.modified || layout2.modified || layout3.modified

	signal accepted()

	property ListModel modelRankList: ListModel {}

	QAccordion {
		id: accordion

		visible: false

		ProfileDetailsUser {
			id: user
			width: parent.width
		}



		QCollapsible {
			title: qsTr("Alapadatok")

			QGridLayout {
				id: layout1

				watchModification: true

				acceptable: (!textFirstname.enabled || textFirstname.acceptableInput)
							&& (!textLastname.enabled || textLastname.acceptableInput)
							&& textNickname.acceptableInput

				onAccepted: control.accepted()

				QGridText {
					Layout.minimumWidth: fieldNameWidth
					text: qsTr("Felhasználó")
				}

				QGridTextField {
					readOnly: true
					text: cosClient.userName
				}

				QGridLabel {
					field: textFirstname
				}

				QGridTextField {
					id: textFirstname
					fieldName: qsTr("Vezetéknév")
					sqlField: "firstname"
					placeholderText: qsTr("Adj meg vezetéknevet")
					readOnly: !modificationEnabled

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textLastname }

				QGridTextField {
					id: textLastname
					fieldName: qsTr("Keresztnév")
					sqlField: "lastname"
					placeholderText: qsTr("Adj meg keresztnevet")
					readOnly: !modificationEnabled

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textNickname }

				QGridTextField {
					id: textNickname
					fieldName: qsTr("Becenév")
					sqlField: "nickname"
					placeholderText: qsTr("Adj meg becenevet")
					readOnly: !modificationEnabled
				}


				QGridButton {
					id: buttonPassword
					text: qsTr("Jelszó változtatás")
					icon.source: CosStyle.iconEdit
					display: AbstractButton.TextBesideIcon

					visible: false

					onClicked: {
						var d = JS.dialogCreateQml("PasswordChange", { username: cosClient.userName })

						d.accepted.connect(function(data) {
							if (data === 1) {
								var pwd = d.item.password
								var opwd = d.item.oldPassword
								profile.send("userPasswordChange", {oldPassword: opwd, password: pwd})
							}
						})
						d.open()
					}
				}

			}
		}

		QCollapsible {
			title: qsTr("Játék beállításai")

			QGridLayout {
				id: layout2
				watchModification: true

				onAccepted: control.accepted()

				QGridText {
					Layout.minimumWidth: fieldNameWidth
					text: qsTr("Karakter")
				}

				QGridImageSpinBox {
					id: spinCharacter
					from: 0
					to: profile.characterList.length-1
					sqlField: "character"
					sqlData: profile.characterList[value].dir

					enabled: modificationEnabled

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
			title: qsTr("Eredmények")

			QGridLayout {
				id: layout3

				onAccepted: control.accepted()

				QGridText {
					Layout.minimumWidth: fieldNameWidth
					text: qsTr("XP")
				}

				QGridText {
					id: labelXP
					sqlField: "xp"
					font.weight: detailsFontWeight
					color: detailsColor
					font.pixelSize: detailsSize
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					leftPadding: parent.columns > 1 ? 10 : 0

					function setData(t) {
						text = "%1 XP".arg(Number(t).toLocaleString())
					}
				}

				QGridText { text: qsTr("Streak") }

				QGridText {
					id: labelStreak
					sqlField: "currentStreak"
					font.weight: detailsFontWeight
					color: detailsColor
					font.pixelSize: detailsSize
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					leftPadding: parent.columns > 1 ? 10 : 0
				}

				QGridText { text: qsTr("Leghosszabb streak") }

				QGridText {
					id: labelLongestStreak
					sqlField: "longestStreak"
					font.weight: detailsFontWeight
					color: detailsColor
					font.pixelSize: detailsSize
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					leftPadding: parent.columns > 1 ? 10 : 0
				}

				QGridText { text: qsTr("Megszerzett trófeák") }

				ProfileDetailsTrophies {
					id: trophies

					Layout.fillWidth: true
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					Layout.bottomMargin: parent.columns === 1 ? 10 : 0

				}
			}
		}


		QCollapsible {
			title: qsTr("Ranglista")

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
						}
					]
				}

				autoSelectorChange: false
				highlightCurrentItem: false

				delegateHeight: CosStyle.halfLineHeight

				leftComponent: Image {
					source: model ? cosClient.rankImageSource(model.rankid, model.level, model.image) : ""
					width: rankList.delegateHeight+10
					height: rankList.delegateHeight*0.8
					fillMode: Image.PreserveAspectFit
				}

				rightComponent: QLabel {
					text: model ? "%1 XP".arg(Number(model.xp).toLocaleString()) : ""
					font.weight: Font.Normal
					font.pixelSize: rankList.delegateHeight*0.8
					color: CosStyle.colorAccentLight
					leftPadding: 5
				}

				contentComponent: Row {
					spacing: 10
					QLabel {
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
						color: CosStyle.colorPrimaryLight
						text: model ? model.timestamp : ""
					}
				}

			}
		}
	}



	Connections {
		target: profile

		function onUserGet(jsonData, binaryData) {
			accordion.visible = true

			if (jsonData.character.length === 0)
				jsonData.character = "default"

			user.image = cosClient.rankImageSource(jsonData.rankid, -1, jsonData.rankimage)

			user.username = jsonData.nickname.length && !forceFullName ? jsonData.nickname : jsonData.firstname+" "+jsonData.lastname
			user.rankname = jsonData.rankname+(jsonData.ranklevel > 0 ? qsTr(" (lvl %1)").arg(jsonData.ranklevel) : "")

			if (jsonData.nameModificationDisabled || jsonData.oauth2Account) {
				textFirstname.readOnly = true
				textLastname.readOnly = true
			}

			if (jsonData.oauth2Account)
				buttonPassword.visible = false
			else
				buttonPassword.visible = true


			JS.setSqlFields([
								textFirstname,
								textLastname,
								textNickname,
								spinCharacter,
								labelXP,
								labelStreak,
								labelLongestStreak
							], jsonData)

			trophies.t1 = jsonData.t1
			trophies.t2 = jsonData.t2
			trophies.t3 = jsonData.t3
			trophies.d1 = jsonData.d1
			trophies.d2 = jsonData.d2
			trophies.d3 = jsonData.d3

			if (jsonData.ranklog)
				JS.listModelReload(modelRankList, jsonData.ranklog)
			else
				modelRankList.clear()
		}


		function onUserModify(jsonData, binaryData) {
			if (jsonData.error === undefined) {
				profile.send("userGet", {
								 withTrophy: true,
								 withRanklog: true
							 })
				layout1.modified = false
				layout2.modified = false
				layout3.modified = false
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

		if (Object.keys(o).length) {
			profile.send("userModify", o)
		}
	}
}

