#ifndef QREMOTETIVA_H
#define QREMOTETIVA_H

#include <QObject>
#include <QSerialPort>      // Comunicacion por el puerto serie
#include <QSerialPortInfo>  // Comunicacion por el puerto serie


#include<stdint.h>      // Cabecera para usar tipos de enteros con tamaño
#include<stdbool.h>     // Cabecera para usar booleanos

extern "C" {
#include <protocol.h>
}

class QRemoteTIVA : public QObject
{

    Q_OBJECT
public:
    explicit QRemoteTIVA(QObject *parent = 0);

    //Define una serie de etiqueta para los errores y estados notificados por la señal statusChanged(...)
    enum TivaStatus {TivaConnected,
                     TivaDisconnected,
                     OpenPortError,
                     BaudRateError,
                     DataBitError,
                     ParityError,
                     StopError,
                     FlowControlError,
                     UnexpectedPacketError,
                     FragmentedPacketError,
                     ReceivedDataError
                    };
    Q_ENUM(TivaStatus)

    //Metodo publico
    QString getLastErrorMessage();

signals:
    void statusChanged(int status, QString message); //Esta señal se genera al realizar la conexión/desconexion o cuando se produce un error de comunicacion
    void pingReceivedFromTiva(void); //Esta señal se genera al recibir una respuesta de PING de la TIVA
    void commandRejectedFromTiva(int16_t code); //Esta señal se genera al recibir una respuesta de Comando Rechazado desde la TIVA
    void commandSwitchMode(int x); // Esta señal es el comando realizado con el cambio de modo GPIO / PWM
    void commandLEDs(uint8_t x,uint8_t y ,uint8_t z);
    void RequestReceivedTIVA(uint8_t x,uint8_t y);
    void IntensityWheel(float intensity);
    void ColourWheel(int rojo,int verde,int azul);

    // Semana 2
    void commandADCReceived(uint16_t chan1,uint16_t chan2,uint16_t chan3,uint16_t chan4);
public slots:
    void startSlave(QString puerto); //Este Slot arranca la comunicacion
    void pingTiva(void); //Este Slot provoca el envio del comando PING
    void LEDsGpioTiva(bool red, bool green, bool blue); //Este Slot provoca el envio del comando LED
    void LEDPwmBrightness(double value); //Este Slot provoca el envio del comando Brightness
    void LEDColour(int rojo,int verde,int azul);
    void SwitchMode(bool y); // Funcion que cambia de modo GPIO / PWM
    void SwitchInterrupts(bool y);
    void RequestTiva();

    // Segunda parte
    // Este Slot permite ordenar al objeto TIVA que envie un comando de conversion
    void ADCSample(void);
private slots:
    void readRequest(); //Este Slot se conecta a la señal readyRead(..) del puerto serie. Se encarga de procesar y decodificar los mensajes que llegan de la TIVA y
                        //generar señales para algunos de ellos.

private:
    QSerialPort serial;
    QString LastError;
    bool connected;
    QByteArray request;

};

#endif // QREMOTETIVA_H
