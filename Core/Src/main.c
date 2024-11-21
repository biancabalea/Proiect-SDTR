#include "main.h"
#include "cmsis_os.h"

// Declarații de funcții
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void StartGasMonitorTask(void *argument);

// Handle pentru task
osThreadId_t gasMonitorTaskHandle;

int main(void) {
    // Inițializare sistem
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // Inițializare kernel FreeRTOS
    osKernelInitialize();

    // Creare task
    const osThreadAttr_t gasMonitorTaskAttr = {
        .name = "GasMonitorTask",
        .priority = osPriorityNormal,
        .stack_size = 128 * 4
    };
    gasMonitorTaskHandle = osThreadNew(StartGasMonitorTask, NULL, &gasMonitorTaskAttr);

    // Pornirea schedulerului FreeRTOS
    osKernelStart();

    while (1) {
        // Schedulerul FreeRTOS controlează execuția
    }
}

// Task pentru monitorizarea senzorului de gaz
void StartGasMonitorTask(void *argument) {
    for (;;) {
        // Citirea pinului digital PA0 (senzor gaz)
        GPIO_PinState gasState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

        if (gasState == GPIO_PIN_SET) {
            // Gaz absent - aprindem LED-ul verde (PA6)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);  // LED verde
            // Stingem LED-ul roșu (PA5) și buzzerul (PB2)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // LED roșu
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Buzzer
        } else {

            // Gaz detectat - aprindem LED-ul roșu (PA5) și buzzerul (PB2)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // LED roșu
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);  // Buzzer
            // Stingem LED-ul verde (PA6)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        }

        osDelay(100); // Delay pentru stabilitate
    }
}

void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE(); // Activare ceas GPIOA
    __HAL_RCC_GPIOB_CLK_ENABLE(); // Activare ceas GPIOB (pentru buzzer)

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configurare pin PA0 (intrare digitală pentru senzorul MQ-02)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN; // Pull-down pentru a evita starea flotantă
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurare pin PA5 (LED verde)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Mod Output push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurare pin PA6 (LED roșu)
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Mod Output push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configurare pin PB2 (Buzzer)
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Mod Output push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Inițializare LED-uri și buzzer (toate stinse la început)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // LED verde stins
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // LED roșu stins
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Buzzer oprit
}

void SystemClock_Config(void) {
    // Configurarea ceasului de sistem
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

void Error_Handler(void) {
    while (1) {
        // Afișare eroare (poți adăuga un LED pentru debug)
    }
}
