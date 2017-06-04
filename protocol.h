// Cabecera de la implementacion de protocolo de comunicaciones
// El estudiante debera añadir nuevos comandos a la lista de comandos disponibles, asi como
// crear las estructuras adecuadas para los parameros de los comandos.
// Comentario de prueba

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

//Caracteres especiales
#define START_FRAME_CHAR 0xFC
#define STOP_FRAME_CHAR 0xFD
#define ESCAPE_CHAR 0xFE
#define STUFFING_MASK 0x20


#define CHEKSUM_TYPE uint16_t
#define COMMAND_TYPE uint8_t

#define CHECKSUM_SIZE (sizeof(CHEKSUM_TYPE))
#define COMMAND_SIZE (sizeof(COMMAND_TYPE))
#define START_SIZE (1)
#define END_SIZE (1)

#define MINIMUN_FRAME_SIZE (START_SIZE+COMMAND_SIZE+CHECKSUM_SIZE+END_SIZE)

#define MAX_DATA_SIZE (32)
#define MAX_FRAME_SIZE (2*(MAX_DATA_SIZE))

//ESTRUCTURA MUESTRAS SEMANA 2
typedef struct
{
    uint16_t chan1;
    uint16_t chan2;
    uint16_t chan3;
    uint16_t chan4;
} MuestrasADC;

//Códigos de los comandos

// El estudiante debe añadir aqui cada nuevo comando que implemente. IMPORTANTE el orden de los comandos
// debe SER EL MISMO aqui, y en el codigo equivalente en la parte del microcontrolador (Code Composer)
/*
typedef enum {
    COMANDO_RECHAZADO,
    COMANDO_PING,
    COMANDO_LEDS,
    COMANDO_BRILLO,
    COMANDO_COLOR,
    COMANDO_MODO,
    COMANDO_REQUEST,
    COMANDO_INTERRUPT,
    COMANDO_ADC,
    COMANDO_FREQ,
    COMANDO_TIMER
} commandTypes;
*/
typedef enum {
    COMANDO_RECHAZADO,
    COMANDO_PING,
    COMANDO_LEDS,
    COMANDO_BRILLO,
    COMANDO_MODO,
    COMANDO_REQUEST,
    COMANDO_COLOR,
    COMANDO_INTERRUPT,

    // Semana 2
    COMANDO_ADC,
    COMANDO_FREQ,
    COMANDO_TIMER
} commandTypes;

//Codigos de Error de protocolo
#define PROT_ERROR_BAD_CHECKSUM (-1)
#define PROT_ERROR_RX_FRAME_TOO_LONG (-2)
#define PROT_ERROR_NOMEM (-3)
#define PROT_ERROR_STUFFED_FRAME_TOO_LONG (-4)
#define PROT_ERROR_COMMAND_TOO_LONG (-5)
#define PROT_ERROR_INCORRECT_PARAM_SIZE (-6)
#define PROT_ERROR_BAD_SIZE (-7)
#define PROT_ERROR_UNIMPLEMENTED_COMMAND (-7)


//Estructuras relacionadas con los parámetros
// El estudiante debera crear y añadir las estructuras adecuadas segun los
// parametros que quiera asociar a los comandos. Las estructuras deben ser iguales en los
// ficheros correspondientes de la parte del micro (Code Composer)

#pragma pack(1)	//Con esto consigo compatibilizar el alineamiento de las estructuras en memoria del PC (32 bits)
                // y del ARM (aunque, en este caso particular no haria falta ya que ambos son 32bit-Little Endian)

#define PACKED


typedef struct {
    unsigned char command;
} PACKED PARAM_COMANDO_RECHAZADO;

typedef union{
    struct {
                uint8_t fRed:1;
                uint8_t fGreen:1;
                uint8_t fBlue:1;
    } PACKED leds;
    uint8_t ui8Valor;
} PACKED PARAM_COMANDO_LEDS;

typedef struct {
    float rIntensity;
} PACKED PARAM_COMANDO_BRILLO;

typedef struct {
    uint8_t x;
} PACKED PARAM_COMANDO_MODO;

typedef struct {
    uint8_t sw1;
    uint8_t sw2;
} PACKED PARAM_COMANDO_REQUEST;

typedef struct {
    int r;
    int g;
    int b;
} PACKED PARAM_COMANDO_COLOR;

typedef struct {
    uint8_t x;
} PACKED PARAM_COMANDO_INTERRUPTS;

// Segunda parte
typedef struct
{
    bool Timer_On;
} PACKED PARAM_COMANDO_TIMER;

typedef struct
{
    uint16_t chan1;
    uint16_t chan2;
    uint16_t chan3;
    uint16_t chan4;
} PACKED PARAM_COMANDO_ADC; //SEMANA2

typedef struct
{
    double frequency;
} PACKED PARAM_COMANDO_FREQ; //SEMANA2

#pragma pack()	//...Pero solo para los comandos que voy a intercambiar, no para el resto.

//Funciones que permiten decodificar partes de la trama
uint8_t decode_command_type(uint8_t * buffer);
//int32_t check_and_extract_command_param(uint8_t * buffer, int32_t frame_size, uint32_t payload,void *campo);
int32_t check_and_extract_command_param(void *ptrtoparam, int32_t param_size, uint32_t payload,void *param);
int32_t get_command_param_pointer(uint8_t * buffer, int32_t frame_size, void **campo);

//Funciones de la libreria
int create_frame(uint8_t *frame, uint8_t command_type, void * param, int32_t param_size, int32_t max_size);
int destuff_and_check_checksum (uint8_t *frame, int32_t max_size);


#endif
