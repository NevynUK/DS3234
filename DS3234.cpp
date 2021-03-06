// 
//  Implment the methods for the DS3234RealTimeClock class.
//
//#include <SPI.h>
#include "DS3234.h"

//
//  Default constructor.
//
//  Prepare any resources needed by this class.
//
DS3234RealTimeClock::DS3234RealTimeClock()
{
    Initialise(6, MOSI, MISO, SCK);
}

//
//  Make a clock object allowing the user t0 change the chip select pin.
//
DS3234RealTimeClock::DS3234RealTimeClock(const uint8_t chipSelect)
{
    Initialise(chipSelect, MOSI, MISO, SCK);
}

//
//  Make a new DS3234 object allowing the user to define the serial communications pins.
//
DS3234RealTimeClock::DS3234RealTimeClock(const uint8_t chipSelect, const uint8_t mosi, const uint8_t miso, const uint8_t clock)
{
    Initialise(chipSelect, mosi, miso, clock);
}

//
//  Initialise the DS3234 RTC module.
//
void DS3234RealTimeClock::Initialise(const uint8_t chipSelect, const uint8_t mosi, const uint8_t miso, const uint8_t clock)
{
    m_ChipSelect = chipSelect;
    m_MOSI = mosi;
    m_MISO = miso;
    m_Clock = clock;
    m_Port = new BitBang(m_ChipSelect, m_MOSI, m_MISO, m_Clock);
    m_Port->ChipSelect(HIGH);
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(INTCON);
    m_Port->ChipSelect(HIGH);
    memset(m_Registers, 0, REGISTER_SIZE);
    memset(m_SRAM, 0, MEMORY_SIZE);
}

//
//  Destructor should tidy up any memory allocation etc.
//
DS3234RealTimeClock::~DS3234RealTimeClock()
{
    delete(m_Port);
}

//
//  Write a series of bytes to the chip but ignore data from the chip.
//
void DS3234RealTimeClock::BurstTransfer(uint8_t *dataToChip, const uint8_t size)
{
    m_Port->ChipSelect(LOW);
    for (int index = 0; index < size; index++)
    {
        m_Port->WriteRead(dataToChip[index]);
    }
    m_Port->ChipSelect(HIGH);
}

//
//  Transfer a number of bytes in a burst mode.
//
void DS3234RealTimeClock::BurstTransfer(uint8_t *dataToChip, uint8_t *dataFromChip, const uint8_t size)
{   
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(dataToChip, dataFromChip, DATE_TIME_REGISTERS_SIZE + 1);
    m_Port->ChipSelect(HIGH);
}

//
//  Read all of the registers from the DS3234 and stoge them in memory.
//
void DS3234RealTimeClock::ReadAllRegisters()
{
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(0x00);
    for (int index = 0; index < REGISTER_SIZE; index++)
    {
        m_Registers[index] = m_Port->WriteRead(0x00);
    }
    m_Port->ChipSelect(HIGH);
}

//
//  Dump the registers to the serial port.
//
void DS3234RealTimeClock::DumpRegisters(const uint8_t *registers)
{
    Serial.println("      0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f");
    for (int index = 0; index < REGISTER_SIZE; index++)
    {
        if ((index % 0x10) == 0)
        {
            if (index > 0)
            {
                Serial.println("");
            }
            Serial.printf("0x%02x: ", index);
        }
        Serial.printf("0x%02x ", registers[index]);
    }
    Serial.println("");
}

//
//  Read the contents of the SRAM.
//
void DS3234RealTimeClock::ReadSRAM()
{
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(WriteSRAMAddress);
    m_Port->WriteRead(0x00);
    m_Port->ChipSelect(HIGH);
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(ReadSRAMData);
    for (int index = 0; index <= MEMORY_SIZE; index++)
    {
        m_SRAM[index] = m_Port->WriteRead(0x00);
    }
    m_Port->ChipSelect(HIGH);
}

