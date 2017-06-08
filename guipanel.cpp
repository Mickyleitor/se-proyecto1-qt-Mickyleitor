#include "guipanel.h"
#include "ui_guipanel.h"
#include <QSerialPort>      // Comunicacion por el puerto serie
#include <QSerialPortInfo>  // Comunicacion por el puerto serie
#include <QMessageBox>      // Se deben incluir cabeceras a los componentes que se vayan a crear en la clase
// y que no estén incluidos en el interfaz gráfico. En este caso, la ventana de PopUp <QMessageBox>
// que se muestra al recibir un PING de respuesta

#include<stdint.h>      // Cabecera para usar tipos de enteros con tamaño
#include<stdbool.h>     // Cabecera para usar booleanos



extern "C" {
#include "protocol.h"    // Cabecera de funciones de gestión de tramas; se indica que está en C, ya que QTs
// se integra en C++, y el C puede dar problemas si no se indica.
}

GUIPanel::GUIPanel(QWidget *parent) :  // Constructor de la clase
    QWidget(parent),
    ui(new Ui::GUIPanel)               // Indica que guipanel.ui es el interfaz grafico de la clase
  , transactionCount(0)
{
    ui->setupUi(this);                // Conecta la clase con su interfaz gráfico.
    setWindowTitle(tr("Interfaz de Control")); // Título de la ventana

    // Inicializa la lista de puertos serie
    ui->serialPortComboBox->clear(); // Vacía de componentes la comboBox
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // La descripción nos permite que SOLO aparezcan los interfaces tipo USB serial con el identificador de fabricante y producto de la TIVA
        if ((info.vendorIdentifier()==0x1CBE) && (info.productIdentifier()==0x0002))
        {
            ui->serialPortComboBox->addItem(info.portName());
        }
    }

    ui->serialPortComboBox->setFocus();   // Componente del GUI seleccionado de inicio


    ui->pingButton->setEnabled(false);    // Se deshabilita el botón de ping del interfaz gráfico, hasta que

    //Semana 2. Inicializacion GRAFICA
    ui->Grafica->setTitle("Sinusoides"); // Titulo de la grafica
    ui->Grafica->setAxisTitle(QwtPlot::xBottom, "Tiempo"); // Etiqueta eje X de coordenadas
    ui->Grafica->setAxisTitle(QwtPlot::yLeft, "Valor");    // Etiqueta eje Y de coordenadas
    //ui->Grafica->axisAutoScale(true); // Con Autoescala
    ui->Grafica->setAxisScale(QwtPlot::yLeft, 0, VCC_LEVEL); // Escala fija
    ui->Grafica->setAxisScale(QwtPlot::xBottom,0,1024.0);

    // Formateo de la curva
    for(int i=0;i<4;i++){
    Channels[i] = new QwtPlotCurve();
    Channels[i]->attach(ui->Grafica);
    }

    //Inicializacion de los valores básicos
    for(int i=0;i<1024;i++){
            xVal[i]=i;
            yVal[0][i]=VCC_LEVEL*(sin((double)i*2.0*3.14159/1024.0)+1.0)/2.0;
            yVal[1][i]=VCC_LEVEL*(sin((double)i*4.0*3.14159/1024.0)+1.0)/2.0;
            yVal[2][i]=VCC_LEVEL*(sin((double)i*8.0*3.14159/1024.0)+1.0)/2.0;
            yVal[3][i]=VCC_LEVEL*(sin((double)i*16.0*3.14159/1024.0)+1.0)/2.0;
    }
    Channels[0]->setRawSamples(xVal,yVal[0],1024);
    Channels[1]->setRawSamples(xVal,yVal[1],1024);
    Channels[2]->setRawSamples(xVal,yVal[2],1024);
    Channels[3]->setRawSamples(xVal,yVal[3],1024);

    Channels[0]->setPen(QPen(Qt::red)); // Color de la curva
    Channels[1]->setPen(QPen(Qt::cyan));
    Channels[2]->setPen(QPen(Qt::yellow));
    Channels[3]->setPen(QPen(Qt::green));
    m_Grid = new QwtPlotGrid();     // Rejilla de puntos
    m_Grid->attach(ui->Grafica);    // que se asocia al objeto QwtPl
    ui->Grafica->setAutoReplot(false); //Desactiva el autoreplot (mejora la eficiencia)
    //Semana 2. FIN Inicializacion GRAFICA

    //Conectamos Slots del objeto "Tiva" con Slots de nuestra aplicacion (o con widgets)
    connect(&tiva,SIGNAL(statusChanged(int,QString)),this,SLOT(tivaStatusChanged(int,QString)));
    connect(ui->pingButton,SIGNAL(clicked(bool)),&tiva,SLOT(pingTiva()));
    connect(ui->Knob,SIGNAL(valueChanged(double)),&tiva,SLOT(LEDPwmBrightness(double)));
    connect(&tiva,SIGNAL(pingReceivedFromTiva()),this,SLOT(pingResponseReceived()));
    connect(&tiva,SIGNAL(commandRejectedFromTiva(int16_t)),this,SLOT(CommandRejected(int16_t)));
    connect(&tiva,SIGNAL(commandSwitchMode(int)),this,SLOT(SwitchModeReceived(int)));
    connect(&tiva,SIGNAL(commandLEDs(uint8_t,uint8_t,uint8_t)),this,SLOT(LedsReceived(uint8_t,uint8_t,uint8_t)));
    connect(&tiva,SIGNAL(RequestReceivedTIVA(uint8_t,uint8_t)),this,SLOT(RequestReceived(uint8_t,uint8_t)));
    connect(&tiva,SIGNAL(IntensityWheel(float)),this,SLOT(IntensityReceived(float)));
    connect(&tiva,SIGNAL(ColourWheel(int,int,int)),this,SLOT(ColourReceived(int,int,int)));

    //SEMANA2: conecta las nuevas senhales y slots
    connect(&tiva,SIGNAL(commandADCReceived(uint16_t,uint16_t,uint16_t,uint16_t)),this,SLOT(procesaDatoADC(uint16_t,uint16_t,uint16_t,uint16_t)));
    connect(ui->ADCButton,SIGNAL(clicked(bool)),&tiva,SLOT(ADCSample()));

    // Alarmas personalizadas LCD
    connect(ui->triggerlevelA0,SIGNAL(sliderReleased()),this,SLOT(ChangedTriggerLevel()));
    connect(ui->triggerlevelA1,SIGNAL(sliderReleased()),this,SLOT(ChangedTriggerLevel()));
    connect(ui->triggerlevelA2,SIGNAL(sliderReleased()),this,SLOT(ChangedTriggerLevel()));
    connect(ui->triggerlevelA3,SIGNAL(sliderReleased()),this,SLOT(ChangedTriggerLevel()));

    connect(ui->lcdAlarmDummy,SIGNAL(checkChanged(bool)),this,SLOT(ChangeAlarmEventFlags()));

    // Inicialización LCDAlarmas
    ui->lcdtriggerlevelA0->display((double)VCC_LEVEL);
    ui->lcdtriggerlevelA1->display((double)VCC_LEVEL);
    ui->lcdtriggerlevelA2->display((double)VCC_LEVEL);
    ui->lcdtriggerlevelA3->display((double)VCC_LEVEL);

    //CAMBIO!!!: Inicializa la ventana
    ventanaPopUp.setIcon(QMessageBox::Information);
    ventanaPopUp.setText(tr("Status: RESPUESTA A PING RECIBIDA"));
    ventanaPopUp.setStandardButtons(QMessageBox::Ok);
    ventanaPopUp.setWindowTitle(tr("Evento"));
    ventanaPopUp.setParent(this,Qt::Popup);

}

