import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

ListView {
	id: root

	enum Position {
		Duration,
		Solved
	}

	property MapGameList mapGameList: null
	property int positionType: MapPlayMissionLevelInfoList.Position.Duration
	property bool showPlaceholders: true


	property color mainColor: root.positionType == MapPlayMissionLevelInfoList.Position.Duration ?
								  Qaterial.Colors.green300 :
								  Qaterial.Style.iconColor()

	property alias filterEnabled: _filter.enabled

	property int _maxValue: 1
	property int _minValue: 0

	height: contentHeight

	model: showPlaceholders ? 3 : _model

	boundsBehavior: Flickable.StopAtBounds

	SortFilterProxyModel {
		id: _model
		sourceModel: mapGameList

		sorters: [
			RoleSorter {
				roleName: root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? "posDuration" : "posSolved"
				sortOrder: Qt.AscendingOrder
			}
		]

		filters: [
			AnyOf {
				id: _filter
				RangeFilter {
					roleName: root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? "posDuration" : "posSolved"
					maximumValue: 5
				}
				ValueFilter {
					roleName: "username"
					value: Client.server && Client.server.user ? Client.server.user.username : ""
				}
			}

		]
	}

	delegate: showPlaceholders ? _cmpPlacholder : _cmpDelegate

	Component {
		id: _cmpDelegate
		Qaterial.FullLoaderItemDelegate {
			id: _delegate

			width: ListView.view.width
			property MapGame game: model.qtObject

			spacing: 10  * Qaterial.Style.pixelSizeRatio
			leftPadding: 0
			rightPadding: 0

			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

			highlighted: game && game.user.username == Client.server.user.username

			contentSourceComponent: Label {
				font: Qaterial.Style.textTheme.body2Upper
				verticalAlignment: Label.AlignVCenter
				text: game && game.user ? (root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? game.posDuration : game.posSolved)+". "
										  +game.user.fullNickName : ""
				color: root.mainColor

				elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
			}

			leftSourceComponent: UserImage {
				user: game ? game.user : null
				pictureEnabled: false
				sublevelEnabled: false
				height: _delegate.height-6
				width: _delegate.height-6
			}

			rightSourceComponent: Row {
				spacing: 8

				Qaterial.ProgressBar {
					anchors.verticalCenter: parent.verticalCenter
					width: 60 * Qaterial.Style.pixelSizeRatio
					from: 0
					to: root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? root._maxValue - root._minValue
																						   : root._maxValue
					value: game ? (root.positionType == MapPlayMissionLevelInfoList.Position.Duration ?
									   root._maxValue - game.durationMin :
									   game.solved) : 0

					color: root.mainColor

					Behavior on value {
						NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
					}
				}

				Qaterial.LabelCaption {
					width: Math.max(30 * Qaterial.Style.pixelSizeRatio, implicitWidth)
					horizontalAlignment: Text.AlignHCenter
					anchors.verticalCenter: parent.verticalCenter
					text: game ? (root.positionType == MapPlayMissionLevelInfoList.Position.Duration ?
									  Client.Utils.formatMSecs(game.durationMin) :
									  qsTr("%1x").arg(game.solved))
							   : ""
					color: root.mainColor
				}

			}
		}
	}

	Component {
		id: _cmpPlacholder
		Qaterial.FullLoaderItemDelegate {
			id: _delegatePlaceholder

			width: ListView.view.width
			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

			spacing: 10  * Qaterial.Style.pixelSizeRatio
			leftPadding: 0
			rightPadding: 0

			contentSourceComponent: QPlaceholderItem {
				horizontalAlignment: Qt.AlignLeft
				heightRatio: 0.5
			}

			leftSourceComponent: QPlaceholderItem {
				contentComponent: ellipseComponent
				fixedHeight: _delegatePlaceholder.height-6
				fixedWidth: fixedHeight
			}

			rightSourceComponent: QPlaceholderItem {
				width: 90
				height: Qaterial.Style.textTheme.caption.pixelSize
				heightRatio: 1.0
				widthRatio: 1.0
			}
		}
	}
}
