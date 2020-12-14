#include "stm32f4xx_hal.h"

/**
 * Override HAL Eth Init function
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	if (heth->Instance == ETH) {

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();

		/**ETH GPIO Configuration
		   PC1     ------> ETH_MDC
		   PA1     ------> ETH_REF_CLK
		   PA2     ------> ETH_MDIO
		   PA7     ------> ETH_CRS_DV
		   PC4     ------> ETH_RXD0
		   PC5     ------> ETH_RXD1
		   PB11     ------> ETH_TX_EN
		   PB12     ------> ETH_TXD0
		   PB13     ------> ETH_TXD1
		 */
        
        GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Pull = GPIO_NOPULL;
        GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
        GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        /* Enable the Ethernet global Interrupt */
        HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
        HAL_NVIC_EnableIRQ(ETH_IRQn);
        
        /* Enable ETHERNET clock */
        __HAL_RCC_ETH_CLK_ENABLE();
    }
}

uint8_t mbed_otp_mac_address(char *mac){

	int startAddress = (0x1FFF7800);

	mac[0] = *(volatile char *)(startAddress + 0);
	mac[1] = *(volatile char *)(startAddress + 1);
	mac[2] = *(volatile char *)(startAddress + 2);
	mac[3] = *(volatile char *)(startAddress + 3);
	mac[4] = *(volatile char *)(startAddress + 4);
	mac[5] = *(volatile char *)(startAddress + 5);

	return 1;
} 

/**
 * Override HAL Eth DeInit function
 */
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
    if (heth->Instance == ETH) {
    
        __HAL_RCC_ETH_CLK_DISABLE();
        /**ETH GPIO Configuration
           PC1     ------> ETH_MDC
           PA1     ------> ETH_REF_CLK
           PA2     ------> ETH_MDIO
           PA7     ------> ETH_CRS_DV
           PC4     ------> ETH_RXD0
           PC5     ------> ETH_RXD1
           PB11     ------> ETH_TX_EN
           PB12     ------> ETH_TXD0
           PB13     ------> ETH_TXD1
         */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);

        /* Disable the Ethernet global Interrupt */
        NVIC_DisableIRQ(ETH_IRQn);
    }
}
