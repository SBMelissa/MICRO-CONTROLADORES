//lIBRERIAS NECESARIAS
#include <stdio.h>
#include <stdlib.h>

//ESTADOS DE LA MS
#define ESTADO_INIT      0
#define ESTADO_CERRADO   1
#define ESTADO_ABIERTO   2
#define ESTADO_CERRANDO  3
#define ESTADO_ABRIENDO  4
#define ESTADO_ERROR     5

//VALORES CONSTANTES
#define TRUE  1
#define FALSE 0
#define RT_MAX 180

//CODIGOS DE ERROR
#define ERROR_LS 1
#define ERROR_OK 0
#define ERROR_RT 2


//VARIABLES DE LOS ESTADOS DE MS
int ESTADO_SIGUENTE  = ESTADO_INIT;
int ESTADO_ACTUAL    = ESTADO_INIT;
int ESTADO_ANTERIOR  = ESTADO_INIT;

// Estructura para manejar las se�ales de entrada/salida//SENSORES
struct DATA_IO
{
    unsigned int LSC:1;      // Sensor de posici�n cerrado
    unsigned int LSA:1;      // Sensor de posici�n abierto
    unsigned int SPP:1;      // Se�al para iniciar el proceso de apertura
    unsigned int MA:1;       // Motor de apertura
    unsigned int MC:1;       // Motor de cierre
    unsigned int Cont_RT;    // Contador de tiempo
    unsigned int led_A:1;    // LED indicador de apertura
    unsigned int led_C:1;    // LED indicador de cierre
    unsigned int led_ER:1;   // LED indicador de error
    unsigned int COD_ERR;    // C�digo de error
    unsigned int DATOS_READY:1; // Se�al para indicar si los datos est�n listos
    unsigned int RSBT:1; //BOTON RESET
} data_io;

// RETRASO PARA EL MICRO
void delay(void);

//AVISO DE FNCIONES FUTURA EN MI CODIGO
int Func_INIT(void);
int Func_CERRADO(void);
int Func_CERRANDO(void);
int Func_ABRIENDO(void);
int Func_ABIERTO(void);
int Func_ERROR(void);
int main()
{
    //MS CON TODOS LOS ESTADOS RELACIONADOS DE ACUERDO AL DIAGRAMA

    for (;;) //ESPERA O PAUSA ENTRE ESTADOS
    {
        if (ESTADO_SIGUENTE == ESTADO_INIT)
        {
            ESTADO_SIGUENTE = Func_INIT();
        }
        if (ESTADO_SIGUENTE == ESTADO_CERRADO)
        {
            ESTADO_SIGUENTE = Func_CERRADO();
        }
        if (ESTADO_SIGUENTE == ESTADO_CERRANDO)
        {
            ESTADO_SIGUENTE = Func_CERRANDO();
        }
        if (ESTADO_SIGUENTE == ESTADO_ABRIENDO)
        {
            ESTADO_SIGUENTE = Func_ABRIENDO();
        }
        if (ESTADO_SIGUENTE == ESTADO_ABIERTO)
        {
            ESTADO_SIGUENTE = Func_ABIERTO();
        }
        if (ESTADO_SIGUENTE == ESTADO_ERROR)
        {
            ESTADO_SIGUENTE = Func_ERROR();
        }
    }

    return 0;
}

