import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	/*stackPopFunction: function() {
		var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}*/

	property TeacherExam teacherExam: null
	readonly property Exam _exam: teacherExam ? teacherExam.exam : null
	property var userList: []
	property int count: 0

	title: _exam ? (_exam.description != "" ? _exam.description : qsTr("Dolgozat #%1").arg(_exam.examId)) : ""
	subtitle: teacherExam && teacherExam.teacherGroup ? teacherExam.teacherGroup.fullName : ""

	appBar.backButtonVisible: true


	property var _fullList: []
	property int _itemLeft: 0

	Grid {
		id: _grid
		anchors.fill: parent

		flow: Grid.TopToBottom

		columns: {
			if (root.height > root.width)
				return root.count > 20 ? 2 : 1
			else if (root.count > 10)
				return 3
			else
				return 2
		}

		rows: Math.ceil(root.count/columns)

		Repeater {
			model: ListModel {
				id: _model
			}

			delegate: Rectangle {
				width: _grid.width/_grid.columns
				height: _grid.height/_grid.rows

				visible: false

				color: "transparent"

				property bool showFinal: false
				property string tmpName: ""
				property int step: 0
				property color textColor: Qaterial.Style.accentColor

				onStepChanged: {
					if (_fullList.length)
						tmpName = _fullList[Math.floor(Math.random() * _fullList.length)]
				}

				Qaterial.Label {
					anchors.fill: parent
					text: showFinal ? name : tmpName

					font.family: Qaterial.Style.textTheme.headline1.family
					font.pixelSize: Math.min(85*Qaterial.Style.pixelSizeRatio, parent.height*0.8/2)
					font.weight: Font.Bold
					lineHeight: 0.8

					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					wrapMode: Text.Wrap

					color: textColor

					leftPadding: 10
					rightPadding: 10
					topPadding: 5
					bottomPadding: 5
				}

				Component.onCompleted: visible = true
			}
		}

		add: Transition {
			id: _transition
			SequentialAnimation {
				PauseAnimation {
					duration: (_transition.ViewTransition.index -
							   _transition.ViewTransition.targetIndexes[0]) * Math.min(4000/root.count, 550)
				}

				ParallelAnimation {
					ColorAnimation {
						from: "transparent"
						to: Qaterial.Style.primaryColorDark
						duration: 200
						property: "color"
					}

					NumberAnimation {
						property: "step"
						duration: 750
						easing.type: Easing.InOutQuad
						from: 0
						to: 12
					}
				}

				PropertyAction {
					property: "showFinal"
					value: true
				}

				ScriptAction {
					script: {
						if (_itemLeft>0)
							--_itemLeft
					}
				}

				ParallelAnimation {
					ColorAnimation {
						to: Qaterial.Colors.black
						duration: 400
						property: "color"
					}

					ColorAnimation {
						to: Qaterial.Style.iconColor()
						duration: 750
						property: "textColor"
					}

				}
			}
		}

	}


	on_ItemLeftChanged: {
		if (_itemLeft == 0) {
			Client.sound.stopSound("", Sound.SfxChannel)
			Client.sound.playSound("qrc:/sound/sfx/win.mp3", Sound.SfxChannel)
		}

	}

	Connections {
		target: teacherExam

		function onVirtualListPicked(list) {
			_itemLeft = list.length
			for (let i=0; i<list.length; ++i) {
				let u=list[i]
				_model.append({name: u.fullName})
			}

			Client.sound.playSound("qrc:/sound/menu/drum.mp3", Sound.SfxChannel)
		}
	}

	StackView.onActivated: if (teacherExam) teacherExam.pickUsers(userList, count)
	StackView.onDeactivated: {
		if (teacherExam)
			teacherExam.finish()

		Client.sound.stopSound("", Sound.SfxChannel)
	}

	Component.onCompleted: {
		if (!teacherExam || !teacherExam.teacherGroup || !teacherExam.teacherGroup)
			return

		for (let i=0; i<teacherExam.teacherGroup.memberList.count; ++i) {
			let u=teacherExam.teacherGroup.memberList.get(i)
			_fullList.push(u.fullName)
		}

	}
}
