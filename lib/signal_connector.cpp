#include "signal_connector.h"

#include "content/shell/shell.h"
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QDebug>

SignalConnector::SignalConnector(content::Shell* shell, QWidget* window)
	: m_shell(shell)
	, m_window(window)
{
	setParent(window);
	m_addressLineEdit = m_window->findChild<QLineEdit*>("AddressLineEdit");
	m_backButton = m_window->findChild<QToolButton*>("BackButton");
	m_forwardButton = m_window->findChild<QToolButton*>("ForwardButton");
	m_reloadButton = m_window->findChild<QToolButton*>("ReloadButton");

	connect(m_addressLineEdit, SIGNAL(returnPressed()), this, SLOT(loadAddressFromAddressBar()));
	connect(m_backButton, SIGNAL(clicked()), this, SLOT(goBack()));
	connect(m_forwardButton, SIGNAL(clicked()), this, SLOT(goForward()));
	connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
}

void SignalConnector::loadAddressFromAddressBar()
{
	load(m_addressLineEdit->text());
}

void SignalConnector::load(const QString& url) const
{
	GURL gurl(url.toStdString());
	if (!gurl.has_scheme())
		gurl = GURL(std::string("http://") + url.toStdString());
	m_shell->LoadURL(gurl);
}

void SignalConnector::goBack() const
{
	m_shell->GoBackOrForward(-1);
}

void SignalConnector::goForward() const
{
	m_shell->GoBackOrForward(1);
}

void SignalConnector::reload() const
{
	m_shell->Reload();
}

