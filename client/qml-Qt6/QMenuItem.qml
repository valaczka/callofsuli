import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.MenuItem {
	icon.source: action ? action.icon.source : ""
}
