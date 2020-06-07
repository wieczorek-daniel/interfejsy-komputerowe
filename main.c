/* ------------------------------------ Biblioteki z WiringPI ------------------------------------*/
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include<wiringPiSPI.h>

/* ------------------------------------ Biblioteki systemowe ------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

/* ------------------------------------------- DEFINE -------------------------------------------*/
#define LAST(a,c) ((a) & ((1<<(c))-1))
#define MID(a,b,c) LAST((a)>>(b),((c)-(b)))

/* -------------------------------------- Definicja funkcji -------------------------------------*/
void menuShow();
void menuHandle();
void tempRead();
double* accelRead();
void gyroRead();
void levelRead(double accelXOut, double accelZOut);
void dateTimeRead();
void dateTimeWrite();
uint16_t eepromRead(uint16_t userAddress);
void eepromWrite(uint16_t userAddress, uint16_t userValue);
void tempPreasureRead();
void authorsShow();
void optionsShow();
void finalize();

/* ---------------------------------------- Funkcja glowna --------------------------------------*/
int main()
{
    system("clear");
    menuShow();
    menuHandle();
    
    return 0;
}

/* ---------------------------------------- Inicjalizacja ----------------------------------------*/
void menuShow()
{
    puts(
        "╔════════════════════════════════════════════════════════════╗\n"
        "║                      Wybierz funkcje                       ║\n"
        "╠════════════════════════════════════════════════════════════╣\n"
        "║ 1. Odczyt temperatury (MPU6050)                            ║\n"
        "║ 2. Odczyt pomiarow z akcelerometru i zyroskopu             ║\n"
        "║ 3. Odczyt daty i czasu z modulu RTC                        ║\n"
        "║ 4. Zapis nowej daty i czasu do modulu RTC                  ║\n"
        "║ 5. Odczyt wartosci z danego adresu pamieci EEPROM          ║\n"
        "║ 6. Zapis wartosci do danego adresu pamieci EEPROM          ║\n"
        "║ 7. Odczyt temperatury i cisnienia atmosferycznego (BMP280) ║\n"
        "║ 8. Wyjscie z programu                                      ║\n"
        "║ 9. Informacje o autorach                                   ║\n"
        "║ 0. Dostepne opcje                                          ║\n"
        "╚════════════════════════════════════════════════════════════╝\n"
    );
}

void menuHandle()
{
    bool wrongChoice = true;
    while(wrongChoice)
    {
        int userChoice;
        printf("Wybierz opcje z menu: ");
        scanf("%d", &userChoice);
        
        switch(userChoice){
            case 1:
                wrongChoice = false;
                system("clear");
                tempRead();
                finalize();
                break;
            case 2:
                wrongChoice = false;
                system("clear");
                double* accelOut = accelRead();
                gyroRead();
                levelRead(accelOut[0], accelOut[2]);
                finalize();
                break;
            case 3:
                wrongChoice = false;
                system("clear");
				dateTimeRead();
                finalize();
                break;
            case 4:
                wrongChoice = false;
                system("clear");
				dateTimeWrite();
                finalize();
                break;
            case 5:
                wrongChoice = false;
                system("clear");
				uint16_t userAddressToRead;
                printf("Podaj adres komorki pamieci EEPROM spod jakiego chcesz odczytac wartosc: ");
                scanf("%hu", &userAddressToRead);
                uint16_t valueEEPROM = eepromRead(userAddressToRead);
	            printf("Odczytana wartosc z komorki %hu pamieci EEPROM: %hu\n", userAddressToRead, valueEEPROM);
                finalize();
                break;
            case 6:
                wrongChoice = false;
                system("clear");
                uint16_t userAddressToWrite, userValue;
                printf("Podaj adres komorki pamieci EEPROM do jakiej chcesz zapisac wartosc: ");
                scanf("%hu", &userAddressToWrite);
                printf("Podaj wartosc jaka chcesz zapisac: ");
                scanf("%hu", &userValue);
                eepromWrite(userAddressToWrite, userValue);
	            printf("W komorce %hu zapisano wartosc %hu\n", userAddressToWrite, userValue);
                finalize();
                break;
            case 7:
                wrongChoice = false;
                system("clear");
                tempPreasureRead();
                finalize();
                break;
            case 8:
                wrongChoice = false;
                system("clear");
                printf("Koniec dzialania programu.");
		        exit(0);
                break;
            case 9:
                wrongChoice = false;
                system("clear");
                authorsShow();
                finalize();
                break;
            case 0:
                wrongChoice = false;
                system("clear");
                optionsShow();
                finalize();
		break;
            default:
                printf("Opcja jest nieprawidlowa.\n");
                break;
        }
    }    
}

