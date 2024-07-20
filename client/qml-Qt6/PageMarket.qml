import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPageGradient {
	id: root

	property ActionRpgGame game: null

	title: qsTr("Shop")

	progressBarEnabled: true

	appBar.rightComponent: Row {
		spacing: 10 * Qaterial.Style.pixelSizeRatio

		visible: _scrollable.visible

		Image {
			source: _scrollable.visible ? "qrc:/rpg/coin/coins.png" : ""
			fillMode: Image.PreserveAspectFit
			height: _labelCurrency.height * 0.7
			width: sourceSize.width * (height/sourceSize.height)
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.LabelHeadline5 {
			id: _labelCurrency
			color: Qaterial.Colors.amber400

			anchors.verticalCenter: parent.verticalCenter

			property int num: Client.server ? Client.server.user.wallet.currency : 0

			text: Number(num).toLocaleString()


			rightPadding: Qaterial.Style.delegate.rightPadding(1, 1)

			Behavior on num {
				NumberAnimation { duration: 450; easing.type: Easing.OutCubic }
			}
		}
	}

	DownloaderItem {
		downloader: Client.downloader
		visible: !_scrollable.visible
		anchors.fill: parent
	}

	ListModel {
		id: _model
		ListElement {
			type: RpgMarket.Map
			title: qsTr("World")
			icon: "qrc:/Qaterial/Icons/earth.svg"
			inBase: true
			inGame: false
		}

		ListElement {
			type: RpgMarket.Skin
			title: qsTr("Skin")
			icon: "qrc:/Qaterial/Icons/human-male.svg"
			inBase: true
			inGame: false
		}

		ListElement {
			type: RpgMarket.Other
			title: qsTr("Item")
			icon: "qrc:/Qaterial/Icons/sword-cross.svg"
			inBase: false
			inGame: true
		}

		ListElement {
			type: RpgMarket.Weapon
			title: qsTr("Weapon")
			icon: "qrc:/Qaterial/Icons/sword-cross.svg"
			inBase: true
			inGame: true
		}

		ListElement {
			type: RpgMarket.Bullet
			title: qsTr("Ammunition")
			icon: "qrc:/Qaterial/Icons/bullet.svg"
			inBase: true
			inGame: true
		}

		ListElement {
			type: RpgMarket.Xp
			title: qsTr("XP")
			icon: "qrc:/Qaterial/Icons/chart-timeline-variant-shimmer.svg"
			inBase: true
			inGame: false
		}
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
			model: SortFilterProxyModel {
				sourceModel: _model

				filters: AnyOf {
					ValueFilter {
						roleName: "inBase"
						value: true
						enabled: !(game && game.config.gameState === RpgConfig.StatePlay)
					}
					ValueFilter {
						roleName: "inGame"
						value: true
						enabled: game && game.config.gameState === RpgConfig.StatePlay
					}
				}
			}


			delegate: Column {
				id: _col

				property int filter: model.type

				width: parent.width

				visible: _viewTerrain.model.count > 0

				RpgSelectTitle {
					anchors.left: parent.left
					icon.source: model.icon
					text: model.title
				}


				RpgSelectView {
					id: _viewTerrain
					width: root.width

					implicitHeight: Math.max(Math.min(300 * Qaterial.Style.pixelSizeRatio,
													  root.height*0.4),
											 240 * Qaterial.Style.pixelSizeRatio)

					spacing: 10 * Qaterial.Style.pixelSizeRatio

					delegate: RpgMarketCard {
						height: _viewTerrain.implicitHeight

						wallet: model.qtObject
						resourceLoaded: _scrollable.visible
						availableCurrency: Client.server ? Client.server.user.wallet.currency : 0

						onBuyRequest: if (wallet) {
										  root._currentWallet = wallet
										  Qaterial.DialogManager.openFromComponent(_cmpBuyDialog)
									  }
					}

					model: SortFilterProxyModel {
						sourceModel: Client.server ? Client.server.user.wallet : null

						filters: ValueFilter {
							roleName: "marketType"
							value: _col.filter
						}

						sorters: [
							FilterSorter {
								filters: AllOf {
									ValueFilter {
										roleName: "buyable"
										value: false
									}
									ValueFilter {
										roleName: "available"
										value: false
									}
								}

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

	property RpgUserWallet _currentWallet: null

	Component {
		id: _cmpBuyDialog

		Qaterial.ModalDialog
		{
			id: _dialog
			//horizontalPadding: 0

			title: qsTr("Vásárlás: %1").arg(_currentWallet.readableName)

			dialogImplicitWidth: Math.max(350, Math.min(350, _image.implicitWidth) * 2)
			autoFocusButtons: true

			contentItem: Item
			{
				id: _dialogRow

				readonly property real spacing: 10 * Qaterial.Style.pixelSizeRatio

				implicitWidth: 100
				implicitHeight: Math.max(Math.min(350, _image.implicitHeight), 250)

				Image
				{
					id: _image
					source: _currentWallet.image
					fillMode: Image.PreserveAspectFit
					horizontalAlignment: Image.AlignHCenter
					verticalAlignment: Image.AlignVCenter
					anchors.left: parent.left
					anchors.verticalCenter: parent.verticalCenter
					width: Math.min((_dialogRow.width-_dialogRow.spacing)/2, sourceSize.width)
					height: Math.min(_dialogRow.height, sourceSize.height)
				}

				Column {
					anchors.left: _image.right
					anchors.leftMargin: _dialogRow.spacing
					anchors.right: parent.right
					anchors.verticalCenter: parent.verticalCenter

					Repeater {
						id: _rptr

						delegate: Row {
							id: _rw
							spacing: 5 * Qaterial.Style.pixelSizeRatio
							width: parent.width

							required property string text
							required property font font
							required property string value
							required property string icon
							required property color color
							required property string image

							Qaterial.IconLabel {
								id: _icon
								icon.source: _rw.icon
								text: _rw.text
								color: Qaterial.Style.secondaryTextColor()
								anchors.verticalCenter: parent.verticalCenter
							}

							Qaterial.Label {
								id: _label
								font: _rw.font
								text: _rw.value
								width: Math.min(implicitWidth,
												_rw.width - _rw.spacing - _icon.width
												- (_rw.image != "" ? _image2.width + _rw.spacing : 0))
								wrapMode: Text.Wrap
								color: _rw.color
								anchors.verticalCenter: parent.verticalCenter
							}

							Image {
								id: _image2
								source: _rw.image
								fillMode: Image.PreserveAspectFit
								height: _rw.font.pixelSize * 0.8
								width: height

								anchors.verticalCenter: parent.verticalCenter
							}
						}
					}
				}

			}

			Component.onCompleted: {
				let list = []

				if (_currentWallet.amount > 0) {
					list.push({
								  icon: Qaterial.Icons.checkBold,
								  text: qsTr("Meglévő:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: qsTr("%1 db").arg(_currentWallet.amount),
								  color: Qaterial.Style.secondaryTextColor(),
								  image: ""
							  })
				}

				if (_currentWallet.market.amount > 1) {
					list.push({
								  icon: Qaterial.Icons.numeric,
								  text: qsTr("Mennyiség:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: qsTr("%1 db").arg(_currentWallet.market.amount),
								  color: Qaterial.Style.primaryTextColor(),
								  image: ""
							  })
				}

				if (_currentWallet.market.exp) {
					let d = new Date()
					d.setTime(d.getTime() + _currentWallet.market.exp*60*1000)

					list.push({
								  icon: Qaterial.Icons.clockOutline,
								  text: qsTr("Felhasználható:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: JS.readableTimestampMin(d),
								  color: Qaterial.Style.primaryTextColor(),
								  image: ""
							  })
				}

				if (_currentWallet.market.rollover !== RpgMarket.None) {
					let t = ""

					switch (_currentWallet.market.rollover) {
					case RpgMarket.Day: t = qsTr("/nap"); break
					case RpgMarket.Game: t = qsTr("/játék"); break
					}

					list.push({
								  icon: Qaterial.Icons.abacus,
								  text: qsTr("Limit:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: _currentWallet.market.num + t,
								  color: Qaterial.Style.primaryTextColor(),
								  image: ""
							  })
				}


				list.push({
							  icon: Qaterial.Icons.cash,
							  text: qsTr("Ár:"),
							  font: Qaterial.Style.textTheme.headline5,
							  value: _currentWallet.market.cost,
							  color: Qaterial.Colors.amber400,
							  image: "qrc:/rpg/coin/coin.png"
						  })


				for (let i=0; i<_currentWallet.extendedInfo.length; ++i) {
					let o = _currentWallet.extendedInfo[i]

					list.push({
								  icon: o.icon,
								  text: o.text,
								  font: Qaterial.Style.textTheme.body2,
								  value: o.value,
								  color: Qaterial.Style.secondaryTextColor(),
								  image: o.image
							  })
				}

				_rptr.model = list
			}

			standardButtons: DialogButtonBox.No | DialogButtonBox.Yes

			onAccepted: root.buyWallet(_currentWallet)

		}

	}


	function buyWallet(wallet) {
		let d = _currentWallet.getJson()

		if (game && game.config.gameState === RpgConfig.StatePlay)
			d.gameid = game.gameid

		let w = _currentWallet

		Client.send(HttpConnection.ApiUser, "buy", d)
		.done(root, function(r){
			Client.server.user.wallet.reload()
			if (game)
				game.addWallet(w)
		})
		.fail(root, JS.failMessage(qsTr("Sikertelen vásárlás")))
	}


	function activate() {
		_scrollable.visible = true

		if (!Client.server)
			return

		Client.server.user.wallet.reload()
	}

	Connections {
		target: Client.downloader

		function onContentDownloaded() {
			root.activate()
		}
	}

	StackView.onActivated: {
		Client.downloader.download()
	}
}
