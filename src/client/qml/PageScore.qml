import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Rangsor")

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	swipeMode: control.width < 900


	activity: Scores {
		id: scores
	}


	panelComponents: Component {
		QPagePanel {
			panelVisible: true
			layoutFillWidth: true

			id: panel

			maximumWidth: 800

			SortFilterProxyModel {
				id: scoreProxyModel
				sourceModel: scores.scoreModel
				filters: [
					/*RegExpFilter {
						enabled: toolbar.searchBar.text.length
						roleName: "name"
						pattern: toolbar.searchBar.text
						caseSensitivity: Qt.CaseInsensitive
						syntax: RegExpFilter.FixedString
					}*/

					AllOf {
						ValueFilter {
							roleName: "isTeacher"
							value: false
						}
						ValueFilter {
							roleName: "isAdmin"
							value: false
						}
					}
				]

				sorters: [
					RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder },
					RoleSorter { roleName: "xp"; sortOrder: Qt.DescendingOrder }
				]

				proxyRoles: [
					ExpressionRole {
						name: "details"
						expression: model.rankname+(model.ranklevel > 0 ? " (lvl "+model.ranklevel+")": "")
					},
					ExpressionRole {
						name: "name"
						expression: model.nickname.length ? model.nickname : model.firstname+" "+model.lastname
					},
					SwitchRole {
						name: "background"
						filters: ValueFilter {
							roleName: "username"
							value: cosClient.userName
							SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
						}
						defaultValue: "transparent"
					},
					SwitchRole {
						name: "titlecolor"
						filters: ValueFilter {
							roleName: "username"
							value: cosClient.userName
							SwitchRole.value: CosStyle.colorAccentLight
						}
						defaultValue: CosStyle.colorAccentLighter
					}

				]
			}


			QVariantMapProxyView {
				id: scoreList
				anchors.fill: parent

				refreshEnabled: true
				delegateHeight: CosStyle.twoLineHeight
				numbered: true

				leftComponent: Image {
					source: cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage)
					width: scoreList.delegateHeight+10
					height: scoreList.delegateHeight*0.7
					fillMode: Image.PreserveAspectFit
				}

				rightComponent: QLabel {
					text: model.xp+" XP"
					font.weight: Font.Normal
					font.pixelSize: scoreList.delegateHeight*0.5
					color: CosStyle.colorAccent
				}

				model: scoreProxyModel
				modelTitleRole: "name"
				modelSubtitleRole: "details"
				modelBackgroundRole: "background"
				modelTitleColorRole: "titlecolor"
				colorSubtitle: CosStyle.colorAccentDark

				highlightCurrentItem: false

				onRefreshRequest: reloadModel()
			}


			Connections {
				target: cosClient
				function onUserRolesChanged(userRoles) {
					reloadModel()
				}
			}

			Connections {
				target: page
				function onPageActivated() {
					reloadModel()
				}
			}
		}

	}



	function reloadModel() {
		scores.send("getAllUser")
	}




	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}
}


