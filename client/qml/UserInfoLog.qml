import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Qaterial.Expandable {
	id: root

	property alias username: _userLog.username

	property bool _showPlaceholders: true
	property int _pendingLoaders: -1

	expanded: true

	onVisibleChanged: evaluateDelegateClipperHeight()

	header: QExpandableHeader {
		text: qsTr("Előléptetések")
		icon: Qaterial.Icons.medalOutline
		expandable: root
	}

	delegate: Column {
		width: root.width
		height: implicitHeight

		Column {
			width: root.width
			visible: _showPlaceholders

			Repeater {
				model: 5
				delegate: QLoaderItemFullDelegate {
					id: _delegatePlaceholder

					width: root.width

					height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

					spacing: 10
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

		Column {
			width: parent.width
			visible: !_showPlaceholders

			Repeater {
				id: _rptr
				model: null
				delegate: Loader {
					asynchronous: true
					sourceComponent: QLoaderItemFullDelegate {
						id: _delegate

						spacing: 10
						leftPadding: 0
						rightPadding: 0

						width: root.width

						height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

						highlighted: modelData && modelData.rank === undefined

						readonly property color _color: modelData && modelData.rank !== undefined ?
															Qaterial.Style.primaryTextColor() : Qaterial.Style.accentColor

						contentSourceComponent: Label {
							font: Qaterial.Style.textTheme.body2
							verticalAlignment: Label.AlignVCenter


							color: _delegate._color

							text: modelData.rank !== undefined
								  ? (modelData.rank.sublevel > 0 ?
										 qsTr("%1 (level %2)").arg(modelData.rank.name).arg(modelData.rank.sublevel) : modelData.rank.name)
								  : "Streak: "+modelData.streak
						}

						leftSourceComponent: UserImage {
							visible: modelData && modelData.rank !== undefined
							userData: modelData && modelData.rank !== undefined ? modelData : null
							pictureEnabled: false
							sublevelEnabled: true
							height: _delegate.height-6
							width: _delegate.height-6
						}

						rightSourceComponent: Qaterial.LabelCaption {
							text: JS.readableDate(modelData.timestamp)
							color: _delegate._color
						}
					}

					onLoaded: {
						if (_showPlaceholders) {
							_pendingLoaders--
							if (_pendingLoaders <= 0)
								_showPlaceholders = false
						}
					}
				}
			}
		}


		Connections {
			target: _userLog

			function onModelReloaded() {
				_pendingLoaders = _userLog.model.length
				_rptr.model = _userLog.model
				if (_pendingLoaders <= 0)
					_showPlaceholders = false
			}

			function onModelReloadRequested() {
				_showPlaceholders = true
				_rptr.model = null
			}
		}
	}





	UserLogListImpl {
		id: _userLog
	}


	function reload() {
		_userLog.reload()
	}
}
