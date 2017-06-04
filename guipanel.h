#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <QWidget>
#include <QtSerialPort/qserialport.h>
#include "qremotetiva.h"
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <QMessageBox>

namespace Ui {
class GUIPanel;
}
extern "C" {
#include "protocol.h"    // Cabecera de funciones de gestión de tramas; se indica que está en C, ya que QTs
// se integra en C++, y el C puede dar problemas si no se indica.
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

    void on_rojo_clicked();
    void on_verde_clicked();
    void on_azul_clicked();
    void on_ModeButton_clicked();
    void on_pushButton_clicked();
    void on_checkSWs_clicked();
    void on_colorWheel_colorChanged(const QColor &arg1);
    void on_Interrupts_clicked(bool checked);

    void tivaStatusChanged(int status,QString message);
    void pingResponseReceived(void);
    void CommandRejected(int16_t code);
    void SwitchModeReceived(int modo);
    void RequestReceived(uint8_t s1, uint8_t s2);
    void IntensityReceived(float x);
    void ColourReceived(int rojo,int azul,int verde);
    void LedsReceived(uint8_t a,uint8_t b,uint8_t c);

    // Segunda parte
    void procesaDatoADC(uint16_t chan1,uint16_t chan2,uint16_t chan3,uint16_t chan4);
    void on_TimerADC_clicked(bool checked);
    void on_frecuencia_valueChanged(double value);


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
    QMessageBox ventanaPopUp;

    //SEMANA2: Para las graficas
    double xVal[1024]; //valores eje X
    double yVal[4][1024]; //valores ejes Y
    QwtPlotCurve *Channels[4]; //Curvas
    QwtPlotGrid  *m_Grid; //Cuadricula
};

#endif // GUIPANEL_H