//
//  Dump the contents of the SRAM.
//
void DS3234RealTimeClock::DumpSRAM()
{
    Serial.println("      0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f");
    for (int index = 0; index <= MEMORY_SIZE; index++)
    {
        if ((index % 0x10) == 0)
        {
            if (index > 0)
            {
                Serial.println("");
            }
            Serial.printf("0x%02x: ", index);
        }
        Serial.printf("0x%02x ", m_SRAM[index]);
    }
    Serial.println("");
}

//
//  Get a value from a register.
//
uint8_t DS3234RealTimeClock::GetRegisterValue(const Registers reg)
{
    uint8_t result;

    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(reg);
    result = m_Port->WriteRead(0x00);
    m_Port->ChipSelect(HIGH);
    return(result);
}

//
//  Set the byte value of the specified register.
//
void DS3234RealTimeClock::SetRegisterValue(const Registers reg, const uint8_t value)
{
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(reg);
    m_Port->WriteRead(value);
    m_Port->ChipSelect(HIGH);
}

//
//  Get the current control register.
//
uint8_t DS3234RealTimeClock::GetControlRegister()
{
    return(GetRegisterValue(ReadControl));
}

//
//  Set the control register.
//
//  This method will mask off bits 0,1 and 2 as these are status bits.
//
void DS3234RealTimeClock::SetControlRegister(const uint8_t controlRegister)
{
    SetRegisterValue(WriteControl, controlRegister);
}

//
//  Get the current control/status register.
//
uint8_t DS3234RealTimeClock::GetControlStatusRegister()
{
    return(GetRegisterValue(ReadControlStatus));
}

//
//  Set the control/status register.
//
void DS3234RealTimeClock::SetControlStatusRegister(const uint8_t controlRegister)
{
    SetRegisterValue(WriteControlStatus, controlRegister & 0xf8);
}

//
//  Get the value from the Aging Offset register.
//
uint8_t DS3234RealTimeClock::GetAgingOffset()
{
    return(GetRegisterValue(ReadAgingOffset));
}

//
//  Set the aging offset register.
//
void DS3234RealTimeClock::SetAgingOffset(uint8_t agingOffset)
{
    SetRegisterValue(WriteAgingOffset, agingOffset);
}

//
//  Get the last temperature reading.
//
float DS3234RealTimeClock::GetTemperature()
{
    uint8_t dataToChip[3], dataFromChip[3];
    uint16_t temperature;

    memset(dataFromChip, 0, 3);
    memset(dataToChip, 0, 3);
    dataToChip[0] = 0x11;
    BurstTransfer(dataToChip, dataFromChip, 3);
    temperature = (dataFromChip[1] << 2) | (dataFromChip[2] >> 6);
    return(temperature * 0.25);
}

