#include "main.h"
#include "cmsis_os.h"

// Declarații de funcții
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void StartGasMonitorTask(void *argument);
void StartBluetoothTask(void *argument);

// Handle-uri pentru UART și task-uri
UART_HandleTypeDef huart1;
osThreadId_t gasMonitorTaskHandle;
osThreadId_t bluetoothTaskHandle;

int main(void) {
    // Inițializare sistem
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    // Inițializare kernel FreeRTOS
    osKernelInitialize();

    // Creare task pentru senzorul de gaz
    const osThreadAttr_t gasMonitorTaskAttr = {
        .name = "GasMonitorTask",
        .priority = osPriorityNormal,
        .stack_size = 128 * 4
    };
    gasMonitorTaskHandle = osThreadNew(StartGasMonitorTask, NULL, &gasMonitorTaskAttr);

    // Creare task pentru Bluetooth
    const osThreadAttr_t bluetoothTaskAttr = {
        .name = "BluetoothTask",
        .priority = osPriorityNormal,
        .stack_size = 128 * 4
    };
    bluetoothTaskHandle = osThreadNew(StartBluetoothTask, NULL, &bluetoothTaskAttr);

    // Pornirea schedulerului FreeRTOS
    osKernelStart();

    while (1) {
        // Schedulerul FreeRTOS controlează execuția
    }
}

// Task pentru monitorizarea senzorului de gaz
void StartGasMonitorTask(void *argument) {
    uint8_t gasAlertMessage[] = "ALERT: Gaz detectat!\r\n";
    uint8_t gasClearMessage[] = "Gaz in limite normale.\r\n";
    GPIO_PinState prevState = GPIO_PIN_RESET;

    for (;;) {
        // Citirea pinului digital PA0 (senzor gaz)
        GPIO_PinState gasState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

        if (gasState == GPIO_PIN_SET) {
            // Gaz absent - aprindem LED-ul verde (PA6)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);  // LED verde
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // LED roșu
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Buzzer

            // Trimitem un mesaj dacă starea s-a schimbat
            if (prevState != gasState) {
                HAL_UART_Transmit(&huart1, gasClearMessage, sizeof(gasClearMessage) - 1, HAL_MAX_DELAY);
            }
        } else {
            // Gaz detectat - aprindem LED-ul roșu (PA5) și buzzerul (PB2)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // LED roșu
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);  // Buzzer
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // LED verde

            // Trimitem un mesaj dacă starea s-a schimbat
            if (prevState != gasState) {
                HAL_UART_Transmit(&huart1, gasAlertMessage, sizeof(gasAlertMessage) - 1, HAL_MAX_DELAY);
            }
        }

        prevState = gasState;
        osDelay(100); // Delay pentru stabilitate
    }
}

// Task pentru gestionarea Bluetooth
void StartBluetoothTask(void *argument) {
    uint8_t rxBuffer[100];  // Buffer pentru recepție
    uint8_t txMessage[] = "Bluetooth functional! Trimite comenzi.\r\n";

    // Trimite un mesaj de inițializare
    HAL_UART_Transmit(&huart1, txMessage, sizeof(txMessage) - 1, HAL_MAX_DELAY);

    for (;;) {
        // Verifică dacă primește date prin UART
        if (HAL_UART_Receive(&huart1, rxBuffer, 1, 500) == HAL_OK) {
            // Trimite înapoi mesajul primit
            HAL_UART_Transmit(&huart1, rxBuffer, 1, HAL_MAX_DELAY);
        }
        osDelay(10); // Delay scurt pentru stabilitate
    }
}

// Inițializare GPIO
void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configurare pin PA0 (intrare digitală pentru senzorul MQ-02)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurare pinuri pentru LED-uri și buzzer
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Inițializare LED-uri și buzzer (toate stinse)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // LED roșu
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // LED verde
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Buzzer
}

// Inițializare UART1 pentru Bluetooth
void MX_USART1_UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

// Configurare ceas sistem
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 10;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

// Funcție pentru gestionarea erorilor
void Error_Handler(void) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // LED roșu pentru debug
        HAL_Delay(200);
    }
}