// Estado INIT
int Func_INIT(void)
{
    ESTADO_ANTERIOR = ESTADO_INIT;
    ESTADO_ACTUAL = ESTADO_INIT;

    // MOTOR, LEDS Y CONTADOR//MICRO
    data_io.MC = FALSE;//MOTOR CERRANDO
    data_io.MA = FALSE;// MOTOR ABRIENDO
    data_io.SPP = FALSE; //PULSO//BTN
    data_io.led_C = TRUE; //LED DE MC
    data_io.led_A = TRUE; //LED DE MA
    data_io.led_ER = TRUE;//LED DE ERROR
    delay(); // ESPERA PARA EL MICRO
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;
    data_io.COD_ERR = ERROR_OK;
    data_io.Cont_RT = 0;//CONTADOR
    data_io.DATOS_READY = FALSE;

    // Esperar hasta que los datos est�n listos
    while (!data_io.DATOS_READY)



        // CAMBIOS DE ESTADO DEPENDIENDO DE LIMIT SWICHT COMO EN EL DIAGRAMA
        for (;;)
        {
            if (data_io.LSC == TRUE && data_io.LSA == FALSE)
            {
                return ESTADO_CERRADO;
            }
            if (data_io.LSC == TRUE && data_io.LSA == TRUE)
            {
                return ESTADO_ERROR;
            }
            if ((data_io.LSC == FALSE && data_io.LSA == FALSE) || (data_io.LSC == FALSE && data_io.LSA == TRUE))
            {
                return ESTADO_CERRANDO;
            }
        }
}

// Estado CERRADO
int Func_CERRADO(void)
{
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;
    data_io.MC = FALSE;
    data_io.SPP = FALSE;
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;

    // CAMBIO ABRIENDO POR EL BOTON
    for (;;)
    {
        if (data_io.SPP == TRUE)
        {
            data_io.SPP = FALSE;
            return ESTADO_ABRIENDO;
        }
    }
}

// Estado CERRANDO
int Func_CERRANDO(void)
{
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;
    data_io.MC = TRUE;
    data_io.Cont_RT = 0;
    data_io.led_C = TRUE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;

    for (;;)
    {
        if (data_io.LSC == TRUE) // Si el sensor de cerrado est� activo//cerro
        {
            return ESTADO_CERRADO;
        }
        if (data_io.Cont_RT > RT_MAX) // Si se supera el tiempo m�ximo hay un error y no cambio a cerrado
        {
            return ESTADO_ERROR;
        }
    }
}

// Estado ABRIENDO
int Func_ABRIENDO(void)
{
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;
    data_io.MA = TRUE;
    data_io.Cont_RT = 0;
    data_io.led_C = FALSE;
    data_io.led_A = TRUE;
    data_io.led_ER = FALSE;

    // Verificar transiciones de estado
    for (;;)
    {
        if (data_io.LSA == TRUE) // Si el sensor de apertura est� activo//abrio
        {
            return ESTADO_ABIERTO;
        }
        if (data_io.Cont_RT > RT_MAX) // Si se supera el tiempo m�ximo hay un error y no cambio a abierto
        {
            return ESTADO_ERROR;
        }
    }
}

// Estado ABIERTO
int Func_ABIERTO(void)
{
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;
    data_io.MA = FALSE;
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;

    // CAMBIO CERRANNDO POR EL BOTON
    for (;;)
    {
        if (data_io.SPP == TRUE)
        {
            data_io.SPP = FALSE;
            return ESTADO_CERRANDO;
        }
    }
}


// Estado ERROR
int Func_ERROR(void)
{
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;
    data_io.MA = FALSE;
    data_io.MC = FALSE;
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = TRUE; // LED DE ERROR ON
    data_io.COD_ERR = ERROR_RT; // ERROR EXCEDIDO DE TIEMPO

    // BUBLE HASTA QUE SE RESUELVA EL ERROR

    for (;;)
    {
        //VERIFICACION DE ERROR SOLUCIONADO
        if (data_io.LSC == FALSE && data_io.LSA == FALSE) // ERROR SOLUCIONADO
        {
            if (data_io.RSBT == FALSE)
            {
                return ESTADO_ACTUAL; // SI NO SE SOLUCIONA O NO SE PULSA RESET SE MANTIENE EL ESTADO ACTUAL
            }

            // SI SE PRESIONA RESET
            data_io.RSBT = TRUE;
            return ESTADO_INIT; // SI SE RESETEA EL SISTEMA VA AL ESTADO INIT
        }
    }
}

void delay(void)
{
    // Implementaci�n del delay espec�fica del microcontrolador
}
