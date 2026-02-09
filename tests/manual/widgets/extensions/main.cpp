// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QAbstractListModel>
#include <QApplication>
#include <QDirListing>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QWebEngineProfileBuilder>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWidget>

#include <QWebEngineExtensionManager>
#include <QWebEngineExtensionInfo>

// Test for extension management APIs using QWebEngineExtensionManager
// and QWebEngineExtensionInfo

using namespace Qt::StringLiterals;

class ExtensionsListModel : public QAbstractListModel
{
public:
    ExtensionsListModel(const QList<QWebEngineExtensionInfo> &extensions)
        : m_extensionsList(std::move(extensions))
    {
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_extensionsList.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();
        switch (role) {
        case Qt::DisplayRole: {
            auto extension = m_extensionsList.at(index.row());
            QString enabled = extension.isEnabled() ? " enabled" : " disabled";
            return extension.name() + enabled + "\nId: " + extension.id()
                    + "\npath: " + extension.path();
        }
        case Qt::UserRole: {
            QWebEngineExtensionInfo extension = m_extensionsList.at(index.row());
            return QVariant::fromValue(extension);
        }
        default:
            break;
        }
        return QVariant();
    }

private:
    QList<QWebEngineExtensionInfo> m_extensionsList;
};

class ExtensionsWidget : public QWidget
{
public:
    ExtensionsWidget(QWebEngineProfile *profile, QWebEngineExtensionManager *manager)
        : m_profile(profile), m_extensionManager(manager)
    {
        update();

        setLayout(new QVBoxLayout);
        m_extensionsView.setSpacing(2);
        m_extensionsView.setWrapping(true);
        layout()->addWidget(&m_extensionsView);

        auto *actionsBtn = new QPushButton("open actions menu");
        QObject::connect(actionsBtn, &QPushButton::clicked, this,
                         &ExtensionsWidget::openActionsMenu);
        layout()->addWidget(actionsBtn);

        auto *enabeBtn = new QPushButton("enable selected");
        QObject::connect(enabeBtn, &QPushButton::clicked, this, &ExtensionsWidget::enable);
        layout()->addWidget(enabeBtn);

        auto *disableBtn = new QPushButton("disable selected");
        QObject::connect(disableBtn, &QPushButton::clicked, this, &ExtensionsWidget::disable);
        layout()->addWidget(disableBtn);

        auto *loadBtn = new QPushButton("load unpacked");
        QObject::connect(loadBtn, &QPushButton::clicked, this, &ExtensionsWidget::loadUnpacked);
        layout()->addWidget(loadBtn);

        auto *installBtn = new QPushButton("install packed");
        QObject::connect(installBtn, &QPushButton::clicked, this, [this]() { install(true); });
        layout()->addWidget(installBtn);

        auto *installUnpackedBtn = new QPushButton("install unpacked");
        QObject::connect(installUnpackedBtn, &QPushButton::clicked, this,
                         [this]() { install(false); });
        layout()->addWidget(installUnpackedBtn);

        auto *unLoadBtn = new QPushButton("unload");
        QObject::connect(unLoadBtn, &QPushButton::clicked, this, &ExtensionsWidget::unload);
        layout()->addWidget(unLoadBtn);

        auto *uninstallBtn = new QPushButton("uninstall");
        QObject::connect(uninstallBtn, &QPushButton::clicked, this, &ExtensionsWidget::uninstall);
        layout()->addWidget(uninstallBtn);

        QObject::connect(m_extensionManager, &QWebEngineExtensionManager::loadFinished,
                         [this](QWebEngineExtensionInfo extension) {
                             if (!extension.isLoaded()) {
                                 showInfoDialog("Failed to load extension\n\nFile:"
                                                + extension.path()
                                                + "\nError: " + extension.error());
                                 return;
                             }
                             m_extensionManager->setExtensionEnabled(extension, true);
                             showInfoDialog("Extension loaded\n\nName:" + extension.name()
                                            + "\nFile: " + extension.path());
                             update();
                         });

        QObject::connect(m_extensionManager, &QWebEngineExtensionManager::installFinished,
                         [this](QWebEngineExtensionInfo extension) {
                             if (!extension.isInstalled()) {
                                 showInfoDialog("Failed to install extension\n\nFile: "
                                                + extension.path()
                                                + "\nError: " + extension.error());
                                 return;
                             }
                             showInfoDialog("Extension installed\n\nName:" + extension.name()
                                            + "\nFile: " + extension.path());
                             m_extensionManager->setExtensionEnabled(extension, true);
                             update();
                         });
        QObject::connect(m_extensionManager, &QWebEngineExtensionManager::unloadFinished,
                         [this](QWebEngineExtensionInfo extension) {
                             if (!extension.error().isEmpty()) {
                                 showInfoDialog("Failed to unload " + extension.name()
                                                + "\n\nFile: " + extension.path()
                                                + "\nError: " + extension.error());
                                 return;
                             }
                             showInfoDialog("Extension unloaded\n\nName: " + extension.name()
                                            + "\nFile: " + extension.path());
                             update();
                         });
        QObject::connect(
                m_extensionManager, &QWebEngineExtensionManager::uninstallFinished,
                [this](QWebEngineExtensionInfo extension) {
                    if (!extension.error().isEmpty()) {
                        showInfoDialog("Failed to uninstall " + extension.name() + "\n\nFile: "
                                       + extension.path() + "\nError: " + extension.error());
                        return;
                    }
                    showInfoDialog("Extension uninstalled\n\nName: " + extension.name()
                                   + "\nFile: " + extension.path());
                    update();
                });
    }

private:
    void update()
    {
        auto *oldModel = m_extensionsView.selectionModel();
        m_extensionsView.setModel(
                new ExtensionsListModel(std::move(m_extensionManager->extensions())));
        if (oldModel)
            delete oldModel;
    }

