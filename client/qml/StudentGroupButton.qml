import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel


Qaterial.AppBarButton {
    id: _root

    property StudentGroupList groupList: null
    property StudentGroup group: null

    signal changeGroup(StudentGroup group)

    icon.source: Qaterial.Icons.accountGroupOutline
    ToolTip.text: qsTr("Csoportok")

    visible: groupList && groupList.count > 1

    onClicked: _menuFilter.popup(_root, 0, _root.height)

    QMenu {
        id: _menuFilter

        Instantiator {
            model: SortFilterProxyModel {
                sourceModel: groupList
                sorters: [
                    StringSorter {
                        roleName: "name"
                        sortOrder: Qt.AscendingOrder
                    }
                ]
            }

            delegate: QMenuItem {
                text: model.name
                onTriggered: changeGroup(model.qtObject)
            }

            onObjectAdded: (index, object) => _menuFilter.insertItem(index, object)
            onObjectRemoved: (index, object) => _menuFilter.removeItem(object)
        }
    }

}
