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

    property bool canAddPrev: true;
    property bool comboBoxBlockSignals: false;
    property var comboBoxUrls: ["http://www.qtcn.org/bbs/i.php", "http://blog.qt.io/"];
    property var prevUrls: [];
    property var nextUrls: [];

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                id: toolButtonPrev
                icon.source: "qrc:/resources/images/goleft-128.png"
                icon.width: 24
                icon.height: 24
                enabled: false
                onClicked: slotPrevUrl()
            }
            ToolButton {
                id: toolButtonNext
                icon.source: "qrc:/resources/images/goright-128.png"
                icon.width: 24
                icon.height: 24
                enabled: false
                onClicked: slotNextUrl()
            }
            ComboBox {
                id: comboBox
                Layout.fillWidth: true
                editable: true
                model: comboBoxUrls
                onCurrentIndexChanged: slotGotoUrl(comboBox.currentIndex)
                onAccepted: {
                    var url = htmlPart.urlFromUserInput(editText);
                    if (htmlPart.urlIsValid(url)) {
                        var item = url.toString();

                        comboBoxBlockSignals = true;

                        var index = comboBoxFind(item);
                        if (index !== -1) {
                            comboBoxUrls.splice(index, 1);
                        }

                        comboBoxUrls.unshift(item);

                        comboBox.model = comboBoxUrls;
                        comboBox.currentIndex = 0;

                        comboBoxBlockSignals = false;

                        slotGotoUrl(comboBox.currentIndex);
                    }
                }
            }
            ToolButton {
                icon.source: "qrc:/resources/images/open-128.png"
                icon.width: 24
                icon.height: 24
                onClicked: slotOpenUrl()
            }
        }
    }

    //AFA: QtHtmlQml basic struct
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

    //AFA: this not work
    //Connections {
    //    target: {
    //        htmlPart.view = htmlView;
    //        return htmlPart.browserExtension();
    //    }

    //    onOpenUrlRequest: {
    //        htmlPart.setArguments(arguments);
    //        htmlPart.browserExtension().setBrowserArguments(browserArguments);
    //        htmlPart.openUrl(url);
    //    }
    //}

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

        onOpenUrlRequest: {
            htmlPart.openUrl(url);
        }

        onUrlChanged: {
            slotUrlChanged(url, prevUrl);
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
            if (htmlPart.urlIsValid(fds.fileUrl)) {
                htmlPart.openUrl(fds.fileUrl);
            }
        }
    }

    //function do something
    function updateButton()
    {
        toolButtonPrev.enabled = (prevUrls.length > 0);
        toolButtonNext.enabled = (nextUrls.length > 0);
    }

    function comboBoxFind(item)
    {
        for (var index = 0; index < comboBoxUrls.length; index++) {
            if (comboBoxUrls[index] === item) {
                return index;
            }
        }
        return -1;
    }

    function slotUrlChanged(url, prevUrl)
    {
        var item = url.toString();

        comboBoxBlockSignals = true;

        var index = comboBoxFind(item);
        if (index !== -1) {
            comboBoxUrls.splice(index, 1);
        }

        comboBoxUrls.unshift(item);

        comboBox.model = comboBoxUrls;
        comboBox.currentIndex = 0;

        comboBoxBlockSignals = false;

        //
        if (canAddPrev && htmlPart.urlIsValid(prevUrl)) {
            console.log("prevUrls enqueue url=", prevUrl);

            prevUrls.push(prevUrl);

            updateButton();
        }

        canAddPrev = true;
    }

    function slotPrevUrl()
    {
        if (prevUrls.length > 0) {
            var url1 = prevUrls.shift();

            var url2 = htmlPart.url();
            if (htmlPart.urlIsValid(url2)) {
               console.log("nextUrls enqueue url=", url2);

                nextUrls.push(url2);
            }

            canAddPrev = false;

            htmlPart.openUrl(url1);

            updateButton();
        }
    }

    function slotNextUrl()
    {
        if (nextUrls.length > 0) {
            var url = nextUrls.shift();

            htmlPart.openUrl(url);

            updateButton();
        }
    }

    function slotGotoUrl(index)
    {
        if (comboBoxBlockSignals) {
            return;
        }

        if (index > comboBoxUrls.length) {
            return;
        }

        var url = htmlPart.urlFromUserInput(comboBoxUrls[index]);
        if (htmlPart.urlIsValid(url)) {
            htmlPart.openUrl(url);
        }
    }

    function slotOpenUrl()
    {
        fds.open();
    }
}
