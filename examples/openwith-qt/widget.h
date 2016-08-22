#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QComboBox;
class QListWidget;
class QListWidgetItem;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void fileDialog();
    void listApplications(QString mimeType);
    void spawnApplication(QListWidgetItem* item);

private:
    QString _lastDirectory;
    QLineEdit* _urlInput;
    QComboBox* _mimeTypeHint;
    QListWidget* _appList;
};

#endif // WIDGET_H
