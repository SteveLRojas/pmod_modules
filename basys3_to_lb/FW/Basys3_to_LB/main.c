#include "CH552.H"
#include "CH552_RCC.h"
#include "CH552_GPIO.h"
#include "CH552_TIMER.h"
#include "CH552_SPI.h"
#include "CH552_UART.h"
#include "CH552_USB_CDC.h"

//Pins:
// DUT_GPIO2    = P11
// MCU_CSN      = P14
// MCU_MOSI     = P15
// MCU_MISO_RXD = P16
// MCU_SCK_TXD  = P17
// DUT_GPIO4    = P30
// DUT_GPIO3    = P31
// LED1         = P32
// DUT_GPIO1    = P33
// DUT_GPIO0    = P34
// UDP          = P36
// UDM          = P37

//FW registers
// 0x7FFF:	Interface Mode
// 0x7FFE:	UART baud low word
// 0x7FFD:	UART baud high word
// 0x7FFC:	SPI clk div
// 0x7FFB:	Pin Mode
// 0x7FFA:	Pin Data
// 0x7FF9:	Pin Set
// 0x7FF8:	Pin Clear

UINT8 interface_mode = 1;
UINT16 pin_mode = 0;
UINT32 uart_baud_rate = 125000ul;
UINT8 spi_clk_div = 2;

void fw_write_interface_mode(UINT8 if_mode)
{
	interface_mode = if_mode;
	spi_disable();
	uart1_init(0, UART_1_P16_P17);
	gpio_set_mode(GPIO_MODE_INPUT, GPIO_PORT_1, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
	if(if_mode == 0)
	{
		spi_init(spi_clk_div, SPI_MODE_0);
		gpio_set_mode(GPIO_MODE_PP, GPIO_PORT_1, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
	}
	else if(if_mode == 1)
	{
		spi_init(spi_clk_div, SPI_MODE_3);
		gpio_set_mode(GPIO_MODE_PP, GPIO_PORT_1, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
	}
	else
	{
		uart1_init(uart_baud_rate, UART_1_P16_P17);
		gpio_set_mode(GPIO_MODE_PP, GPIO_PORT_1, GPIO_PIN_7);
	}
}

void fw_write_pin_mode(UINT16 pn_mode)
{
	pin_mode = pn_mode;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_1, GPIO_PIN_1);
	pin_mode = pin_mode >> 2;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_3, GPIO_PIN_0);
	pin_mode = pin_mode >> 2;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_3, GPIO_PIN_1);
	pin_mode = pin_mode >> 2;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_3, GPIO_PIN_2);
	pin_mode = pin_mode >> 2;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_3, GPIO_PIN_3);
	pin_mode = pin_mode >> 2;
	gpio_set_mode(pin_mode & 0x03, GPIO_PORT_3, GPIO_PIN_4);
}

void fw_write_pin_data(UINT8 pin_val)
{
	gpio_write_pin(GPIO_PORT_1, GPIO_PIN_1, pin_val & 0x01);
	pin_val = pin_val >> 1;
	gpio_write_pin(GPIO_PORT_3, GPIO_PIN_0, pin_val & 0x01);
	pin_val = pin_val >> 1;
	gpio_write_pin(GPIO_PORT_3, GPIO_PIN_1, pin_val & 0x01);
	pin_val = pin_val >> 1;
	gpio_write_pin(GPIO_PORT_3, GPIO_PIN_2, pin_val & 0x01);
	pin_val = pin_val >> 1;
	gpio_write_pin(GPIO_PORT_3, GPIO_PIN_3, pin_val & 0x01);
	pin_val = pin_val >> 1;
	gpio_write_pin(GPIO_PORT_3, GPIO_PIN_4, pin_val & 0x01);
}

