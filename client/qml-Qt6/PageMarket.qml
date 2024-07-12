import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QPageGradient {
	id: root

	progressBarEnabled: true

	DownloaderItem {
		downloader: Client.downloader
		visible: !_scrollable.visible
		anchors.fill: parent
	}

	QScrollable {
		id: _scrollable
		anchors.fill: parent

		visible: false

		refreshEnabled: true
		onRefreshRequest: if (Client.server)
							  Client.server.user.wallet.reload()


		Item {
			width: parent.width
			height: root.paddingTop
		}


		Repeater {
			model: ListModel {
				ListElement {
					type: RpgMarket.Map
					title: qsTr("World")
					icon: "qrc:/Qaterial/Icons/earth.svg"
				}

				ListElement {
					type: RpgMarket.Skin
					title: qsTr("Skin")
					icon: "qrc:/Qaterial/Icons/human-male.svg"
				}

				ListElement {
					type: RpgMarket.Weapon
					title: qsTr("Weapon")
					icon: "qrc:/Qaterial/Icons/sword-cross.svg"
				}

				ListElement {
					type: RpgMarket.Bullet
					title: qsTr("Ammunition")
					icon: "qrc:/Qaterial/Icons/bullet.svg"
				}
			}

			delegate: Column {
				id: _col

				property int filter: model.type

				width: parent.width

				RpgSelectTitle {
					anchors.left: parent.left
					icon.source: model.icon
					text: model.title
				}


				RpgSelectView {
					id: _viewTerrain
					width: root.width

					implicitHeight: Math.max(Math.min(350 * Qaterial.Style.pixelSizeRatio,
											 root.height/2),
											 220 * Qaterial.Style.pixelSizeRatio)

					spacing: 10 * Qaterial.Style.pixelSizeRatio

					delegate: RpgMarketCard {
						height: _viewTerrain.implicitHeight

						wallet: model.qtObject
						resourceLoaded: _scrollable.visible

						//locked: !wallet.available
						//selected: wallet.market.name === _viewTerrain.selected
						//onClicked: _viewTerrain.selectOne(wallet)
					}

					model: SortFilterProxyModel {
						sourceModel: Client.server ? Client.server.user.wallet : null

						filters: ValueFilter {
							roleName: "marketType"
							value: _col.filter
						}

						sorters: [
							RoleSorter {
								roleName: "buyable"
								priority: 1
								sortOrder: Qt.DescendingOrder
							},

							StringSorter {
								roleName: "sortName"
								priority: 0
								sortOrder: Qt.AscendingOrder
							}
						]
					}
				}
			}
		}

	}

	Connections {
		target: Client.downloader

		function onContentDownloaded() {
			_scrollable.visible = true

			if (!Client.server)
				return

			Client.server.user.wallet.reload()
		}
	}

	StackView.onActivated: {
		Client.downloader.download()
	}
}
