#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "NRF24L01.h"
#include "HW504.h"
#include "OLED.h"
/***************************************************************
以下为NRF24L01接发送数据的模板

发送的数据格式如下

[第一位，第二位，第三位，第四位.....，第三十三位]

其中，第一位也就是Buf[0]位接收到的数据长度
第二位到第三十三位一个32个字节的数据位接受到的数据；

例子如下，5代表后面跟着5个数据
{5, 0x11, 0x22, 0x33, 0x44, 0x55};

********************By:普普通通Ordinary*************************/


int main(void)
{
	uint8_t Buf[32] = {0};	
	uint16_t X, Y;
	uint8_t SW;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	HW504_Init();
 
	NRF24L01_Init();
	
//	OLED_Init();		//OLED初始化
//	OLED_ShowString(1, 1, "X:");
//	OLED_ShowString(2, 1, "Y:");
//	OLED_ShowString(3, 1, "SW :");
	
	while (1)
	{
		X = HW504_R_X();
		Y = HW504_R_Y();
		SW = HW504_R_SW();
		
		if (X <= 100 || Y <= 100) {
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		} else if (X >= 4000 || Y >= 4000) {
			GPIO_SetBits(GPIOC, GPIO_Pin_13);
		}
		
		Buf[0] = (uint8_t)(X & 0xFF); // 获取低8位
    Buf[1] = (uint8_t)((X >> 8) & 0xFF); // 获取高8位
		Buf[2] = (uint8_t)(Y & 0xFF); // 获取低8位
    Buf[3] = (uint8_t)((Y >> 8) & 0xFF); // 获取高8位
		Buf[4] = SW;
		NRF24L01_SendBuf(Buf);
		
		
//		OLED_ShowNum(1, 3, X, 4);
//		OLED_ShowNum(2, 3, Y, 4);
//		OLED_ShowNum(3, 5, SW, 1);
		Delay_ms(100);
		
	}
}
