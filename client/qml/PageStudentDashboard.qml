import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

QPage {
	id: control

	signal userLoaded()

	stackPopFunction: function() {
		if (_closeEnabled || (Client.server && Client.server.user.loginState != User.LoggedIn))
			return true

		if (_stack.currentItem.stackPopFunction) {
			if (!_stack.currentItem.stackPopFunction())
				return false
		}

		if (tabBar.currentIndex !== 2) {
			tabBar.currentIndex = 2
			return false
		}

		JS.questionDialog(
					{
						onAccepted: function()
						{
							_closeEnabled = true
							if (Client.server && Client.server.user.loginState == User.LoggedIn)
								Client.httpConnection.close()
							else
								Client.stackPop(control)
						},
						text: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?"),
						title: qsTr("Kilépés"),
						iconSource: Qaterial.Icons.closeCircle
					})


		return false
	}

	header: null

	property StudentGroupList groupList: Client.cache("studentGroupList")
	property StudentGroup group: null
	property StudentMapHandler studentMapHandler: StudentMapHandler {  }

	property bool _closeEnabled: false

	Qaterial.StackView {
		id: _stack
		anchors.fill: parent
	}

	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.httpConnection.pending
	}

	footer: QTabBar
	{
		id: tabBar

		onCurrentIndexChanged: {
			if (currentIndex < 0)
				return

			_stack.replace(model.get(currentIndex).cmp)
		}

		Component.onCompleted: {
			model.append({ text: qsTr("Kihívások"), source: Qaterial.Icons.trophyBroken, color: Qaterial.Colors.pink300, cmp: cmpCampaign })
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.fileDocumentMultiple, color: Qaterial.Colors.red400, cmp: cmpExam })
			model.append({ text: qsTr("Áttekintés"), source: Qaterial.Icons.speedometer, cmp: cmpDashboard })
			model.append({ text: qsTr("Call Pass"), source: "qrc:/internal/img/passIcon.svg", color: Qaterial.Colors.lightBlue400, cmp: cmpPass })
			model.append({ text: qsTr("Rangsor"), source: Qaterial.Icons.podium, color: Qaterial.Colors.green400, cmp: cmpScoreList })

			currentIndex = 2
		}
	}




	Component {
		id: cmpCampaign

		StudentGroupCampaignList {
			id: _btn
			group: control.group
			groupList: control.groupList
			mapHandler: control.studentMapHandler
			onChangeGroup: group => control.group = group
		}
	}

	Component {
		id: cmpExam

		StudentGroupExamList {
			id: _btn
			group: control.group
			groupList: control.groupList
			mapHandler: control.studentMapHandler
			onChangeGroup: group => control.group = group
		}
	}


	Component {
		id: cmpDashboard

		StudentDashboard {
			studentMapHandler: control.studentMapHandler
		}
	}

	Component {
		id: cmpPass

		StudentGroupPassList {
			id: _btn
			group: control.group
			groupList: control.groupList
			mapHandler: control.studentMapHandler
			onChangeGroup: group => control.group = group
		}
	}



	Component {
		id: cmpScoreList

		QItemGradient {
			id: _pageScoreList

			ScoreList {
				id: _scoreList
				anchors.fill: parent
				paddingTop: _pageScoreList.paddingTop
			}

			appBar.rightComponent: Row {
				Qaterial.AppBarButton {
					id: _filterButton
					anchors.verticalCenter: parent.verticalCenter
					icon.source: _scoreList.filterClassId === -1 ? Qaterial.Icons.filter : Qaterial.Icons.filterCheck
					foregroundColor: _scoreList.filterClassId === -1 ? Qaterial.Style.primaryTextColor() : Qaterial.Style.accentColor

					ToolTip.text: qsTr("Szűrés")

					onClicked: _scoreList.selectFilter(_filterButton)
				}
				Qaterial.AppBarButton {
					anchors.verticalCenter: parent.verticalCenter
					icon.source: Qaterial.Icons.refresh
					ToolTip.text: qsTr("Frissítés")

					onClicked: _scoreList.reload()
				}
				Qaterial.AppBarButton {
					anchors.verticalCenter: parent.verticalCenter
					icon.source: Qaterial.Icons.medal
					ToolTip.text: qsTr("Ranglista")

					onClicked: Client.stackPushPage("Ranks.qml")
				}
			}


			StackView.onActivated: {
				Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentScore)
				_scoreList.reload()
			}

			StackView.onDeactivating: {
				Client.contextHelper.unsetContext(ContextHelperData.ContextStudentScore)
			}
		}
	}


	StackView.onActivated: {
		Client.reloadUser()
		Client.reloadCache("studentGroupList", control, function() {
			if (!control.group && control.groupList.count > 0)
				control.group = control.groupList.get(0)
		})
		Client.reloadCache("studentCampaignList")
		Client.reloadCache("classList")
		Client.reloadCache("passList")
		studentMapHandler.reload()
	}
}

