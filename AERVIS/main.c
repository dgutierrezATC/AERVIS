#include <CoOS.h>
#include "stm32f4xx_conf.h"
#include "BSP.h"
#include "LedTask.h"
#include "JoyTmr.h"
#include "LCDTask.h"
#include "AERTask.h"


void SystemInit(void);

void CreateSystemObjetcs(void){
	//Inicialización de los elementos compartidos: flags, semaf., colas...
	CreateJoyFlags();		//Crear banderas del joystick
}

void CreateUserTasks(void){
	//Creación de las tareas de usuario
	CreateLedTask();
	CreateJoyTask();
	CreateAERTask();
	CreateLCDTask();
}


int main(void)
{

	SystemInit();			//Inicialización del reloj

	CoInitOS ();			//Inicialización del CoOs

	CreateSystemObjetcs();	//Inicialización Sem, flags, queues...

	CreateUserTasks();		//Creación de Tareas

	CoStartOS ();			//Comienzo de ejecución del planificador

    while(1)				//La ejecución nunca debería llegar aquí
    {
    }
}

