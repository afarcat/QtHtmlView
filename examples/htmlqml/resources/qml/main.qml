/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

import QtQuick 2.7
import QtQuick.Window 2.2

Window {
    visible: true
    width: Screen.width*0.75
    height: Screen.height*0.75

    HtmlPage {
        anchors.fill: parent
    }
}
