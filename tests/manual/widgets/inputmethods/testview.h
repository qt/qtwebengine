// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <QTextCharFormat>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
class QTableView;
QT_END_NAMESPACE

class TestView : public QWidget
{
    Q_OBJECT
public:
    explicit TestView(QWidget *parent = 0);

    void cancelTest();

public slots:
    void loadTestData(const QString &);
    void startOrCancelTest();
    void collectAndSendData();

signals:
    void sendInputMethodData(int, int, QTextCharFormat::UnderlineStyle, const QColor &, const QColor &, const QString &);
    void requestInputMethodEvent();

private:
    QTableView *m_tableView;
    QPushButton *m_testButton;

    bool m_testRunning;
};

#endif // TESTVIEW_H