GUIPanel::~GUIPanel() // Destructor de la clase
{
    delete ui;   // Borra el interfaz gráfico asociado a la clase
}

void GUIPanel::on_serialPortComboBox_currentIndexChanged(const QString &arg1)
{
    QString arreglobasura = arg1; // Pequeño arreglo para quitar advertencias de que no se usa arg1 (No me gustan las advertencias)
    activateRunButton();
}

// Funcion auxiliar de procesamiento de errores de comunicación
void GUIPanel::processError(const QString &s)
{
    activateRunButton(); // Activa el botón RUN
    // Muestra en la etiqueta de estado la razón del error (notese la forma de pasar argumentos a la cadena de texto)
    ui->statusLabel->setText(tr("Status: Not running, %1.").arg(s));
}

// Funcion de habilitacion del boton de inicio/conexion
void GUIPanel::activateRunButton()
{
    ui->runButton->setEnabled(true);
}

//Este Slot lo hemos conectado con la señal statusChange de la TIVA, que se activa cuando
//El puerto se conecta o se desconecta y cuando se producen determinados errores.
//Esta función lo que hace es procesar dichos eventos
void GUIPanel::tivaStatusChanged(int status,QString message)
{
    switch (status)
    {
        case QRemoteTIVA::TivaConnected:

            //Caso conectado..
            // Deshabilito el boton de conectar
            ui->runButton->setEnabled(false);

            // Se indica que se ha realizado la conexión en la etiqueta 'statusLabel'
            ui->statusLabel->setText(tr("Ejecucion, conectado al puerto %1.")
                             .arg(ui->serialPortComboBox->currentText()));

            //    // Y se habilitan los controles deshabilitados
            ui->pingButton->setEnabled(true);
            ui->pushButton->setText(tr("Borrar Mensaje"));

        break;

        case QRemoteTIVA::TivaDisconnected:
            //Caso desconectado..
            // Rehabilito el boton de conectar
            ui->runButton->setEnabled(false);
        break;
        case QRemoteTIVA::UnexpectedPacketError:
        case QRemoteTIVA::FragmentedPacketError:
            //Errores detectados en la recepcion de paquetes
            ui->statusLabel->setText(message);
            ui->pushButton->setText(tr("Borrar Mensaje"));
        default:
            //Otros errores (por ejemplo, abriendo el puerto)
            processError(tiva.getLastErrorMessage());
    }
}

