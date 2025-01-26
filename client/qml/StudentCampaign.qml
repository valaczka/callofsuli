import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli


Item {
	id: root

	property User user: null
	property Campaign campaign: null
	property QtObject mapHandler: null

	property bool campaingDetails: true

	default property alias _colContent: col.data

	implicitWidth: 200
	implicitHeight: 200

	width: Math.min(parent.width, 600)
	height: col.height+85//+50

	BorderImage {
		width: parent.width/scale
		height: parent.height/scale
		source: "qrc:/internal/img/paper.png"
		border {
			left: 253
			top: 129
			right: 1261-1175
			bottom: 851-737
		}
		scale: Qaterial.Style.dense ? 0.3 : 0.15
		transformOrigin: Item.TopLeft
	}

	Connections {
		target: campaign

		function onTaskListReloaded() {
			_colTaskList.reload()
		}
	}


	Column {
		id: col
		x: Qaterial.Style.dense ? 50 : 25
		width: parent.width-2*x
		y: Qaterial.Style.dense ? 35 : 20

		Row {
			width: parent.width
			spacing: 10

			Column {
				anchors.verticalCenter: parent.verticalCenter
				width: parent.width-_labelResult.width-parent.spacing

				Qaterial.Label {
					visible: campaingDetails
					text: campaign ? campaign.readableName : ""
					width: parent.width
					wrapMode: Text.Wrap
					font.family: "HVD Peace"
					font.pixelSize: Qaterial.Style.textTheme.headline4.pixelSize
					color: root.finished ? "black" :"saddlebrown"
				}

				Qaterial.LabelCaption {
					visible: campaingDetails
					text: campaign ? campaign.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm – ")
									 + (campaign.endTime.getTime() ? campaign.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm") : "")
								   : ""
					width: parent.width
					wrapMode: Text.Wrap
					color: root.finished ? "black" :"saddlebrown"
				}
			}

			Label {
				id: _labelResult

				anchors.top: parent.top

				text: campaign ? campaign.readableShortResult(campaign.resultGrade, campaign.resultXP, campaign.progress * campaign.maxPts) : ""

				wrapMode: Text.Wrap
				color: campaign && !campaign.finished ? Qaterial.Colors.red900 : Qaterial.Colors.black
				font.family: Qaterial.Style.textTheme.headline5.family
				font.pixelSize: Qaterial.Style.textTheme.headline5.pixelSize
				font.capitalization: Font.AllUppercase
				font.weight: Font.DemiBold
				topPadding: 5
				bottomPadding: 5
			}
		}




		Column {
			id: _colTaskList

			width: parent.width

			Repeater {
				id: _rptr

				property bool showPlaceholders: true

				model: 5

				delegate: Item {
					width: parent.width
					height: _rptr.showPlaceholders || task ? _delegate.implicitHeight : _section.implicitHeight

					readonly property Task task: modelData.task !== undefined ? modelData.task : null

					Qaterial.LabelCaption {
						id: _section
						visible: !_rptr.showPlaceholders && !task
						width: parent.width
						text: modelData.section !== undefined ? modelData.section : ""
						color: "saddlebrown"
						topPadding: 15
						bottomPadding: 5
						font.pixelSize: Qaterial.Style.textTheme.caption.pixelSize
						font.family: Qaterial.Style.textTheme.caption.family
						font.capitalization: Font.AllUppercase
						font.weight: Qaterial.Style.textTheme.caption.weight
					}

					Row {
						id: _delegate

						visible: task && !_rptr.showPlaceholders

						width: parent.width

						spacing: 10

						Image {
							id: imgSuccess
							anchors.top: parent.top
							width: labelText.font.pixelSize*1.4
							height: labelText.font.pixelSize*1.4
							fillMode: Image.PreserveAspectFit
							source: task && task.success && task.result >= 1.0 ? "qrc:/internal/img/checkmark_red.png" : ""

							sourceSize.width: width
							sourceSize.height: height
						}

						Column {
							anchors.top: parent.top
							width: parent.width-imgSuccess.width-parent.spacing

							Label {
								id: labelText

								width: parent.width

								wrapMode: Text.Wrap

								color: task && task.required ? Qaterial.Colors.red900 : Qaterial.Colors.black
								font.strikeout: task && task.success && task.result >= 1.0
								opacity: task && task.success && task.result >= 1.0 ? 0.4 : 1.0

								topPadding: 2
								bottomPadding: 2

								font.family: "Special Elite"
								font.pixelSize: Qaterial.Style.textTheme.subtitle2.pixelSize
								lineHeight: 1.2

								text: task ? (task.required ? qsTr("* ") : "") + task.readableCriterion(root.mapHandler.mapList, campaign) : " "

								Connections {
									target: root.mapHandler

									function onReloaded() {
										if (_delegate.task)
											labelText.text = _delegate.task.readableCriterion(root.mapHandler.mapList, campaign)
									}
								}

								states: State {
									when: (task && task.success && imgSuccess.height > labelText.height)

									AnchorChanges {
										target: labelText
										anchors.top: undefined
										anchors.verticalCenter: parent.verticalCenter
									}
								}
							}

							Row {
								visible: task && task.criterion.pts !== undefined && task.criterion.pts > 0

								spacing: 5 * Qaterial.Style.pixelSizeRatio

								Qaterial.ProgressBar {
									width: parent.parent.width - _labelProgress.width - parent.spacing

									anchors.verticalCenter: parent.verticalCenter

									color: "saddlebrown"

									from: 0.0
									to: 1.0
									value: Math.min(1.0, _labelProgress.percent)

									Behavior on value {
										NumberAnimation { duration: 200 }
									}
								}

								Qaterial.LabelHint1 {
									id: _labelProgress

									anchors.verticalCenter: parent.verticalCenter

									readonly property real percent: task && task.criterion.pts !== undefined && task.criterion.pts > 0 ?
																	   task.result :
																	   0
									text: qsTr("%1%").arg(Math.floor(percent*100))

									color: "saddlebrown"
								}
							}
						}
					}


					QPlaceholderItem {
						visible: _rptr.showPlaceholders
						anchors.fill: parent
						horizontalAlignment: Qt.AlignLeft
						height: 48
						heightRatio: 0.8
					}

				}
			}


			function reload() {
				if (campaign) {
					_rptr.model = campaign.getOrderedTaskListModel()
					_rptr.showPlaceholders = false
					_labelHasRequired.visible = campaign.hasRequiredTask()
				}
			}
		}


		Qaterial.LabelCaption {
			id: _labelHasRequired
			visible: false
			text: qsTr("*A csillaggal jelölt feladatok teljesítése szükséges a magasabb értékeléshez is!")
			width: parent.width
			wrapMode: Text.Wrap
			color: Qaterial.Colors.red900
			topPadding: 10
		}
	}


}