    template<typename Functor>
    void doWithCurrentExtension(Functor &&f)
    {
        QModelIndex idx = m_extensionsView.currentIndex();
        QVariant var = m_extensionsView.model()->data(idx, Qt::UserRole);
        auto *maybeExtension = get_if<QWebEngineExtensionInfo>(&var);
        if (maybeExtension) {
            f(*maybeExtension);
            update();
        } else {
            showInfoDialog("No extension is selected");
        }
    }

    void enable()
    {
        doWithCurrentExtension([this](QWebEngineExtensionInfo info) {
            m_extensionManager->setExtensionEnabled(info, true);
        });
    }

    void disable()
    {
        doWithCurrentExtension([this](QWebEngineExtensionInfo info) {
            m_extensionManager->setExtensionEnabled(info, false);
        });
    }

    void loadUnpacked()
    {
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec())
            m_extensionManager->loadExtension(dialog.selectedFiles()[0]);
    }

    void install(bool packed)
    {
        QFileDialog dialog(this);
        if (packed) {
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setNameFilter("Extensions(*.zip)");
        } else {
            dialog.setFileMode(QFileDialog::Directory);
        }
        if (dialog.exec())
            m_extensionManager->installExtension(dialog.selectedFiles()[0]);
    }

    void unload()
    {
        doWithCurrentExtension([this](QWebEngineExtensionInfo info) {
            m_extensionManager->unloadExtension(info);
        });
    }

    void uninstall()
    {
        doWithCurrentExtension([this](QWebEngineExtensionInfo info) {
            m_extensionManager->uninstallExtension(info);
        });
    }

    void openActionsMenu()
    {
        doWithCurrentExtension([this](QWebEngineExtensionInfo info) {
            const auto url = info.actionPopupUrl();
            if (url.isEmpty()) {
                showInfoDialog("No popup page set for this extension");
                return;
            }

            auto *view = new QWebEngineView(m_profile);
            view->setAttribute(Qt::WA_DeleteOnClose, true);
            view->load(url);
            view->show();
        });
    }

    void showInfoDialog(const QString &msg)
    {
        QMessageBox *msgBox = new QMessageBox;
        msgBox->setWindowModality(Qt::NonModal);
        msgBox->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        msgBox->setAttribute(Qt::WA_DeleteOnClose, true);
        msgBox->resize(400, 100);
        msgBox->setText(msg);
        QTimer::singleShot(4000, msgBox, &QMessageBox::close);
        msgBox->show();
    }

    QWebEngineProfile *m_profile;
    QWebEngineExtensionManager *m_extensionManager;
    QListView m_extensionsView;
};

QWidget *createTabPage(QWebEngineProfile *profile)
{
    auto *tabPage = new QWidget();
    auto *layout = new QHBoxLayout(tabPage);
    auto *extensionManager = profile->extensionManager();
    auto *view = new QWebEngineView(profile);
    view->setUrl(QUrl("https://www.google.com"_L1));
    layout->addWidget(view);
    layout->addWidget(new ExtensionsWidget(profile, extensionManager));
    return tabPage;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    QWebEngineProfileBuilder profileBuilder;
    std::unique_ptr<QWebEngineProfile> normalProfile{ profileBuilder.createProfile(
            "ExtensionsManualTest") };
    std::unique_ptr<QWebEngineProfile> otrProfile{ profileBuilder.createOffTheRecordProfile() };

    QMainWindow window;
    auto *tabWidget = new QTabWidget();
    window.setCentralWidget(tabWidget);

    tabWidget->addTab(createTabPage(normalProfile.get()), u"Normal Profile"_s);
    tabWidget->addTab(createTabPage(otrProfile.get()), u"Off-the-Record Profile"_s);

    window.resize(1024, 750);
    window.show();

    return app.exec();
}