// SLOT asociada a pulsación del botón RUN
void GUIPanel::on_runButton_clicked()
{
    //Intenta arrancar la comunicacion con la TIVA
    tiva.startSlave( ui->serialPortComboBox->currentText());
}

//Slot asociado al chechbox rojo (por nombre)
void GUIPanel::on_rojo_clicked()
{
    cambiaLEDs();
}

//Slot asociado al chechbox verde (por nombre)
void GUIPanel::on_verde_clicked()
{
    cambiaLEDs();
}

//Slot asociado al chechbox azul (por nombre)
void GUIPanel::on_azul_clicked()
{
    cambiaLEDs();
}

void GUIPanel::on_ModeButton_clicked()
{
    tiva.SwitchMode(ui->ModeButton->isChecked());
}

void GUIPanel::SwitchModeReceived(int modo)
{
    bool m = true;
    if(modo==1) m=false;
    ui->ModeButton->setChecked(m);
}

void GUIPanel::cambiaLEDs(void)
{
    //Invoca al metido LEDsGpioTiva para enviar la orden a la TIVA
    tiva.LEDsGpioTiva(ui->rojo->isChecked(),ui->verde->isChecked(),ui->azul->isChecked());
}

void GUIPanel::ChangedTriggerLevel(){
    ui->lcdtriggerlevelA0->display((double)(ui->triggerlevelA0->value())*(VCC_LEVEL/100));
    ui->lcdtriggerlevelA1->display((double)(ui->triggerlevelA1->value())*(VCC_LEVEL/100));
    ui->lcdtriggerlevelA2->display((double)(ui->triggerlevelA2->value())*(VCC_LEVEL/100));
    ui->lcdtriggerlevelA3->display((double)(ui->triggerlevelA3->value())*(VCC_LEVEL/100));


    // PASAR MASCARA A TIVA PARA QUE LA TIVA SE ENCARGA DE PROGRAMAR LAS ALARMAS
}

