#ifndef QPDFDOCUMENT_H
#define QPDFDOCUMENT_H

#include <QObject>
#include "qtpdfglobal.h"

class QPdfDocumentPrivate;

class Q_PDF_EXPORT QPdfDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pageCount READ pageCount FINAL)
public:

    enum Error {
        NoError,
        UnknownError,
        FileNotFoundError,
        InvalidFileFormatError,
        IncorrectPasswordError,
        UnsupportedSecuritySchemeError
    };

    explicit QPdfDocument(QObject *parent = 0);
    ~QPdfDocument();

    Error load(const QString &fileName, const QString &password = QString());

    int pageCount() const;

private:
    QScopedPointer<QPdfDocumentPrivate> d;
};

#endif // QPDFDOCUMENT_H
