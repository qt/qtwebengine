// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

function updateTitle()
{
    var inp = document.getElementById("upfile");
    var allfiles = new String("");
    var name = new String("");
    for (var i = 0; i < inp.files.length; ++i) {
        name = inp.files.item(i).name;
        if (allfiles.length == 0)
            allfiles = name;
        else
            allfiles = allfiles + "," + name;
    }
    document.title = allfiles;
}