UINT8 fw_read_pin_data(void)
{
	UINT8 val;
	
	val = gpio_read_pin(GPIO_PORT_3, GPIO_PIN_4);
	val = val << 1;
	val |= gpio_read_pin(GPIO_PORT_3, GPIO_PIN_3);
	val = val << 1;
	val |= gpio_read_pin(GPIO_PORT_3, GPIO_PIN_2);
	val = val << 1;
	val |= gpio_read_pin(GPIO_PORT_3, GPIO_PIN_1);
	val = val << 1;
	val |= gpio_read_pin(GPIO_PORT_3, GPIO_PIN_0);
	val = val << 1;
	val |= gpio_read_pin(GPIO_PORT_1, GPIO_PIN_1);
	
	return val;
}

void fw_pin_set(UINT8 pin_val)
{
	gpio_set_pin(GPIO_PORT_1, (pin_val << 1) & 0x02);
	gpio_set_pin(GPIO_PORT_3, (pin_val >> 1) & 0x1F);
}

void fw_pin_clear(UINT8 pin_val)
{
	gpio_clear_pin(GPIO_PORT_1, (pin_val << 1) & 0x02);
	gpio_clear_pin(GPIO_PORT_3, (pin_val >> 1) & 0x1F);
}

void dut_write_reg_spi(UINT8 addr, UINT16 val)
{
	gpio_clear_pin(GPIO_PORT_1, GPIO_PIN_4);
	
	(void)spi_transfer(addr);
	(void)spi_transfer((UINT8)(val >> 8));
	(void)spi_transfer((UINT8)val);
	
	gpio_set_pin(GPIO_PORT_1, GPIO_PIN_4);
}

UINT16 dut_read_reg_spi(UINT8 addr)
{
	UINT16 read_val;
	
	gpio_clear_pin(GPIO_PORT_1, GPIO_PIN_4);
	(void)spi_transfer(addr);
	(void)spi_transfer(0x00);
	(void)spi_transfer(0x00);
	gpio_set_pin(GPIO_PORT_1, GPIO_PIN_4);
	
	gpio_clear_pin(GPIO_PORT_1, GPIO_PIN_4);
	(void)spi_transfer(addr);
	read_val = spi_transfer(0x00);
	read_val = read_val << 8;
	read_val |= spi_transfer(0x00);
	gpio_set_pin(GPIO_PORT_1, GPIO_PIN_4);
	
	return read_val;
}

void dut_write_request_uart(UINT8 addr, UINT16 val)
{
	UINT8 datagram[3];
	
	datagram[0] = addr;
	datagram[1] = (UINT8)(val >> 8);
	datagram[2] = (UINT8)val;
	
	uart_write_bytes(UART_1, datagram, 3);
}

