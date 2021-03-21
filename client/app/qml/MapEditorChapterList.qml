import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	title: qsTr("Szakaszok")
	icon: "image://font/AcademicI/\uf203"

	contextMenuFunc: function (m) {
		m.addAction(actionNew)
		m.addAction(actionRename)
		m.addAction(actionRemove)
	}


	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: mapEditor.modelChapterList.count

		model: SortFilterProxyModel {
			sourceModel: mapEditor.modelChapterList

			sorters: [
				StringSorter { roleName: "name" }
			]
		}

		modelTitleRole: "name"

		autoSelectorChange: true
		autoUnselectorChange: true

		leftComponent: QFontImage {
			width: list.delegateHeight
			height: list.delegateHeight
			size: Math.min(height*0.8, 32)

			icon: "image://font/Academic/\uf1ca"

			color: CosStyle.colorAccent
		}

		rightComponent: Row {
			spacing: 5

			QBadge {
				anchors.verticalCenter: parent.verticalCenter

				visible: model && model.missionCount

				color: CosStyle.colorWarningDark

				text: model && model.missionCount ? model.missionCount : ""
			}

			QBadge {
				anchors.verticalCenter: parent.verticalCenter

				visible: model && model.objCount

				text: model && model.objCount ? model.objCount : ""
			}
		}


		onClicked: {
			var o = list.model.get(index)
			mapEditor.chapterSelected(o.id)
			mapEditor.run("chapterLoad", {id: o.id})
		}

		onRightClicked: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionNew }
			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
		}


		onKeyInsertPressed: actionNew.trigger()
		onKeyF4Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !mapEditor.modelChapterList.count
		action: actionNew
		color: CosStyle.colorOK
	}



	Action {
		id: actionNew
		text: qsTr("Új szakasz")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új szakasz neve")

			d.accepted.connect(function(data) {
				mapEditor.run("chapterAdd", {"name": data})
			})
			d.open()
		}
	}


	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && (list.currentIndex !== -1 || mapEditor.modelChapterList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = mapEditor.modelChapterList.selectedCount

			if (more > 0)
				mapEditor.run("chapterRemove", {"list": mapEditor.modelChapterList.getSelectedData("id") })
			else
				mapEditor.run("chapterRemove", {"id": o.id})
		}
	}

	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: !mapEditor.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Szakasz neve"),
										   value: o.name
									   })

			d.accepted.connect(function(data) {
				mapEditor.run("chapterModify", {
								  "id": o.id,
								  "data": {"name": data}
							  })
			})
			d.open()
		}
	}



	Connections {
		target: mapEditor

		function onChapterAdded(id) {
			for (var i=0; i<list.model.count; i++) {
				if (list.model.get(i).id === id) {
					list.currentIndex = i
					break
				}
			}

			mapEditor.run("chapterLoad", {id: id})
		}


	}


	Component.onCompleted: {
		mapEditor.run("chapterListReload")
	}

	onPopulated: list.forceActiveFocus()

}



