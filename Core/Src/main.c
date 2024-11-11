#include "main.h"

// Funcție pentru inițializarea GPIO-ului
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  // Inițializarea HAL
  HAL_Init();

  // Configurarea ceasului
  SystemClock_Config();

  // Inițializarea GPIO-urilor
  MX_GPIO_Init();

  // Buclă principală
  while (1)
  {
    // Aprinderea LED-ului (setarea PA5 HIGH)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(500);  // Așteaptă 500 ms

    // Stingerea LED-ului (setarea PA5 LOW)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);  // Așteaptă 500 ms
  }
}

// Funcție pentru configurarea pinului PA5 ca ieșire
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Activarea ceasului pentru GPIOA
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // Configurarea pinului PA5 ca ieșire digitală
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Configurare ca ieșire push-pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;  // Fără rezistență de pull-up/pull-down
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // Viteza joasă
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// Funcție de configurare a ceasului
void SystemClock_Config(void)
{
  // Configurarea ceasului sistemului poate fi adăugată aici dacă este necesar
  // (în funcție de setările STM32CubeMX, acest lucru poate fi generat automat)
}

