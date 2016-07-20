#ifndef QPDFDOCUMENT_H
#define QPDFDOCUMENT_H

#include <QObject>
#include <QImage>
#include "qtpdfglobal.h"

QT_BEGIN_NAMESPACE

class QPdfDocumentPrivate;
class QNetworkReply;

class Q_PDF_EXPORT QPdfDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged FINAL)
    Q_PROPERTY(QString password READ password WRITE setPassword FINAL)
    Q_PROPERTY(bool loading READ isLoading FINAL)
public:

    enum Error {
        NoError,
        UnknownError,
        FileNotFoundError,
        InvalidFileFormatError,
        IncorrectPasswordError,
        UnsupportedSecuritySchemeError
    };

    explicit QPdfDocument(QObject *parent = Q_NULLPTR);
    ~QPdfDocument();

    Error load(const QString &fileName);

    bool isLoading() const;

    void load(QIODevice *device);
    void setPassword(const QString &password);
    QString password() const;

    Error error() const;

    int pageCount() const;

    QSizeF pageSize(int page) const;

    QImage render(int page, const QSizeF &pageSize);

Q_SIGNALS:
    void passwordRequired();
    void documentLoadStarted();
    void documentLoadFinished();
    void pageCountChanged();

private:
    Q_PRIVATE_SLOT(d, void _q_tryLoadingWithSizeFromContentHeader())
    Q_PRIVATE_SLOT(d, void _q_copyFromSequentialSourceDevice())
    QScopedPointer<QPdfDocumentPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_H
