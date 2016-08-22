#include "widget.h"

#include <QPushButton>
#include <QLineEdit>
#include <QIcon>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include <QListWidget>
#include <QListWidgetItem>
#include <QByteArray>
#include <QMessageBox>

#include <vector>
#include <iterator>
#include "../../source/mimeapps.h"
#include "../../source/desktopfile.h"

using namespace mimeapps;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    _lastDirectory = QDir::homePath();

    _urlInput = new QLineEdit;
    _urlInput->setPlaceholderText("File path or URL...");

    QPushButton* chooseFile = new QPushButton(QIcon::fromTheme("document-open"), "Choose file");
    connect(chooseFile, SIGNAL(clicked()), SLOT(fileDialog()));

    QLabel* mimeTypeLabel = new QLabel("MIME-type hint:");
    _mimeTypeHint = new QComboBox;
    _mimeTypeHint->setEditable(true);

    const char* mimeTypes[] = {"text/plain", "text/xml", "text/html",
                               "image/jpeg", "image/png", "application/pdf",
                               "application/zip", "application/x-compressed-tar",
                               "audio/x-wav", "video/x-msvideo",
                               "x-scheme-handler/http", "x-scheme-handler/https"
                              };
    for (size_t i=0; i<sizeof(mimeTypes)/sizeof(const char*); ++i) {
        QString name(mimeTypes[i]);
        QString iconName = QString(name).replace('/', '-');
        QString genericIconName = QString(name).replace(QRegExp("/\\w+"), "-x-generic");
        QIcon icon = QIcon::fromTheme(iconName, QIcon::fromTheme(genericIconName, QIcon::fromTheme("unknown")));
        _mimeTypeHint->addItem(icon, name);
    }

    mimeTypeLabel->setBuddy(_mimeTypeHint);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(_urlInput);
    hbox->addWidget(chooseFile);
    hbox->addWidget(mimeTypeLabel);
    hbox->addWidget(_mimeTypeHint);

    _appList = new QListWidget;

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(_appList);

    listApplications(_mimeTypeHint->currentText());
    connect(_mimeTypeHint, SIGNAL(activated(QString)), SLOT(listApplications(QString)));
    connect(_appList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(spawnApplication(QListWidgetItem*)));

    setLayout(vbox);
}

Widget::~Widget()
{

}

void Widget::fileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose file", _lastDirectory);
    if (!filePath.isEmpty()) {
        _lastDirectory = QFileInfo(filePath).path();
        _urlInput->setText(filePath);
    }
}

void Widget::listApplications(QString mimeType)
{
    QByteArray arr = mimeType.toUtf8();
    std::vector<DesktopFile> vec;
    findAssociatedApplications(std::string(arr.begin(), arr.end()), std::back_inserter(vec));

    _appList->clear();
    for (size_t i=0; i<vec.size(); ++i) {
        QString text = QString::fromUtf8(vec[i].name().c_str()) + " (" + QString::fromUtf8(vec[i].fileName().c_str()) + ")";

        QListWidgetItem* item = new QListWidgetItem(QIcon::fromTheme(QString::fromUtf8(vec[i].icon().c_str()), QIcon::fromTheme("application-x-desktop")), text);
        QString tooltip = QString::fromUtf8(vec[i].comment().c_str());
        if (tooltip.isEmpty()) {
            tooltip = QString::fromUtf8(vec[i].genericName().c_str());
        }
        item->setToolTip(tooltip);
        item->setData(Qt::UserRole, QString::fromUtf8(vec[i].fileName().c_str()));
        _appList->addItem(item);
    }
}

void Widget::spawnApplication(QListWidgetItem *item)
{
    QString arg = _urlInput->text();
    if (arg.isEmpty()) {
        QMessageBox::warning(this, "Argument is not set", "You must provide file path or URL");
        return;
    }

    QByteArray arr = item->data(Qt::UserRole).toString().toUtf8();
    std::string fileName(arr.begin(), arr.end());
    DesktopFile file(fileName);
    if (file.isValid()) {
        arr = arg.toUtf8();
        try {
            file.spawnApplication(std::string(arr.begin(), arr.end()));
        } catch(std::exception& e) {
            QMessageBox::critical(this, "Spawn error", QString::fromUtf8(e.what()));
        }
    } else {
        QMessageBox::critical(this, "Desktop file error", "Could not read .desktop file");
    }
}
