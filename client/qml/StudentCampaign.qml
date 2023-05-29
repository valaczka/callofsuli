import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0


Item {
	id: root

	property User user: null
	property Campaign campaign: null
	property StudentMapHandler mapHandler: null

	property bool campaingDetails: true

	default property alias _colContent: col.data

	property bool _firstRun: true

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

				text: campaign ? campaign.readableShortResult(campaign.resultGrade, campaign.resultXP) : ""

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
							source: task && task.success ? "qrc:/internal/img/checkmark_red.png" : ""
						}

						Label {
							id: labelText
							width: parent.width-imgSuccess.width-parent.spacing
							wrapMode: Text.Wrap

							anchors.top: parent.top

							color: task && task.required ? Qaterial.Colors.red900 : Qaterial.Colors.black
							font.strikeout: task && task.success
							opacity: task && task.success ? 0.4 : 1.0

							topPadding: 2
							bottomPadding: 2

							font.family: "Special Elite"
							font.pixelSize: Qaterial.Style.textTheme.subtitle2.pixelSize
							lineHeight: 1.2

							text: task ? (task.required ? qsTr("* ") : "") + task.readableCriterion(root.mapHandler.mapList) : " "

							Connections {
								target: root.mapHandler

								function onReloaded() {
									if (_delegate.task)
										labelText.text = _delegate.task.readableCriterion(root.mapHandler.mapList)
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
			text: qsTr("*A csillaggal jelölt feladat teljesítése szükséges a további értékeléshez is!")
			width: parent.width
			wrapMode: Text.Wrap
			color: Qaterial.Colors.red900
			topPadding: 10
		}
	}


	Connections {
		target: mapHandler

		function onReloaded() {
			_firstRun = false
		}
	}

}


