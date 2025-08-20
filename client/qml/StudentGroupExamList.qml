import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS


QItemGradient {
	id: control

	property StudentMapHandler mapHandler: null
	property StudentGroupList groupList: null
	property StudentGroup group: null

	title: group ? group.name+qsTr(" | dolgozatok") : ""

	signal changeGroup(StudentGroup group)

	appBar.rightComponent: StudentGroupButton {
		anchors.verticalCenter: parent.verticalCenter
		groupList: control.groupList
		group: control.group
		onChangeGroup: group => control.changeGroup(group)
	}

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

		topPadding: Math.max(Client.safeMarginTop, control.paddingTop + 5)
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
		topPadding: Math.max(Client.safeMarginTop, control.paddingTop + 5)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		visible: !_scrResult.visible

		refreshEnabled: true
		onRefreshRequest: _examList.reload()

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
				property StudentMap _map : exam && mapHandler && exam.mapUuid != "" ?
											   Client.findOlmObject(mapHandler.mapList, "uuid", exam.mapUuid) :
											   null

				highlighted: ListView.isCurrentItem

				readonly property string iconSource: {
					switch (mode) {
					case Exam.ExamVirtual:
						return Qaterial.Icons.stickerTextOutline
					case Exam.ExamPaper:
						return Qaterial.Icons.fileDocumentEdit
					default:
						return Qaterial.Icons.laptopWindows
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

				secondaryTextColor: Qaterial.Style.secondaryTextColor()


				text: description != "" ? description : qsTr("Dolgozat #%1").arg(examId)
				secondaryText: {
					if (timestamp.getTime()) {
						return JS.readableTimestampMin(timestamp)
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


				rightSourceComponent: Row {
					Column {
						visible: mode != Exam.ExamVirtual
						anchors.verticalCenter: parent.verticalCenter
						Qaterial.LabelHeadline5 {
							anchors.right: parent.right
							text: resultGrade ? resultGrade.shortname : ""
							color: Qaterial.Colors.red400
						}
						Qaterial.LabelHint1 {
							anchors.right: parent.right
							text: result>=0 ? Math.floor(result*100)+"%" : ""
							color: Qaterial.Style.primaryTextColor()
						}
					}

					Qaterial.Icon {
						visible: mode == Exam.ExamVirtual && examData.length
								 && examData[0].joker === true
						anchors.verticalCenter: parent.verticalCenter
						icon: Qaterial.Icons.cards
						color: Qaterial.Colors.amber400
						size: 25 * Qaterial.Style.pixelSizeRatio
						sourceSize: Qt.size(size*2, size*2)
					}

					Qaterial.Icon {
						visible: mode == Exam.ExamVirtual && examData.length
								 && examData[0].picked === true
						anchors.verticalCenter: parent.verticalCenter
						icon: Qaterial.Icons.star
						color: Qaterial.Colors.green400
						size: 25 * Qaterial.Style.pixelSizeRatio
						sourceSize: Qt.size(size*2, size*2)
					}
				}

				onClicked: {
					if (mode == Exam.ExamVirtual)
						return

					if (!_map) {
						Client.messageWarning(qsTr("A pálya nem található!"), qsTr("Belső hiba"))
					} else if (!_map.downloaded) {
						Client.snack(qsTr("Pálya letöltése..."))
						_openAfterDownload = true
						mapHandler.mapDownload(_map)
						return
					}

					exam.resultToQuickTextDocument(_testResult.textDocument)
					_scrResult.visible = true
					_scrResult.flickable.contentY = 0
				}

				property bool _openAfterDownload: false

				Connections {
					target: _delegate._map

					function onDownloadedChanged() {
						if (_delegate._openAfterDownload) {
							_delegate._openAfterDownload = false
							_delegate.clicked()
						}
					}
				}
			}
		}

	}


	onGroupChanged: {
		_examList.reload()
		_scrResult.visible = false
	}

	Component.onCompleted: _examList.reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _examList.reload()

	StackView.onActivated: {
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentGroupExam)
		_examList.reload()
	}

	StackView.onDeactivating: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentGroupExam)
	}
}
