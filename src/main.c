
/*******************************************************************************
 * File Name          :
 * Author             :
 * Version            :
 * Date               :
 * Description        :
 ********************************************************************************
 *
 *
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Beacon_config.h"
#include "BlueNRG1_conf.h"
#include "BlueNRG1_it.h"
#include "FreeRTOS.h"
#include "SDK_EVAL_Config.h"
#include "ble_status.h"
#include "bluenrg_x_device.h"
#include "semphr.h"
#include "task.h"

#define DEBUG 1

#ifndef DEBUG
    #define DEBUG 0
#endif

#if DEBUG
    #include <stdio.h>
    #define PRINTF(...)                                         \
        do                                                      \
        {                                                       \
            xSemaphoreTake(UARTSemaphoreHandle, portMAX_DELAY); \
            printf(__VA_ARGS__);                                \
            xSemaphoreGive(UARTSemaphoreHandle);                \
        } while (0)
#else
    #define PRINTF(...)
#endif

/*-----------------------------------------------------------*/
/* Priorities at which the tasks are created.
   Assigning an high priority to BLE Task can give better latency, especially
   if other tasks are CPU resource hungry. */
#define TASK_PRIORITY_TEST (tskIDLE_PRIORITY + 2)
/*-----------------------------------------------------------*/
/* Wait time of the test task (number of ticks) */
#define TEST_PERIOD        (250 / portTICK_PERIOD_MS)
// #define MS_TO_TICKS(ms)    (ms / portTICK_PERIOD_MS)

volatile uint32_t lSystickCounter = 0;

/* Binary semaphore used to synchronize Stack Tick and radio ISR. */
SemaphoreHandle_t radioActivitySemaphoreHandle;
/* Mutex used to avoid that the BLE Stack Tick can be interrupted by an ACI
   function in another thread. */
SemaphoreHandle_t BLETickSemaphoreHandle;
/* Mutex used to access UART resource */
SemaphoreHandle_t UARTSemaphoreHandle;

void createTasks(void);

/**
 * @brief  Main program code.
 * @param  None
 * @retval None
 */
int main(void)
{
    tBleStatus ret;

    /* System Init */
    SystemInit();

    /* Identify BlueNRG1 platform */
    SdkEvalIdentification();

    /* Configure the LEDs */
    SdkEvalLedInit(LED1);

    // while (1)
    // {
    //     SdkEvalLedToggle(LED1);
    //     // for (size_t i = 0; i < 500000; i++)
    //     // {
    //     //     /* code */
    //     //     __NOP();
    //     // }
    // }

    /* Init the UART peripheral */
    SdkEvalComUartInit(UART_BAUDRATE);
    UARTSemaphoreHandle = xSemaphoreCreateMutex();

    /* Create a binary semaphore to sync with radio interrupts */
    radioActivitySemaphoreHandle = xSemaphoreCreateBinary();
    /* Create a mutex semaphore to avoid calling aci functions while BTLE_StackTick() is running.*/
    BLETickSemaphoreHandle = xSemaphoreCreateMutex();
    if (radioActivitySemaphoreHandle == NULL || BLETickSemaphoreHandle == NULL)
    {
        while (1)
            ;
    }

    /* BlueNRG-1,2 stack init */
    ret = BlueNRG_Stack_Initialization(&BlueNRG_Stack_Init_params);
    if (ret != BLE_STATUS_SUCCESS)
    {
        PRINTF("Error in BlueNRG_Stack_Initialization() 0x%02x\r\n", ret);
        while (1)
            ;
    }

    /* tasks generation */
    createTasks();

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the idle and/or
    timer tasks	to be created.  See the memory management section on the
    FreeRTOS web site for more details. */

    /** NEVER REACHED */
    for (;;)
        ;
}

/*-----------------------------------------------------------*/

/**
 * @brief  FreeRTOS task callback function
 * @retval None
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

/**
 * @brief  FreeRTOS task callback function
 * @retval None
 */
void vApplicationTickHook(void)
{
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/

/* Just a test task which makes a very short pulse on a GPIO. */
static void taskTest(void* pvParameters)
{
    TickType_t xNextWakeTime;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, the constant used converts ticks
        to ms.  While in the Blocked state this task will not consume any CPU
        time. */
        vTaskDelayUntil(&xNextWakeTime, TEST_PERIOD);

        /* Only do a pulse. */
        // SdkEvalLedOn(LED3);
        // __NOP();
        // __NOP();
        // __NOP();
        // __NOP();
        // SdkEvalLedOff(LED3);
        SdkEvalLedToggle(LED1);

        PRINTF("Task Test\r\n");
    }
}
/*-----------------------------------------------------------*/

void createTasks(void)
{

    xTaskCreate(taskTest, "TaskTest", 80, NULL, TASK_PRIORITY_TEST, NULL);

    // //   xTaskCreate(BLETask,"BLEStack", 512, NULL, BLE_TASK_PRIORITY, NULL);

    // //   xTaskCreate( changeADVDataTask, "ADV", 140, NULL, TEST_TASK_PRIORITY, NULL );
}
/*-----------------------------------------------------------*/

/**
 * @brief  FreeRTOS task callback function
 * @retval None
 */
void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif
