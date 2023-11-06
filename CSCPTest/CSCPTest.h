#ifndef CSCPTEST_H
#define CSCPTEST_H

#include <QWidget>
#include "CSCP/CSCPManager.h"
#include "ShowMsg/ShowMsg.h"
#include "NewConnect/NewConnect.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CSCPTest; }
QT_END_NAMESPACE

class CSCPTest : public QWidget
{
    Q_OBJECT

public:
    explicit CSCPTest(QWidget *parent = nullptr);
    ~CSCPTest() override;

private:
    Ui::CSCPTest *ui;
    CSCPManager *cscpManager = nullptr;
    NewConnect *newConnect = nullptr;
    void bind();
    void enableOperateBtn();
    void connected(void *);
    void showMsg();
    void closeConnect();
    void disconnected();
    void appendLog(const QByteArray &);
    void connectFail(const QHostAddress &, unsigned short, const QByteArray &);
    void toConnect(const QByteArray &, unsigned short);
    QMap<QString, ShowMsg*> connectList;
};
#endif // CSCPTEST_H