//Slots Asociado al boton que limpia los mensajes del interfaz
void GUIPanel::on_pushButton_clicked()
{
    ui->statusLabel->setText(tr(""));
    ui->pushButton->setText("Estado");
}

//**** Slots asociados a la recepción de mensajes desde la TIVA ********/
//Están conectados a señales generadas por el objeto TIVA de clase QRemoteTiva (se conectan en el constructor de la ventana, más arriba en este fichero))
//Se pueden añadir los que se quieran para completar la aplicación

//Este se ejecuta cuando se recibe una respuesta de PING
void GUIPanel::pingResponseReceived()
{
    //CAMBIO: La ventana PoP Up ahora esta declarada como componente de la clase GUIPANEL. Se configura en el constructor
    // Ventana popUP para el caso de comando PING; no te 2deja definirla en un "caso"
    ventanaPopUp.setStyleSheet("background-color: lightg0rey");
    ventanaPopUp.setModal(true); //CAMBIO: Se sustituye la llamada a exec(...) por estas dos.
    ventanaPopUp.show();
}


//Este se ejecuta cuando se recibe un mensaje de comando rechazado
void GUIPanel::CommandRejected(int16_t code)
{
    ui->statusLabel->setText(tr("Status: Comando rechazado,%1").arg(code));
    ui->pushButton->setText(tr("Borrar Mensaje"));
}


void GUIPanel::on_colorWheel_colorChanged(const QColor &arg1)
{
    //Poner aqui el codigo para pedirle al objeto "tiva" (clase QRemoteTIVA) que envíe la orden de cambiar el Color hacia el microcontrolador
    tiva.LEDColour(arg1.red(),arg1.green(),arg1.blue());
}

void GUIPanel::on_checkSWs_clicked()
{
   tiva.RequestTiva();
}

void GUIPanel::RequestReceived(uint8_t s1, uint8_t s2)
{
// Cuando recibamos el aviso de que podemos tomar valores de los pulsadores, vemos que valores tienen y actuamos sobre los leds
   //Según los valores que hayamos recibido encendemos o apagamos los leds de qt
    bool pulsador1=false;
    bool pulsador2=false;

    if(s1>0) pulsador1=true;
    else pulsador1=false;

    if(s2>0) pulsador2=true;
    else pulsador2=false;

    ui->LEDsw1->setChecked(!pulsador1);
    ui->LEDsw2->setChecked(!pulsador2);
}

void GUIPanel::IntensityReceived(float x)
{
   ui->Knob->setValue(x);
}
void GUIPanel::ColourReceived(int rojo,int verde,int azul)
{
    QColor aux;
    aux.setRed(rojo >> 8);
    aux.setGreen(verde >> 8);
    aux.setBlue(azul >> 8);
    ui->colorWheel->setColor(aux);
}



void GUIPanel::on_Interrupts_clicked(bool checked)
{
    tiva.SwitchInterrupts(checked);
}

void GUIPanel::LedsReceived(uint8_t a, uint8_t b, uint8_t c)
{
    bool rojo,verde,azul;

    if(a>0) rojo=true;
    else rojo=false;
    if(b>0) verde=true;
    else verde=false;
    if(c>0) azul=true;
    else azul=false;

    ui->rojo->setChecked(rojo);
    ui->verde->setChecked(verde);
    ui->azul->setChecked(azul);
}

