import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
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
		QLoaderItemFullDelegate {
			id: _delegate
			property MapGame game: model.qtObject

			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

			highlighted: game && game.user.username == Client.server.user.username

			contentSourceComponent: Qaterial.LabelBody2 {
				font.family: Qaterial.Style.textTheme.body2.family
				font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
				font.capitalization: Font.AllUppercase
				font.weight: Font.DemiBold
				verticalAlignment: Label.AlignVCenter
				text: game && game.user ? (root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? game.posDuration : game.posSolved)+". "
										  +game.user.fullNickName : ""
				color: root.mainColor
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
		QLoaderItemFullDelegate {
			id: _delegatePlaceholder

			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

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