//
//  Get the date and time from the DS3234 and return a ts structure containing
//  the decoded time.
//
ts *DS3234RealTimeClock::GetDateTime()
{
    uint8_t dataToChip[DATE_TIME_REGISTERS_SIZE + 1], dataFromChip[DATE_TIME_REGISTERS_SIZE + 1];
    ts *result;

    //
    //  The trick here is to send one more byte than necessary.  The first byte
    //  contains the address of the first register we want to read, in this case
    //  0x00 (so we don't need to explicitly set it).  The subsequent bytes written
    //  to the chip will be ignored and we are accessing the data from the registers.
    //
    memset(dataToChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    memset(dataFromChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    BurstTransfer(dataToChip, dataFromChip, DATE_TIME_REGISTERS_SIZE + 1);
    //
    //  We should now have the date/time in dataFromChip[1..DATE_TIME_REGISTERS_SIZE + 1].
    //
    result = new(ts);
    result->seconds = ConvertBCDToUint8(dataFromChip[1]);
    result->minutes = ConvertBCDToUint8(dataFromChip[2]);
    if (dataFromChip[3] & 0x40)
    {
        result->hour = ConvertBCDToUint8(dataFromChip[3] & 0x1f);
        if (dataFromChip[3] & 0x20)
        {
            result->hour += 12;
        }
    }
    else
    {
        result->hour = ConvertBCDToUint8(dataFromChip[3] & 0x3f);
    }
    result->wday = dataFromChip[4];
    result->day = ConvertBCDToUint8(dataFromChip[5]);
    result->month = ConvertBCDToUint8(dataFromChip[6] & 0x7f);
    result->year = 1900 + ConvertBCDToUint8(dataFromChip[7]);
    if (dataFromChip[6] & 0x80)
    {
        result->year += 100;
    }
    return(result);
}

//
//  Set the date and time.
//
void DS3234RealTimeClock::SetDateTime(ts *dateTime)
{
    uint8_t dataToChip[DATE_TIME_REGISTERS_SIZE + 1], dataFromChip[DATE_TIME_REGISTERS_SIZE + 1];

    //
    //  The trick here is to send one more byte than necessary.  The first byte
    //  contains the address of the first register we want to read, in this case
    //  0x80.  The subsequent bytes contain the encoded date and time.
    //
    memset(dataToChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    memset(dataFromChip, 0, DATE_TIME_REGISTERS_SIZE + 1);
    dataToChip[0] = 0x80;
    dataToChip[1] = ConvertUint8ToBCD(dateTime->seconds);
    dataToChip[2] = ConvertUint8ToBCD(dateTime->minutes);
    dataToChip[3] = ConvertUint8ToBCD(dateTime->hour);           // Using 24 hour notation.
    dataToChip[4] = dateTime->wday;
    dataToChip[5] = ConvertUint8ToBCD(dateTime->day);
    dataToChip[6] = ConvertUint8ToBCD(dateTime->month);
    if (dateTime->year > 1999)
    {
        dataToChip[6] |= 0x80;
        dataToChip[7] = ConvertUint8ToBCD((dateTime->year - 2000) & 0xff);
    }
    else
    {
        dataToChip[7] = ConvertUint8ToBCD((dateTime->year - 1900) & 0xff);
    }
    //
    //  Now we can set the time.
    //
    BurstTransfer(dataToChip, dataFromChip, DATE_TIME_REGISTERS_SIZE + 1);
}

//
//  Check the alarm status flags to work out which alarm has just triggered an interrupt.
//
DS3234RealTimeClock::Alarm DS3234RealTimeClock::WhichAlarm()
{
    uint8_t controlStatusRegister;

    controlStatusRegister = GetControlStatusRegister();
    if ((controlStatusRegister & 0x01) && (controlStatusRegister & 0x02))
    {
        return(DS3234RealTimeClock::Both);
    }
    if (controlStatusRegister & 0x01)
    {
        return(DS3234RealTimeClock::Alarm1);
    }
    if (controlStatusRegister & 0x02)
    {
        return(DS3234RealTimeClock::Alarm2);
    }
    return(DS3234RealTimeClock::Unknown);
}

//
//  Set the date and time of the specified alarm.
//
void DS3234RealTimeClock::SetAlarm(Alarm alarm, ts *time, AlarmType type)
{
    uint8_t dataToChip[5];
    int element = 1;
    uint8_t amount;
    uint8_t a1 = 0, a2 = 0, a3 = 0, a4 = 0;

    if (alarm == Alarm1)
    {
        dataToChip[0] = WriteAlarm1;
        dataToChip[1] = ConvertUint8ToBCD(time->seconds);
        element = 2;
        amount = 6;
    }
    else
    {
        dataToChip[0] = WriteAlarm2;
        amount = 5;
    }
    dataToChip[element++] = ConvertUint8ToBCD(time->minutes);
    dataToChip[element++] = ConvertUint8ToBCD(time->hour);
    if ((type == WhenDayHoursMinutesMatch) || (type == WhenDayHoursMinutesSecondsMatch))
    {
        dataToChip[element] = ConvertUint8ToBCD(time->wday) | 0x40;
    }
    else
    {
        dataToChip[element] = ConvertUint8ToBCD(time->day);
    }
    switch (type)
    {
        //
        //  Alarm 1 interrupts.
        //
        case OncePerSecond:
            dataToChip[1] |= 0x80;
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenSecondsMatch:
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenMinutesSecondsMatch:
            dataToChip[3] |= 0x80;
            dataToChip[4] |= 0x80;
            break;
        case WhenHoursMinutesSecondsMatch:
            dataToChip[4] |= 0x80;
            break;
        case WhenDateHoursMinutesSecondsMatch:
            break;
        case WhenDayHoursMinutesSecondsMatch:
            dataToChip[4] |= 0x40;
            break;
        //
        //  Alarm 2 interupts.
        //
        case OncePerMinute:
            dataToChip[1] |= 0x80;
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            break;
        case WhenMinutesMatch:
            dataToChip[2] |= 0x80;
            dataToChip[3] |= 0x80;
            break;
        case WhenHoursMinutesMatch:
            dataToChip[3] |= 0x80;
            break;
        case WhenDateHoursMinutesMatch:
            break;
        case WhenDayHoursMinutesMatch:
            dataToChip[3] |= 0x40;
            break;
    }
    BurstTransfer(dataToChip, amount);
    SetControlRegister(INTCON | A1IE);
}

//
//  Enable or disable the specified alarm.
//
void DS3234RealTimeClock::EnableDisableAlarm(const Alarm alarm, const bool enable)
{
    uint8_t controlRegister;

    controlRegister = GetControlRegister();
    if (alarm == Alarm1)
    {
        if (enable)
        {
            controlRegister |= A1IE;
        }
        else
        {
            controlRegister &= ~A1IE;
        }
    }
    else
    {
        if (enable)
        {
            controlRegister |= A2IE;
        }
        else
        {
            controlRegister &= ~A2IE;
        }
    }
    SetControlRegister(controlRegister);
}

//
//  Process the interrupts generated by the alarms.
//
void DS3234RealTimeClock::InterruptHandler()
{
}

//
//  Clear the interrupt flag for the specified alarm.
//
void DS3234RealTimeClock::ClearInterrupt(Alarm alarm)
{
    uint8_t controlStatusRegister;

    controlStatusRegister = GetControlStatusRegister();
    if (alarm == Alarm1)
    {
        controlStatusRegister &= ~A1F;
    }
    else
    {
        controlStatusRegister &= ~A2F;
    }
    SetControlStatusRegister(controlStatusRegister);
}

//
//  Convert a BCD number into an integer.
//
uint8_t DS3234RealTimeClock::ConvertBCDToUint8(const uint8_t value)
{
    uint8_t result;

    result = (value & 0xf0) >> 4;
    result *= 10;
    result += (value & 0x0f);
    return (result);
}

//
//  Convert a number to BCD format.
//
uint8_t DS3234RealTimeClock::ConvertUint8ToBCD(const uint8_t value)
{
    uint8_t result;

    result = value % 10;
    result |= (value / 10) << 4;
    return (result);
}

//
//  Get the current date and time as a string.
//
String DS3234RealTimeClock::DateTimeString(ts *theTime)
{
    char buff[MAX_BUFFER_SIZE];

    sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", theTime->day, theTime->month, theTime->year, theTime->hour, theTime->minutes, theTime->seconds);
    return(buff);
}

//
//  Write a byte into the DS3234 SRAM.
//
void DS3234RealTimeClock::WriteToSRAM(const uint8_t address, const uint8_t data)
{
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(WriteSRAMAddress);
    m_Port->WriteRead(address);
    m_Port->ChipSelect(HIGH);
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(WriteSRAMData);
    m_Port->WriteRead(data);
    m_Port->ChipSelect(HIGH);
}

//
//  Read a byte from the SRAM.
//
uint8_t DS3234RealTimeClock::ReadFromSRAM(const uint8_t address)
{
    uint8_t result;

    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(WriteSRAMAddress);
    m_Port->WriteRead(address);
    m_Port->ChipSelect(HIGH);
    m_Port->ChipSelect(LOW);
    m_Port->WriteRead(ReadSRAMData);
    result = m_Port->WriteRead(0x00);
    m_Port->ChipSelect(HIGH);
    return(result);
}