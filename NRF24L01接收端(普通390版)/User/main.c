#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "NRF24L01.h"
#include "OLED.h"
#include "Servo.h"

uint16_t knt = 0; // 信号检测

int main(void)
{
	uint8_t Buf[32] = {0};
	uint16_t X, Y;
	uint8_t SW, SW_pre = 2;
	uint16_t Speed = 0; // 按键控制2档速度
	uint16_t Base = 1700, Step = 200; // 前进
	uint16_t Stop = 1500, Back = 1100; // 停止与后退
	uint16_t Mid = 87, Angle = 12;
	
	NRF24L01_Init();
	
	Servo_Init();
	Servo_SetAngle(Mid);
	Motor_SetPWM(Stop);
	Delay_ms(3000);
	
//	OLED_Init();		//OLED初始化
//	OLED_ShowString(1, 1, "X:");
//	OLED_ShowString(2, 1, "Y:");
//	OLED_ShowString(3, 1, "Speed:");
	
	while (1)
	{ 
		if (knt >= 50) { // 超过1s没收到信号，停车
			Motor_SetPWM(Stop);
			Servo_SetAngle(Mid);
			knt = 0;
		}
		
		if (NRF24L01_Get_Value_Flag() == 0)
		{
			knt = 0;
			NRF24L01_GetRxBuf(Buf);
			
			X = Buf[1] * 256 + Buf[0];
			Y = Buf[3] * 256 + Buf[2];
			SW = Buf[4];
			
			if (SW_pre == 0 && SW == 1){ // 按键松开时设置
				if (Y <= 1000 && Mid >= 80) { // 微调前行方向向左
					Mid -= 1;
				} else if (Y >= 3000 && Mid <= 100) { // 微调前行方向向右
					Mid += 1;
				} else if (X >= 3000) {
					Speed = (Speed + 1) % 2; // 调速
				}
			}
			SW_pre = SW;
			
			if (X >= 3000) { // 前进
				Motor_SetPWM(Base + Speed * Step);
			} else if (X <= 1000) { //后退
				Motor_SetPWM(Back);
			} else { // 停止
				Motor_SetPWM(Stop);
			}
			
			if (Y <= 1000) { // 左转
				Servo_SetAngle(Mid - Angle);
				if (X >1000 && X <3000) {
					Motor_SetPWM(Base);
					Servo_SetAngle(Mid - Angle * 2);
				}
			} else if (Y >= 3000) { // 右转
				Servo_SetAngle(Mid + Angle);
				if (X >1000 && X <3000) {
					Motor_SetPWM(Base);
					Servo_SetAngle(Mid + Angle * 2);
				}
			} else { // 回正
				Servo_SetAngle(Mid);
			}
			
//			OLED_ShowNum(1, 3, X, 4);
//			OLED_ShowNum(2, 3, Y, 4);
//			OLED_ShowNum(3, 7, Speed, 3);
		}
		
		Delay_ms(100);
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)		//判断是否是TIM2的更新事件触发的中断
	{
		knt ++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);			//清除TIM2更新事件的中断标志位
	}
}