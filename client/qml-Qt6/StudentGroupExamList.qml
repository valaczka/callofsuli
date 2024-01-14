import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS

Item
{
	id: control

	property StudentGroup group: null
	property StudentMapHandler mapHandler: null

	property real topPadding: 0

	property var stackPopFunction: function() {
		if (_scrResult.visible) {
			_scrResult.visible = false
			return false
		}

		return true
	}

	ExamList {
		id: _examList

		function reload() {
			if (!group)
				return

			Client.send(HttpConnection.ApiUser, "group/%1/exam".arg(group.groupid))
			.done(control, function(r){
				Client.callReloadHandler("exam", _examList, r.list)
			})
			.fail(control, JS.failMessage(qsTr("Dolgozatlista letöltése sikertelen")))
		}
	}

	QScrollable {
		id: _scrResult
		anchors.fill: parent

		visible: false

		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		contentCentered: true
		refreshEnabled: false

		GameTestResult {
			id: _testResult
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}

	QScrollable {
		id: _scrList
		anchors.fill: parent
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		visible: !_scrResult.visible

		refreshEnabled: true
		onRefreshRequest: _examList.reload()

		Qaterial.LabelHeadline5 {
			width: parent.width
			topPadding: 25
			leftPadding: 50
			rightPadding: 50
			horizontalAlignment: Qt.AlignHCenter
			text: group ? group.name : ""
			visible: group
		}

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: false

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: _examList

				sorters: [
					RoleSorter {
						roleName: "timestamp"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]
			}


			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate

				width: view.width

				property Exam exam: model.qtObject

				highlighted: ListView.isCurrentItem

				readonly property string iconSource: {
					switch (mode) {
					case Exam.ExamVirtual:
						return Qaterial.Icons.checkBold
					case Exam.ExamPaper:
						return Qaterial.Icons.playCircle
					default:
						return Qaterial.Icons.account
					}
				}

				readonly property color iconColor: {
					switch (mode) {
					case Exam.ExamVirtual:
						return Qaterial.Style.primaryTextColor()
					default:
						return Qaterial.Style.iconColor()
					}
				}

				textColor: iconColor

				//secondaryTextColor: state === Exam.Finished ?
				//						Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText


				text: description != "" ? description : qsTr("Dolgozat #%1").arg(examId)
				secondaryText: {
					if (timestamp.getTime()) {
						return timestamp.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm")
					}

					return ""
				}

				leftSourceComponent: Qaterial.RoundColorIcon
				{
					source: _delegate.iconSource
					color: _delegate.iconColor
					iconSize: Qaterial.Style.delegate.iconWidth

					fill: true
					width: roundIcon ? roundSize : iconSize
					height: roundIcon ? roundSize : iconSize
				}

				rightSourceComponent: Qaterial.LabelHeadline5 {
					visible: text != ""
					text: {
						if (_delegate.exam) {
							if (_delegate.exam.resultGrade)
								return _delegate.exam.resultGrade.shortname

							if (_delegate.exam.mode == Exam.ExamVirtual) {
								let ed = _delegate.exam.examData
								if (ed.length && ed[0].picked === true)
									return "X"
							}
						}

						return ""
					}
					color: Qaterial.Style.accentColor
				}

				onClicked: {
					if (mode == Exam.ExamVirtual)
						return

					exam.resultToQuickTextDocument(_testResult.textDocument)
					_scrResult.visible = true
					_scrResult.flickable.contentY = 0
				}

				/*onClicked: Client.stackPushPage("PageStudentCampaign.qml", {
													user: Client.server ? Client.server.user : null,
													campaign: campaign,
													studentMapHandler: control.mapHandler,
													withResult: true,
													title: group ? group.name : ""
												})*/
			}
		}

	}


	StackView.onActivated: _examList.reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _examList.reload()
}
