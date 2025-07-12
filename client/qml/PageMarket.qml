import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPageGradient {
	id: root

	property ActionRpgGame game: null

	title: qsTr("Vásárlás")

	progressBarEnabled: true

	appBar.rightComponent: Row {
		spacing: 10 * Qaterial.Style.pixelSizeRatio

		visible: _scrollable.visible

		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.export_
			visible: Client.debug && game
			onClicked: game.saveTerrainInfo()
		}

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
		/*ListElement {
			type: RpgMarket.Map
			title: qsTr("World")
			icon: "qrc:/Qaterial/Icons/earth.svg"
			inBase: true
			inGame: false
		}*/

		ListElement {
			type: RpgMarket.Skin
			title: qsTr("Character")
			icon: "qrc:/Qaterial/Icons/human-male.svg"
			inBase: true
			inGame: false
		}

		ListElement {
			type: RpgMarket.Other
			title: qsTr("Item")
			icon: "qrc:/Qaterial/Icons/chess-knight.svg"
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

		/*ListElement {
			type: RpgMarket.Bullet
			title: qsTr("Ammunition")
			icon: "qrc:/Qaterial/Icons/bullet.svg"
			inBase: true
			inGame: true
		}*/

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
										  root._isBuy = true
										  root._currentWallet = wallet
										  Qaterial.DialogManager.openFromComponent(_cmpBuyDialog)
									  }


						onClicked: if (wallet) {
									   root._isBuy = false
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
	property bool _isBuy: false

	Component {
		id: _cmpBuyDialog

		Qaterial.ModalDialog
		{
			id: _dialog
			//horizontalPadding: 0

			title: (_isBuy ? qsTr("Vásárlás: ") : qsTr("Információk: "))
				   + _currentWallet.readableName.split("\n").join(" ")

			dialogImplicitWidth: 600
			autoFocusButtons: true


			contentItem: Flickable
			{
				id: _flickable2

				implicitHeight: Math.max(250, _layout.implicitHeight)
				interactive: contentHeight > height

				contentHeight: _layout.height
				flickableDirection: Flickable.VerticalFlick

				boundsBehavior: Flickable.StopAtBounds
				boundsMovement: Flickable.StopAtBounds

				ScrollIndicator.vertical: Qaterial.ScrollIndicator {}

				clip: true

				GridLayout {
					id: _layout
					columns: root.width > root.height && root.width > _col2.implicitWidth + 100 ? 2 : 1
					columnSpacing: 10 * Qaterial.Style.pixelSizeRatio
					rowSpacing: 5 * Qaterial.Style.pixelSizeRatio

					width: _flickable2.width
					height: Math.max(implicitHeight, _flickable2.height)

					/*Binding on implicitHeight
					{
						value: (_layout.columns > 1 ? Math.max(_image.implicitHeight, _text2.implicitHeight)
													: _image.implicitHeight + _text2.implicitHeight + _layout.rowSpacing)

						delayed: true
					}*/


					Item
					{
						//id: _image

						Layout.fillWidth: true
						Layout.fillHeight: true

						Layout.maximumWidth: _layout.width / _layout.columns
						Layout.maximumHeight: 300

						implicitHeight: _img.sourceSize.height
						implicitWidth: _img.sourceSize.width

						AnimatedImage {
							id: _img

							source: _currentWallet.image
							fillMode: Image.PreserveAspectFit
							horizontalAlignment: Image.AlignHCenter
							verticalAlignment: Image.AlignVCenter

							anchors.centerIn: parent

							width: Math.min(parent.width, sourceSize.width)
							height: Math.min(parent.height, sourceSize.height)
						}
					}

					Item {
						id: _text2

						Layout.fillWidth: true
						Layout.fillHeight: false
						Layout.alignment: Qt.AlignCenter

						implicitWidth: _col2.width
						implicitHeight: _col2.height

						Column {
							id: _col2

							anchors.centerIn: parent

							Repeater {
								id: _rptr

								delegate: Row {
									id: _rw
									spacing: 5 * Qaterial.Style.pixelSizeRatio
									//width: Math.min(parent.width)

									required property string text
									required property font font
									required property string value
									required property string icon
									required property color color
									required property string image
									required property int imageSize

									Qaterial.IconLabel {
										id: _icon
										icon.source: _rw.icon
										text: _rw.text
										color: Qaterial.Style.secondaryTextColor()
										wrapMode: Text.Wrap
										anchors.verticalCenter: parent.verticalCenter
									}


									Qaterial.Label {
										id: _label
										font: _rw.font
										text: _rw.value
										width: Math.min(implicitWidth,
														_text2.width - _rw.spacing - _icon.width
														- (_rw.image != "" ? _image2.width + _rw.spacing : 0))
										wrapMode: Text.Wrap
										color: _rw.color
										anchors.verticalCenter: parent.verticalCenter
									}

									Image {
										id: _image2
										source: _rw.image
										fillMode: Image.PreserveAspectFit
										height: _rw.imageSize > 0 ? _rw.imageSize : _rw.font.pixelSize * 0.8
										width: height

										visible: source != ""

										anchors.verticalCenter: parent.verticalCenter
									}

								}
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
								  image: "",
								  imageSize: 0
							  })
				}

				if (_currentWallet.market.amount > 1) {
					list.push({
								  icon: Qaterial.Icons.numeric,
								  text: qsTr("Mennyiség:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: qsTr("%1 db").arg(_currentWallet.market.amount),
								  color: Qaterial.Style.primaryTextColor(),
								  image: "",
								  imageSize: 0
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
								  image: "",
								  imageSize: 0
							  })
				}

				if (_currentWallet.market.rollover !== RpgMarket.None) {
					let t = ""

					switch (_currentWallet.market.rollover) {
					case RpgMarket.Day: t = qsTr("/nap"); break
					case RpgMarket.Game: t = qsTr("/játék"); break
					}

					list.push({
								  icon: Qaterial.Icons.gaugeFull,
								  text: qsTr("Limit:"),
								  font: Qaterial.Style.textTheme.body2,
								  value: _currentWallet.market.num + t,
								  color: Qaterial.Style.primaryTextColor(),
								  image: "",
								  imageSize: 0
							  })
				}


				list.push({
							  icon: Qaterial.Icons.cash,
							  text: qsTr("Ár:"),
							  font: Qaterial.Style.textTheme.headline5,
							  value: _currentWallet.market.cost,
							  color: Qaterial.Colors.amber400,
							  image: "qrc:/rpg/coin/coin.png",
							  imageSize: 0
						  })

				if (_currentWallet.market.rank > 0) {
					let r = Client.server.rank(_currentWallet.market.rank)

					if (r.id > 0) {
						list.push({
									  icon: Qaterial.Icons.medal,
									  text: qsTr("Rank:"),
									  font: Qaterial.Style.textTheme.body1,
									  value: r.name + qsTr(" lvl %1").arg(r.sublevel),
									  color: Qaterial.Colors.cyan400,
									  image: "qrc:/internal/rank/"+r.level+".svg",
									  imageSize: Qaterial.Style.textTheme.body1.pixelSize * 1.1
								  })
					}
				}

				if (_currentWallet.market.belongs !== "") {
					let bw = _currentWallet.getBelongsTo()

					if (bw) {
						list.push({
									  icon: Qaterial.Icons.account,
									  text: qsTr("Karakter:"),
									  font: Qaterial.Style.textTheme.body1,
									  value: bw.readableName,
									  color: Qaterial.Colors.cyan400,
									  image: "",
									  imageSize: 0
								  })
					}
				}


				for (let i=0; i<_currentWallet.extendedInfo.length; ++i) {
					let o = {}
					let keys = Object.keys(_currentWallet.extendedInfo[i])

					for (let n=0; n<keys.length; ++n)
						o[keys[n]] = _currentWallet.extendedInfo[i][keys[n]]

					o.font = Qaterial.Style.textTheme.body2
					o.color = Qaterial.Style.primaryTextColor()

					list.push(o)
				}

				_rptr.model = list
			}

			standardButtons: _isBuy ? DialogButtonBox.No | DialogButtonBox.Yes : DialogButtonBox.Ok

			onAccepted: if (_isBuy)
							root.buyWallet(_currentWallet)

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
