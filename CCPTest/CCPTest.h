#ifndef CCPTEST_H
#define CCPTEST_H

#include <QWidget>
#include "CCP/CCPManager.h"
#include "ShowMsg/ShowMsg.h"
#include "NewConnect/NewConnect.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CCPTest; }
QT_END_NAMESPACE

class CCPTest : public QWidget
{
    Q_OBJECT

public:
    explicit CCPTest(QWidget * = nullptr);
    ~CCPTest() override;

private:
    Ui::CCPTest *ui{};
    CCPManager *ccpManager = nullptr;
    NewConnect *newConnect = nullptr;
    void bind();
    void enableOperateBtn();
    void connected(CCP *);
    void showMsg();
    void closeConnect();
    void disconnected();
    void appendLog(const QString &);
    void connectFail(const QHostAddress &, unsigned short, const QByteArray &);
    void toConnect(const QByteArray &, unsigned short);
    QMap<QString, ShowMsg*> connectList;
    void closeEvent(QCloseEvent *) override;
};
#endif // CCPTEST_H