// Segunda parte
//SEMANA2: Slot que asociaremos a una señal que genera el objeto TIVA cuando recibe datos del ADC
void GUIPanel::procesaDatoADC(uint16_t chan1,uint16_t chan2,uint16_t chan3,uint16_t chan4)
{
    //Manda cada dato a su correspondiente display (pasandolos a voltios)
    ui->lcdCh1->display(((double)chan1)*VCC_LEVEL/4096.0);
    ui->lcdCh2->display(((double)chan2)*VCC_LEVEL/4096.0);
    ui->lcdCh3->display(((double)chan3)*VCC_LEVEL/4096.0);
    ui->lcdCh4->display(((double)chan4)*VCC_LEVEL/4096.0);

    int i;
    static int  j=0;
    //Inicializacion de los valores básicos
    for(i=0;i<8;i++){
        yVal[0][i+j]=((double)chan1)*VCC_LEVEL/4096.0;
        yVal[1][i+j]=((double)chan2)*VCC_LEVEL/4096.0;
        yVal[2][i+j]=((double)chan3)*VCC_LEVEL/4096.0;
        yVal[3][i+j]=((double)chan4)*VCC_LEVEL/4096.0;
        xVal[i+j]=i+j;
   }
    j=j+8;
    Channels[0]->setRawSamples(xVal,yVal[0],1024);
    Channels[1]->setRawSamples(xVal,yVal[1],1024);
    Channels[2]->setRawSamples(xVal,yVal[2],1024);
    Channels[3]->setRawSamples(xVal,yVal[3],1024);
    if (j>=1024) j=0;

    ui->Grafica->replot(); //Refresca la grafica una vez actualizados los valores
}

//SEMANA2: Slot asociado a la rosca "frecuencia"
void GUIPanel::on_frecuencia_sliderReleased()
{
    double value = ui->frecuencia->value();
    tiva.ChangeFrequency(value);

    //Recalcula los valores de la grafica
    for(int i=0;i<1024;i++){
            //xVal[i]=i;
        //Los valores "constantes" deberian definirse como etiquetas por "estética"
        yVal[0][i]=VCC_LEVEL*(sin((double)i*2.0*(value+1.0)*3.14159/1024.0)+1.0)/2.0;
        yVal[1][i]=VCC_LEVEL*(sin((double)i*4.0*(value+1.0)*3.14159/1024.0)+1.0)/2.0;
        yVal[2][i]=VCC_LEVEL*(sin((double)i*8.0*(value+1.0)*3.14159/1024.0)+1.0)/2.0;
        yVal[3][i]=VCC_LEVEL*(sin((double)i*16.0*(value+1.0)*3.14159/1024.0)+1.0)/2.0;
    }
    ui->Grafica->replot(); //Refresca la grafica una vez actualizados los valores
}

void GUIPanel::on_TimerADC_clicked(bool checked)
{
    tiva.SetTimerADC(checked);
}

void GUIPanel::ChangeAlarmEventFlags(){
    PARAM_COMANDO_FLAGALARM FlagsAlarm;
    if(ui->AlarmP0_check->isChecked()) FlagsAlarm.flags.PE0=1;
    if(ui->AlarmP1_check->isChecked()) FlagsAlarm.flags.PE1=1;
    if(ui->AlarmP2_check->isChecked()) FlagsAlarm.flags.PE2=1;
    if(ui->AlarmP3_check->isChecked()) FlagsAlarm.flags.PE3=1;
    if(ui->AlarmA0_check->isChecked()) FlagsAlarm.flags.AI0=1;
    if(ui->AlarmA1_check->isChecked()) FlagsAlarm.flags.AI1=1;
    if(ui->AlarmA2_check->isChecked()) FlagsAlarm.flags.AI2=1;
    if(ui->AlarmA3_check->isChecked()) FlagsAlarm.flags.AI3=1;
    // debug
    ui->statusLabel->setNum((double)FlagsAlarm.ui8Valor);

    // Inicializar temporizador de 10 segundos

    tiva.setFlagAlarm(FlagsAlarm);
}
