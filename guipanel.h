#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <QWidget>
#include <QtSerialPort/qserialport.h>
#include "qremotetiva.h"

namespace Ui {
class GUIPanel;
}

//QT4:QT_USE_NAMESPACE_SERIALPORT

class GUIPanel : public QWidget
{
    Q_OBJECT
    
public:
    //GUIPanel(QWidget *parent = 0);
    explicit GUIPanel(QWidget *parent = 0);
    ~GUIPanel(); // Da problemas
    
private slots:
    //void on_pingButton_clicked();
    void on_runButton_clicked();
    //void readRequest();
    void on_serialPortComboBox_currentIndexChanged(const QString &arg1);

    void on_rojo_stateChanged(int arg1);
    void on_verde_stateChanged(int arg1);
    void on_azul_stateChanged(int arg1);
    void on_pushButton_clicked();    

    void tivaStatusChanged(int status,QString message);
    void pingResponseReceived(void);
    void CommandRejected(int16_t code);

    void on_colorWheel_colorChanged(const QColor &arg1);

private: // funciones privadas
    void pingDevice();
    //void startSlave();
    void processError(const QString &s);
    void activateRunButton();
    void cambiaLEDs();
private:
    Ui::GUIPanel *ui;
    int transactionCount;    
    QRemoteTIVA tiva;

};

#endif // GUIPANEL_H
