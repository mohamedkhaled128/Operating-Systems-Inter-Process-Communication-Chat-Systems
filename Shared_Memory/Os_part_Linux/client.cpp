#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

#include <string>
#include <vector>

void writeMessageToSharedMemory(const std::string&);
std::vector<std::string> readMessagesFromSharedMemory();

class ChatWindow : public QMainWindow {
public:
    ChatWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        QWidget* central = new QWidget(this);
        QVBoxLayout* vbox = new QVBoxLayout();

        chatView = new QTextEdit(this);
        chatView->setReadOnly(true);
        vbox->addWidget(chatView);

        QHBoxLayout* hbox = new QHBoxLayout();
        input = new QLineEdit(this);
        sendBtn = new QPushButton("Send", this);
        hbox->addWidget(input);
        hbox->addWidget(sendBtn);

        vbox->addLayout(hbox);
        central->setLayout(vbox);
        setCentralWidget(central);

        setWindowTitle("Local Chat Client");
        resize(600, 400);

        connect(sendBtn, &QPushButton::clicked, this, &ChatWindow::sendMessage);
        connect(input, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);

        timer = new QTimer(this);
        timer->setInterval(500);
        connect(timer, &QTimer::timeout, this, &ChatWindow::updateMessages);
        timer->start();
    }

private:
    void sendMessage() {
        QString text = input->text().trimmed();
        if (text.isEmpty()) return;
        writeMessageToSharedMemory(text.toStdString());
        input->clear();
    }

    void updateMessages() {
        auto messages = readMessagesFromSharedMemory();
        for (const auto& m : messages)
            chatView->append(QString::fromStdString(m));
    }

    QTextEdit* chatView;
    QLineEdit* input;
    QPushButton* sendBtn;
    QTimer* timer;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ChatWindow w;
    w.show();
    return app.exec();
}
