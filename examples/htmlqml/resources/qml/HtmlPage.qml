/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.0
import QtHtmlQml 1.0

Page {
    id: page

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                icon.source: "qrc:/resources/images/goleft-128.png"
                icon.width: 24
                icon.height: 24
            }
            ToolButton {
                icon.source: "qrc:/resources/images/goright-128.png"
                icon.width: 24
                icon.height: 24
            }
            ComboBox {
                Layout.fillWidth: true
                editable: true
                model: ["http://www.qtcn.org/bbs/i.php", "http://blog.qt.io/"]
            }
            ToolButton {
                icon.source: "qrc:/resources/images/open-128.png"
                icon.width: 24
                icon.height: 24
                onClicked: {
                    fds.open()
                }
            }
        }
    }

    HTMLPart {
        id: htmlPart
        anchors.fill: parent
        view: htmlView

        //AFA: at least use QtQuick.Controls 2.2, Since: Qt 5.9
        ScrollView {
            id: scrollView
            clip: true

            HTMLView {
                id: htmlView
                part: htmlPart
                viewport: scrollView
            }
        }
    }

    Connections {
        target: htmlPart

        onOnURL: {
            if (url.length === 0)
                toolTip.visible = false;
            else {
                toolTip.x = 4;
                toolTip.y = htmlPart.height - 48;

                toolTip.text = url;
                toolTip.visible = true;
            }
        }

        onShowToolTip: {
            if (text.length === 0)
                toolTip.visible = false;
            else {
                toolTip.x = point.x;
                toolTip.y = point.y;

                toolTip.text = text;
                toolTip.visible = true;
            }
        }

        onUrlChanged: {
            console.log(url, prevUrl);
        }
    }

    ToolTip {
        id: toolTip
    }

    FileDialog {
        id:fds
        title: qsTr("Please select file")
        folder: shortcuts.documents
        selectExisting: true
        selectFolder: false
        selectMultiple: false
        nameFilters: ["*.htm *.html"]
        onAccepted: {
            htmlPart.openUrl(fds.fileUrl);
        }
   }
}
