#ifndef QT_SIGNAL_CONNECTOR_H
#define QT_SIGNAL_CONNECTOR_H

#include <QObject>

class QWidget;
class QToolButton;
class QLineEdit;

namespace content {
	class Shell;
}

class SignalConnector : public QObject
{
	Q_OBJECT
public:
	SignalConnector(content::Shell* shell, QWidget* window);

public Q_SLOTS:
	void loadAddressFromAddressBar();
	void load(const QString& url) const;
	void goBack() const;
	void goForward() const;
	void reload() const;

private:
	content::Shell* m_shell;
	QWidget* m_window;

	QLineEdit* m_addressLineEdit;
	QToolButton* m_forwardButton;
	QToolButton* m_backButton;
	QToolButton* m_reloadButton;
};

#endif
