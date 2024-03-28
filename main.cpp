#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <utility>


// Класс для суммирования чисел из файла
class SumWorker : public QObject {
Q_OBJECT

public slots:

    void process(const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit resultReady(0);
        }

        QTextStream in(&file);
        QStringList numbers;
        QString line;
        qint64 sum = 0;

        while (!in.atEnd()) {
            line = in.readLine();
            numbers = line.trimmed().split(" ", Qt::SkipEmptyParts);
            sum += findLocalSum(numbers);
        }

        file.close();

        emit resultReady(sum);
    }

signals:

    void resultReady(const qint64 &sum);

private:
    static qint64 findLocalSum(const QStringList &list) {
        qint64 localSum = 0;
        for (auto &element: list) {
            localSum += element.toLongLong();
        }
        return localSum;
    }
};

// Класс для выполнения операции XOR над числами в файле
class XORWorker : public QObject {
Q_OBJECT

public slots:

    void process(const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit resultReady(0);
        }

        QTextStream in(&file);
        QStringList numbers;
        QString line;
        qint64 resXor = 0;

        while (!in.atEnd()) {
            line = in.readLine();
            numbers = line.trimmed().split(" ", Qt::SkipEmptyParts);
            findLocalXor(numbers, resXor);
        }

        file.close();

        emit resultReady(resXor);
    }

signals:

    void resultReady(const qint64 &sum);

private:
    static void findLocalXor(const QStringList &list, qint64 &curValue) {
        for (auto &element: list) {
            curValue = curValue ^ element.toLongLong();
        }
    }
};

// Класс для вычитания остальных чисел из первого числа в файле в отдельном потоке
class SubtractWorker : public QObject {
Q_OBJECT

public slots:

    void process(const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit resultReady(0);
        }

        QTextStream in(&file);
        QString line;
        QStringList numbers;

        qint64 resSub = 0;

        if (!in.atEnd()) {
            line = in.readLine();
            numbers = line.trimmed().split(" ", Qt::SkipEmptyParts);
            resSub = numbers[0].toLongLong();
            numbers.removeAt(0);
            findLocalSubtract(numbers, resSub);
        }

        while (!in.atEnd()) {
            line = in.readLine();
            numbers = line.trimmed().split(" ", Qt::SkipEmptyParts);
            findLocalSubtract(numbers, resSub);
        }

        file.close();

        emit resultReady(resSub);
    }

signals:

    void resultReady(const qint64 &sum);

private:
    static void findLocalSubtract(const QStringList &list, qint64 &curValue) {
        for (auto &element: list) {
            curValue -= element.toLongLong();
        }
    }
};

// Класс, запускающий потоки, принимающий результат обработки
class Controller : public QObject {
Q_OBJECT
    QThread sumThread, xorThread, subThread;
    SumWorker *sumWorker;
    XORWorker *xorWorker;
    SubtractWorker *subtractWorker;

public:
    Controller() {

        sumWorker = new SumWorker;
        xorWorker = new XORWorker;
        subtractWorker = new SubtractWorker;

        sumWorker->moveToThread(&sumThread);
        xorWorker->moveToThread(&xorThread);
        subtractWorker->moveToThread(&subThread);

        connect(&sumThread, &QThread::finished, sumWorker, &QObject::deleteLater);
        connect(this, &Controller::findSum, sumWorker, &SumWorker::process);
        connect(sumWorker, &SumWorker::resultReady, this, &Controller::handleSumResult);

        connect(&xorThread, &QThread::finished, xorWorker, &QObject::deleteLater);
        connect(this, &Controller::findXor, xorWorker, &XORWorker::process);
        connect(xorWorker, &XORWorker::resultReady, this, &Controller::handleXorResult);

        connect(&subThread, &QThread::finished, subtractWorker, &QObject::deleteLater);
        connect(this, &Controller::findXor, subtractWorker, &SubtractWorker::process);
        connect(subtractWorker, &SubtractWorker::resultReady, this,
                &Controller::handleSubtractionResult);

        sumThread.start();
        xorThread.start();
        subThread.start();
    }

    void handleFile(QString fileName) {
        emit findSum(fileName);
        emit findXor(fileName);
        emit findSubtraction(fileName);
    }


public slots:

    void handleSumResult(const qint64 &sum) {
        qDebug() << "Sum is " << sum;
        sumThread.quit();
    };

    void handleXorResult(const qint64 &xorRes) {
        qDebug() << "XOR is " << xorRes;
        xorThread.quit();
    }

    void handleSubtractionResult(const qint64 &subRes) {
        qDebug() << "The difference of the first number and all others is " << subRes;
        subThread.quit();
    }

signals:

    void findSum(const QString &);

    void findXor(const QString &);

    void findSubtraction(const QString &);
};


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString defaultFileName = "numbers.txt";
    QString fileName;

    if (argc > 1){
        fileName = a.arguments().at(1);
    } else {
        qDebug() << "Default filename: numbers.txt";
        fileName = defaultFileName;
    }

    qDebug() << "Start app";

    Controller controller;
    controller.handleFile(fileName);

    return QCoreApplication::exec();
}

#include "main.moc"