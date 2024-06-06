#pragma once

#include <QWidget>
#include "CFUP/CFUPManager.h"
#include "ShowMsg/ShowMsg.h"
#include "NewConnect/NewConnect.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CFUPTest; }
QT_END_NAMESPACE

class CFUPTest : public QWidget
{
    Q_OBJECT

public:
    explicit CFUPTest(QWidget * = nullptr);
    ~CFUPTest() override;

private:
    Ui::CFUPTest *ui{};
    CFUPManager *cfupManager = nullptr;
    NewConnect *newConnect = nullptr;
    void bind();
    void enableOperateBtn();
    void connected(CFUP *);
    void showMsg();
    void closeConnect();
    void disconnected();
    void appendLog(const QString &);
    void connectFail(const QHostAddress &, unsigned short, const QByteArray &);
    void toConnect(const QByteArray &, unsigned short);
    QMap<QString, ShowMsg*> connectList;
    void closeEvent(QCloseEvent *) override;
};