/* ------------------------------------------- Opcja 1 -------------------------------------------*/
// Pomiar temperatury
void tempRead()
{	
    int fd;
	int addressMPU6050 = 0x69;
	
	if(wiringPiSetup() == -1)
		exit(1);

	if((fd=wiringPiI2CSetup(addressMPU6050)) == -1){
		printf("Blad podczas inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: MPU6050\r\n");

	// Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B; 
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	int regTempOutH = 0x41;
	int regTempOutL = 0x42;
	int16_t temp[2] = { 0 };
	int16_t tempOut;
	double tempOutCelcius;
	temp[0] = wiringPiI2CReadReg8(fd, regTempOutH);
	temp[1] = wiringPiI2CReadReg8(fd, regTempOutL);
	tempOut = temp[0] << 8 | temp[1];
	tempOutCelcius = tempOut / 340. + 36.53;
	printf("TEMP_OUT = %f *C\n", tempOutCelcius);
}

/* ------------------------------------------- Opcja 2 -------------------------------------------*/
// Akcelerometr
double* accelRead()
{
    int fd;
	int addressMPU6050 = 0x69;
	
	if(wiringPiSetup() == -1)
		exit(1);

	if((fd=wiringPiI2CSetup(addressMPU6050)) == -1){
		printf("Blad podczas inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: MPU6050\r\n");

    // Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B;
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	int regAccelXOutH = 0x3B;
	int regAccelXOutL = 0x3C;
	int regAccelYOutH = 0x3D;
	int regAccelYOutL = 0x3E;
	int regAccelZOutH = 0x3F;
	int regAccelZOutL = 0x40;
	int16_t accelX[2] = { 0 };
	int16_t accelY[2] = { 0 };
	int16_t accelZ[2] = { 0 };
	int16_t accelOutRaw[3] = { 0 };
	double* accelOut = malloc(sizeof(double) * 3);
	double accelSens = 16384.;
	accelX[0] = wiringPiI2CReadReg8(fd, regAccelXOutH);
	accelX[1] = wiringPiI2CReadReg8(fd, regAccelXOutL);
	accelOutRaw[0] = accelX[0] << 8 | accelX[1];
	accelOut[0] = accelOutRaw[0] / accelSens;
	accelY[0] = wiringPiI2CReadReg8(fd, regAccelYOutH);
	accelY[1] = wiringPiI2CReadReg8(fd, regAccelYOutL);
	accelOutRaw[1] = accelY[0] << 8 | accelY[1];
	accelOut[1] = accelOutRaw[1] / accelSens;
	accelZ[0] = wiringPiI2CReadReg8(fd, regAccelZOutH);
	accelZ[1] = wiringPiI2CReadReg8(fd, regAccelZOutL);
	accelOutRaw[2] = accelZ[0] << 8 | accelZ[1];
	accelOut[2] = accelOutRaw[2] / accelSens;
	printf("ACCEL_X_OUT = %f g, ACCEL_Y_OUT = %f g, ACCEL_Z_OUT = %f g\n", accelOut[0], accelOut[1], accelOut[2]);

	return accelOut;
}

// Zyroskop cyfrowy
void gyroRead()
{
    int fd;
	int addressMPU6050 = 0x69;
	
	if(wiringPiSetup() == -1)
		exit(1);

	if((fd=wiringPiI2CSetup(addressMPU6050)) == -1){
		printf("Blad podczas inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: MPU6050\r\n");

    // Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B;
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	int regGyroXOutH = 0x43;
	int regGyroXOutL = 0x44;
	int regGyroYOutH = 0x45;
	int regGyroYOutL = 0x46;
	int regGyroZOutH = 0x47;
	int regGyroZOutL = 0x48;
	int16_t gyroX[2] = { 0 };
	int16_t gyroY[2] = { 0 };
	int16_t gyroZ[2] = { 0 };
	int16_t gyroOutRaw[3] = { 0 };
	double gyroOut[3] = { 0 };
	double gyroSens = 131.;
	gyroX[0] = wiringPiI2CReadReg8(fd, regGyroXOutH);
	gyroX[1] = wiringPiI2CReadReg8(fd, regGyroXOutL);
	gyroOutRaw[0] = gyroX[0] << 8 | gyroX[1];
	gyroOut[0] = gyroOutRaw[0] / gyroSens;
	gyroY[0] = wiringPiI2CReadReg8(fd, regGyroYOutH);
	gyroY[1] = wiringPiI2CReadReg8(fd, regGyroYOutL);
	gyroOutRaw[1] = gyroY[0] << 8 | gyroY[1];
	gyroOut[1] = gyroOutRaw[1] / gyroSens;
	gyroZ[0] = wiringPiI2CReadReg8(fd, regGyroZOutH);
	gyroZ[1] = wiringPiI2CReadReg8(fd, regGyroZOutL);
	gyroOutRaw[2] = gyroZ[0] << 8 | gyroZ[1];
	gyroOut[2] = gyroOutRaw[2] / gyroSens;
	printf("GYRO_X_OUT = %f */s, GYRO_Y_OUT = %f */s, GYRO_Z_OUT = %f */s\n", gyroOut[0], gyroOut[1], gyroOut[2]);
}

// Poziomica cyfrowa
void levelRead(double accelXOut, double accelZOut)
{
    int fd;
	int addressMPU6050 = 0x69;
	
	if(wiringPiSetup() == -1)
		exit(1);

	if((fd=wiringPiI2CSetup(addressMPU6050)) == -1){
		printf("Blad podczas inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: MPU6050\r\n");

    // Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B;
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	double level = (atan2(accelXOut, accelZOut)) * (180 / M_PI);
	printf("Poziom = %f *\n", level);
}

/* ------------------------------------------- Opcja 3 -------------------------------------------*/
void dateTimeRead()
{
    int fd;
	int addressDS1307 = 0x68;

	if (wiringPiSetup() == -1)
		exit(1);

	if ((fd = wiringPiI2CSetup(addressDS1307)) == -1) {
		printf("Blad podczas inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: DS1307\r\n");

    // Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B;
	wiringPiI2CWriteReg8(fd, regPWR, 0);


	int regSecond = 0x00;
	int regMinute = 0x01;
	int regHour = 0x02;
	int regDay = 0x04;
	int regMonth = 0x05;
	int regYear = 0x06;

	int timeSecond = wiringPiI2CReadReg8(fd, regSecond);
	int timeSecond10 = MID(timeSecond, 4, 7);
	int timeSecond1 = MID(timeSecond, 0, 4);

	int timeMinute = wiringPiI2CReadReg8(fd, regMinute);
	int timeMinute10 = MID(timeMinute, 4, 7);
	int timeMinute1 = MID(timeMinute, 0, 4);

	int timeHour = wiringPiI2CReadReg8(fd, regHour);
	int timeHour10;
	int timeHour1;

	if (!(MID(timeHour, 6, 7))) 
	{ 
		timeHour10 = MID(timeHour, 4, 6); 
	}
	else
	{
		timeHour10 = MID(timeHour, 4, 5);
	}
	timeHour1 = MID(timeHour, 0, 4);

	int dateDay = wiringPiI2CReadReg8(fd, regDay);
	int dateDay10 = MID(dateDay, 4, 6);
	int dateDay1 = MID(dateDay, 0, 4);

	int dateMonth = wiringPiI2CReadReg8(fd, regMonth);
	int dateMonth10 = MID(dateMonth, 4, 5);
	int dateMonth1 = MID(dateMonth, 0, 4);

	int dateYear = wiringPiI2CReadReg8(fd, regYear);
	int dateYear10 = MID(dateYear, 4, 8);
	int dateYear1 = MID(dateYear, 0, 4);

	printf("Odczyt daty i czasu:\n20%d%d-%d%d-%d%d %d%d:%d%d:%d%d\n", dateYear10, dateYear1, dateMonth10, dateMonth1, dateDay10, dateDay1, timeHour10, timeHour1, timeMinute10, timeMinute1, timeSecond10, timeSecond1);
}

/* ------------------------------------------- Opcja 4 -------------------------------------------*/
void dateTimeWrite()
{
    int fd;
	int addressDS1307 = 0x68;

	if (wiringPiSetup() == -1)
		exit(1);

	if ((fd = wiringPiI2CSetup(addressDS1307)) == -1) {
		printf("Blad inicjalizacji I2C");
		exit(1);
	}
	printf("Modul I2C: DS1307\r\n");

    // Uruchamia pomiary z adresem rejestru PWR_MGMT_1
	int regPWR = 0x6B;
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	int regSecond = 0x00;
	int regMinute = 0x01;
	int regHour = 0x02;
	int regDay = 0x04;
	int regMonth = 0x05;
	int regYear = 0x06;

	printf("Zapis daty i czasu:\nWpisz date i czas w nastepujacym formacie YY-MM-DD_HH:MM:SS: ");
	char dateTime[17];
	scanf("%s", dateTime);

	int ch = wiringPiI2CReadReg8(fd, regSecond);
	ch = ch >> 7;

	int timeHour1 = 0;
	if (isdigit(dateTime[10])) { timeHour1 = dateTime[10] - 48; }
	int timeMinute1 = 0;
	if (isdigit(dateTime[13])) { timeMinute1 = dateTime[13] - 48; }
	int timeSecond1 = 0;
	if (isdigit(dateTime[16])) { timeSecond1 = dateTime[16] - 48; }
	int timeHour10 = 0;
	if (isdigit(dateTime[9])) { timeHour10 = dateTime[9] - 48; }
	int timeMinute10 = 0;
	if (isdigit(dateTime[12])) { timeMinute10 = dateTime[12] - 48; }
	int timeSecond10 = 0;
	if (isdigit(dateTime[15])) { timeSecond10 = dateTime[15] - 48; }

	int timeHour = (0 << 6) | (timeHour10 << 4) | timeHour1;
	int timeMinute = (timeMinute10 << 4) | timeMinute1;
	int timeSecond = (ch << 7) | (timeSecond10 << 4) | timeSecond1;

	wiringPiI2CWriteReg8(fd, regHour, timeHour);
	wiringPiI2CWriteReg8(fd, regMinute, timeMinute);
	wiringPiI2CWriteReg8(fd, regSecond, timeSecond);

	int dateDay1 = 0;
	if (isdigit(dateTime[7])) { dateDay1 = dateTime[7] - 48; }
	int dateMonth1 = 0;
	if (isdigit(dateTime[4])) { dateMonth1 = dateTime[4] - 48; }
	int dateYear1 = 0;
	if (isdigit(dateTime[1])) { dateYear1 = dateTime[1] - 48; }
	int dateDay10 = 0;
	if (isdigit(dateTime[6])) { dateDay10 = dateTime[6] - 48; }
	int dateMonth10 = 0;
	if (isdigit(dateTime[3])) { dateMonth10 = dateTime[3] - 48; }
	int dateYear10 = 0;
	if (isdigit(dateTime[0])) { dateYear10 = dateTime[0] - 48; }

	int dateDay = (dateDay10 << 4) | dateDay1;
	int dateMonth = (dateMonth10 << 4) | dateMonth1;
	int dateYear = (dateYear10 << 4) | dateYear1;

	wiringPiI2CWriteReg8(fd, regDay, dateDay);
	wiringPiI2CWriteReg8(fd, regMonth, dateMonth);
	wiringPiI2CWriteReg8(fd, regYear, dateYear);

	printf("Data i czas zostały pomyslnie zapisane\n");
}

/* ------------------------------------------- Opcja 5 -------------------------------------------*/
uint8_t i2cReadEEPROM(int fd, uint16_t address) {
	wiringPiI2CWriteReg8(fd, (address > 8), (address & 0xFF));
	delay(20);
	return wiringPiI2CRead(fd);
}

uint16_t eepromRead(uint16_t userAddress)
{
    int fd;
    int addressEEPROM = 0x50;

	if (wiringPiSetup() == -1)
		exit(1);

	if ((fd = wiringPiI2CSetup(addressEEPROM)) == -1) {
		printf("Blad inicjalizacji I2C");
		exit(1);
	}
	printf("Start I2C: odczyt z EEPROM\r\n");

    uint16_t valueH = i2cReadEEPROM(fd, userAddress) << 8;
    uint16_t valueL = i2cReadEEPROM(fd, userAddress + 1);
	uint16_t value = valueH | valueL; 
	delay(20);

	return value;
}

/* ------------------------------------------- Opcja 6 -------------------------------------------*/
void i2cWriteEEPROM(int fd, uint16_t address, uint16_t value) {
	wiringPiI2CWriteReg16(fd, (address >> 8), (value << 8) | (address & 0xFF));
	delay(20);
}

void eepromWrite(uint16_t userAddress, uint16_t userValue)
{
    int fd;
    int addressEEPROM = 0x50;

	if (wiringPiSetup() == -1)
		exit(1);

	if ((fd = wiringPiI2CSetup(addressEEPROM)) == -1)
	{
		printf("Blad inicjalizacji I2C");
		exit(1);
	}
	printf("Start I2C: zapis do EEPROM\r\n");
    
    uint8_t valueH = (userValue & 0xFF00) >> 8; 
    uint8_t valueL = userValue & 0x00FF; 
	i2cWriteEEPROM(fd, userAddress, valueH); 
    i2cWriteEEPROM(fd, userAddress + 1, valueL);

	delay(20);
}

/* ------------------------------------------- Opcja 7 -------------------------------------------*/
int chan = 0;
int speed = 1000000;
int t_fine;

enum 
{
    BMP280_REGISTER_DIG_T1 = 0x88,
    BMP280_REGISTER_DIG_T2 = 0x8A,
    BMP280_REGISTER_DIG_T3 = 0x8C,
    BMP280_REGISTER_DIG_P1 = 0x8E,
    BMP280_REGISTER_DIG_P2 = 0x90,
    BMP280_REGISTER_DIG_P3 = 0x92,
    BMP280_REGISTER_DIG_P4 = 0x94,
    BMP280_REGISTER_DIG_P5 = 0x96,
    BMP280_REGISTER_DIG_P6 = 0x98,
    BMP280_REGISTER_DIG_P7 = 0x9A,
    BMP280_REGISTER_DIG_P8 = 0x9C,
    BMP280_REGISTER_DIG_P9 = 0x9E,
    BMP280_REGISTER_CHIPID = 0xD0,
    BMP280_REGISTER_VERSION = 0xD1,
    BMP280_REGISTER_SOFTRESET = 0xE0,
    BMP280_REGISTER_CAL26 = 0xE1, /* R calibration = 0xE1-0xF0 */
    BMP280_REGISTER_STATUS = 0xF3,
    BMP280_REGISTER_CONTROL = 0xF4,
    BMP280_REGISTER_CONFIG = 0xF5,
    BMP280_REGISTER_PRESSUREDATA = 0xF7,
    BMP280_REGISTER_TEMPDATA = 0xFA,
};

typedef struct 
{
    uint16_t dig_T1; /* dig_T1 cal register */
    int16_t dig_T2;  /* dig_T2 cal register */
    int16_t dig_T3;  /* dig_T3 cal register */
    uint16_t dig_P1; /* dig_P1 cal register */
    int16_t dig_P2;  /* dig_P2 cal register */
    int16_t dig_P3;  /* dig_P3 cal register */
    int16_t dig_P4;  /* dig_P4 cal register */
    int16_t dig_P5;  /* dig_P5 cal register */
    int16_t dig_P6;  /* dig_P6 cal register */
    int16_t dig_P7;  /* dig_P7 cal register */
    int16_t dig_P8;  /* dig_P8 cal register */
    int16_t dig_P9;  /* dig_P9 cal register */
} 
bmp280_calib_data;
bmp280_calib_data calib_data;

int writeRegister(int address, int data) 
{
    unsigned char buff[2];
    buff[0] = (address& ~0x80);
    buff[1] = data;
    wiringPiSPIDataRW(chan, buff, 2);

    return 1;
}

int readRegister(int address)
{
    unsigned char buff[2];
    buff[0] = (address | 0x80);
    wiringPiSPIDataRW(chan, buff, 2);

    return buff[1];
}

int init()
{
    short unsigned wartosc=0;
    // Reset
    wartosc=0xb6;
    writeRegister(BMP280_REGISTER_SOFTRESET, wartosc);
	// Ustawienie bitów t_sb - czas miedzy pomiarami 250ms w trybie SPI 4-wire bez filtru
	int t_standby = 0b01100000;
	// Aktualizacja ustawien rejestru BMP280_REGISTER_CONFIG
    writeRegister(BMP280_REGISTER_CONFIG, t_standby);
	// Ustawienie ctrl_meas - tryb: Normal, oversampling: x2 dla temperatury i x16 dla cisnienia     
    int settingsBMP = 0b01010111;  
	// Aktualizacja ustawien rejestru BMP280_REGISTER_CONTROL
    writeRegister(BMP280_REGISTER_CONTROL, settingsBMP);   
    delay(1000); 

    return 0;
}

// Funkcja do odczytu temperatury z dokumentacji (strona 22)
long signed int bmp280_compensate_T_int32(long signed int adc_T)
{   
    long signed int var1, var2, T;
    var1 = ((((adc_T>>3) - ((long signed int)calib_data.dig_T1<<1))) * ((long signed int)calib_data.dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((long signed int)calib_data.dig_T1)) * ((adc_T>>4) - ((long signed int)calib_data.dig_T1))) >> 12) * ((long signed int)calib_data.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;
}

// Funkcja do odczytu cisnienia z dokumentacji (strona 22)
long unsigned int bmp280_compensate_P_int64(long signed int adc_P)
{  
    long long signed int var1, var2, p;
    var1 = ((long long signed int)t_fine) - 128000;
    var2 = var1 * var1 * (long long signed int)calib_data.dig_P6;
    var2 = var2 + ((var1*(long long signed int)calib_data.dig_P5)<<17);
    var2 = var2 + (((long long signed int)calib_data.dig_P4)<<35);
    var1 = ((var1 * var1 *(long long signed int)calib_data.dig_P3)>>8) + ((var1 * (long long signed int)calib_data.dig_P2)<<12);
    var1 = (((((long long signed int)1)<<47)+var1))*((long long signed int)calib_data.dig_P1)>>33;
    if (var1 == 0)
	{
        return 0;
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((long long signed int)calib_data.dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((long long signed int)calib_data.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((long long signed int)calib_data.dig_P7)<<4);

    return (long unsigned int)p;
}

// Odczyt ID modułu, temperatury i cisnienia z modulu BMP280
void tempPreasureRead()
{
	if (wiringPiSPISetup(chan, speed) == -1)
    {
        printf("Blad inicjalizacji SPI\n");
        exit(1);
    }

	init();
	delay(600);

	int id = readRegister(BMP280_REGISTER_CHIPID);
	printf("Czujnik BMP280 - ChipID: %x\r\n",id);

	calib_data.dig_T1 = readRegister(BMP280_REGISTER_DIG_T1) | readRegister(BMP280_REGISTER_DIG_T1 + 1) << 8 ;
    calib_data.dig_T2 = readRegister(BMP280_REGISTER_DIG_T2) | readRegister(BMP280_REGISTER_DIG_T2 + 1) << 8 ;
    calib_data.dig_T3 = readRegister(BMP280_REGISTER_DIG_T3) | readRegister(BMP280_REGISTER_DIG_T3 + 1) << 8 ;
    calib_data.dig_P1 = readRegister(BMP280_REGISTER_DIG_P1) | readRegister(BMP280_REGISTER_DIG_P1 + 1) << 8 ;
    calib_data.dig_P2 = readRegister(BMP280_REGISTER_DIG_P2) | readRegister(BMP280_REGISTER_DIG_P2 + 1) << 8 ;
    calib_data.dig_P3 = readRegister(BMP280_REGISTER_DIG_P3) | readRegister(BMP280_REGISTER_DIG_P3 + 1) << 8 ;
    calib_data.dig_P4 = readRegister(BMP280_REGISTER_DIG_P4) | readRegister(BMP280_REGISTER_DIG_P4 + 1) << 8 ;
    calib_data.dig_P5 = readRegister(BMP280_REGISTER_DIG_P5) | readRegister(BMP280_REGISTER_DIG_P5 + 1) << 8 ;
    calib_data.dig_P6 = readRegister(BMP280_REGISTER_DIG_P6) | readRegister(BMP280_REGISTER_DIG_P6 + 1) << 8 ;
    calib_data.dig_P7 = readRegister(BMP280_REGISTER_DIG_P7) | readRegister(BMP280_REGISTER_DIG_P7 + 1) << 8 ;
    calib_data.dig_P8 = readRegister(BMP280_REGISTER_DIG_P8) | readRegister(BMP280_REGISTER_DIG_P8 + 1) << 8 ;
    calib_data.dig_P9 = readRegister(BMP280_REGISTER_DIG_P9) | readRegister(BMP280_REGISTER_DIG_P9 + 1) << 8 ;

    int measurements[6];
    for (int i=0; i<6; i++)
	{
        measurements[i] = readRegister(0xF7 + i);
    }

    long signed int tempOut = measurements[3]<<12 | measurements[4]<<4 | (measurements[5]>>4);
    tempOut = bmp280_compensate_T_int32(tempOut);
    float tempOutCelcius = (float)tempOut/100;   

    long signed int preasureOut = measurements[0]<<12 | measurements[1]<<4 | (measurements[2]>>4);
    preasureOut = bmp280_compensate_P_int64(preasureOut);
    float preasure = (float)preasureOut/256/100;

    printf("Temperatura: %.2f *C\r\n", tempOutCelcius);
    printf("Cisnienie: %.2f hPa\r\n", preasure);
}

/* ------------------------------------------- Opcja 9 -------------------------------------------*/
void authorsShow()
{
    puts(
        "╔═════════╦═══════════╦════════════╗\n"
        "║  Imie   ║ Nazwisko  ║ Nr indeksu ║\n"
        "╠═════════╬═══════════╬════════════╣\n"
        "║ Kajetan ║  Swiatly  ║   241204   ║\n"
        "║ Daniel  ║ Wieczorek ║   241203   ║\n"
        "╚═════════╩═══════════╩════════════╝\n"  
    );
    
    finalize();
}

/* ------------------------------------------- Opcja 0 -------------------------------------------*/
void optionsShow()
{
    puts(
        "╔══════════╦═════════════════════════════════════════════════════════════════╗\n"
        "║ Nr opcji ║  	                          Opis                               ║\n"
        "╠══════════╬═════════════════════════════════════════════════════════════════╣\n"
        "║     1    ║ Odczyt temperatury z ukladu MPU6050                             ║\n"
        "║     2    ║ Odczyt pomiarow z akcelerometru i zyroskopu z ukladu MPU6050    ║\n"
        "║     3    ║ Odczyt daty i czasu z modulu RTC z układu DS1307                ║\n"
        "║     4    ║ Zapis nowej daty i czasu do modulu RTC z układu DS1307          ║\n"
        "║     5    ║ Odczyt wartosci z danego adresu pamieci EEPROM z układu AT24C32 ║\n"
        "║     6    ║ Zapis wartosci do danego adresu pamieci EEPROM z układu AT24C32 ║\n"
        "║     7    ║ Odczyt temperatury i cisnienia atmosferycznego z układu BMP280  ║\n"
        "║     8    ║ Wyjscie z programu                                              ║\n"
        "║     9    ║ Informacje o autorach                                           ║\n"
        "║     0    ║ Dostepne opcje                                                  ║\n"
        "╚══════════╩═════════════════════════════════════════════════════════════════╝\n"
    );
}

/* ----------------------------------------- Finalizacja -----------------------------------------*/
void finalize()
{
    bool wrongChoice = true;
    while(wrongChoice)
    {
	    int userChoice;
    	printf("\nWcisnij 1, aby wrocic do menu lub 0, aby zakonczyc program: ");
	    scanf("%d", &userChoice);
	    switch(userChoice){
            case 0:
                wrongChoice = false;
                system("clear");
		        exit(0);
                break;
            case 1:
                wrongChoice = false;
                system("clear");
		        menuShow();
    		    menuHandle();
                break;
	        default:
                printf("Opcja jest nieprawidlowa.\n");
                break;
	    }
    }
}