int main()
{
	UINT8 datagram[4];
	UINT8 prev_control_line_state;
	UINT16 datagram_val;
	
	rcc_set_clk_freq(RCC_CLK_FREQ_16M);
	//Initialize only the SPI and LED pins
	gpio_set_mode(GPIO_MODE_PP, GPIO_PORT_1, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
	gpio_set_mode(GPIO_MODE_INPUT, GPIO_PORT_1, GPIO_PIN_6);
	gpio_set_mode(GPIO_MODE_PP, GPIO_PORT_3, GPIO_PIN_2);
	
	timer_init(TIMER_0, NULL);
	timer_set_period(TIMER_0, FREQ_SYS / 1000ul);	//period is 1ms
	spi_init(spi_clk_div, SPI_MODE_3);
	uart1_init(uart_baud_rate, UART_1_P16_P17);	//dummy init to have buffers in a valid state
	uart1_init(0, UART_1_P16_P17);	//disable UART
	EA = 1;	//enable interupts
	E_DIS = 0;
	
	//Blink LED once
	gpio_clear_pin(GPIO_PORT_3, GPIO_PIN_2);
	timer_long_delay(TIMER_0, 250);
	gpio_set_pin(GPIO_PORT_3, GPIO_PIN_2);
	timer_long_delay(TIMER_0, 250);
	
	cdc_init();
	cdc_set_serial_state(0x03);
	prev_control_line_state = cdc_control_line_state;
	while(!cdc_config);
	timer_long_delay(TIMER_0, 250);
	
	while(TRUE)
	{
		if((cdc_bytes_available() >= 4) && (cdc_peek() & 0x80))	//Handle write datagram
		{
			cdc_read_bytes(datagram, 4);
			datagram_val = datagram[2];
			datagram_val = datagram_val << 8;
			datagram_val |= datagram[3];
			
			if(datagram[0] & 0x7F)	//handle access to FW register
			{
				switch(datagram[1])
				{
					case 0xF8:	//Pin Clear
						fw_pin_clear(datagram[3]);
						break;
					case 0xF9:	//Pin Set
						fw_pin_set(datagram[3]);
						break;
					case 0xFA:	//Pin Data
						fw_write_pin_data(datagram[3]);
						break;
					case 0xFB:	//Pin Mode
						fw_write_pin_mode(datagram_val);
						break;
					case 0xFC:	//SPI clk div
						spi_clk_div = datagram[3];
						break;
					case 0xFD:	//UART baud high word
						uart_baud_rate &= 0x0000FFFF;
						uart_baud_rate |= ((UINT32)datagram_val) << 16;
						break;
					case 0xFE:	//UART baud low word
						uart_baud_rate &= 0xFFFF0000;
						uart_baud_rate |= datagram_val;
						break;
					case 0xFF:	//Interface Mode
						fw_write_interface_mode(datagram[3]);
						break;
				}
			}
			else	//handle access to DUT register
			{
				if(interface_mode > 1)
					dut_write_request_uart(datagram[1] | 0x80, datagram_val);
				else
					dut_write_reg_spi(datagram[1] | 0x80, datagram_val);
			}
		}
		
		if((cdc_bytes_available() >= 2) && !(cdc_peek() & 0x80))	//handle read datagram
		{
			cdc_read_bytes(datagram, 2);
			
			if(datagram[0] & 0x7F)	//handle access to FW register
			{
				switch(datagram[1])
				{
					case 0xF8:	//Pin Clear
					case 0xF9:	//Pin Set
						datagram[2] = 0;
						datagram[3] = 0;
						break;
					case 0xFA:	//Pin Data
						datagram[2] = 0;
						datagram[3] = fw_read_pin_data();
						break;
					case 0xFB:	//Pin Mode
						datagram[2] = (UINT8)(pin_mode >> 8);
						datagram[3] = (UINT8)pin_mode;
						break;
					case 0xFC:	//SPI clk div
						datagram[2] = 0;
						datagram[3] = spi_clk_div;
						break;
					case 0xFD:	//UART baud high word
						datagram[2] = (UINT8)(uart_baud_rate >> 24);
						datagram[3] = (UINT8)(uart_baud_rate >> 16);
						break;
					case 0xFE:	//UART baud low word
						datagram[2] = (UINT8)(uart_baud_rate >> 8);
						datagram[3] = (UINT8)uart_baud_rate;
						break;
					case 0xFF:	//Interface Mode
						datagram[2] = 0;
						datagram[3] = interface_mode;
						break;
				}
				
				datagram[1] = 0xFF;	//tag for FW response
				cdc_write_bytes(datagram + 1, 3);
			}
			else	//handle access to DUT register
			{
				if(interface_mode > 1)
				{
					uart_write_byte(UART_1, datagram[1] & 0x7F);
				}
				else
				{
					datagram_val = dut_read_reg_spi(datagram[1] & 0x7F);
					datagram[1] = 0x80;
					datagram[2] = (UINT8)(datagram_val >> 8);
					datagram[3] = (UINT8)datagram_val;
					cdc_write_bytes(datagram + 1, 3);
				}
			}
		}
		
		if(uart_bytes_available(UART_1) >= 2)	//handle UART read response
		{
			uart_read_bytes(UART_1, datagram + 2, 2);
			datagram[1] = 0x40;
			cdc_write_bytes(datagram + 1, 3);
		}
		
		if(prev_control_line_state != cdc_control_line_state)
		{
			cdc_set_serial_state(cdc_control_line_state & 3);
			prev_control_line_state = cdc_control_line_state;
		}
	}
}
