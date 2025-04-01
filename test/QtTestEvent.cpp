#include <QtTest/QtTest>

class QtTestEvent : public QObject {
    Q_OBJECT

private slots:
    void testExample() {
        QVERIFY(true); // 示例测试，始终通过
    }
};

QTEST_MAIN(QtTestEvent)
#include "QtTestEvent.moc"
