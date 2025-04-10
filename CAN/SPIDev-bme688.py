import spidev
import time

# BME688 SPI commands
BME688_CHIP_ID = 0x50
BME688_TEMP_MSB = 0x22
BME688_CTRL_MEAS = 0x74
BME688_RESET = 0x60
BME688_SOFT_RESET_CMD = 0xB6
BME688_CALIB_DATA = 0x88  
BME688_GAS_CTRL = 0x71  
EXPECTED_CHIP_ID = 0x61

# Forced mode setting
BME688_FORCED_MODE = 0x01
BME688_OSRS_T = 0x20  # Temperature oversampling x1
BME688_CTRL_MEAS_VALUE = BME688_OSRS_T | BME688_FORCED_MODE
BME688_DISABLE_GAS = 0x00  # Disable gas sensor

# SPI Configuration
SPI_BUS = 0  # Change if using different SPI bus
SPI_DEVICE = 0  # Change if using a different CS pin
spi = spidev.SpiDev()
spi.open(SPI_BUS, SPI_DEVICE)
spi.max_speed_hz = 1000000  # 1MHz
spi.mode = 0b00

def write_register(register, value):
    # Write to a register
    spi.xfer2([register & 0x7F, value])

def read_register(register, length=1):  # length=1 is a default but can be overridden when calling the function
    # Read from a register
    response = spi.xfer2([register | 0x80] + [0x00] * length)
    return response[1:]

def check_chip_id():
    # Check if the chip ID is valid
    chip_id = read_register(BME688_CHIP_ID, 1)[0]
    if chip_id != EXPECTED_CHIP_ID:
        print(f"Error: Invalid Chip ID {chip_id:#X}, expected {EXPECTED_CHIP_ID:#X}")
        return False
    print("BME688 detected successfully.")
    return True

def soft_reset():
    # Soft reset the BME688
    write_register(BME688_RESET, BME688_SOFT_RESET_CMD)
    time.sleep(0.1)  # Wait for reset to complete

def disable_gas_sensor():
    # Disable gas sensor
    write_register(BME688_GAS_CTRL, BME688_DISABLE_GAS)

def set_forced_mode():
    # Set the BME688 to forced mode
    write_register(BME688_CTRL_MEAS, BME688_CTRL_MEAS_VALUE)
    time.sleep(0.2)  # Ensure measurement is completed

def read_calibration_data():
    print("Reading calibration data...")
    # Read calibration coefficients from the BME688
    calib = read_register(BME688_CALIB_DATA, 6)
    dig_T1 = (calib[1] << 8) | calib[0]
    dig_T2 = (calib[3] << 8) | calib[2]
    dig_T3 = calib[4]  
    if dig_T2 > 32767:
        dig_T2 -= 65536
    if dig_T3 > 127:
        dig_T3 -= 256
        print(f"Calibration Parameters: dig_T1={dig_T1}, dig_T2={dig_T2}, dig_T3={dig_T3}")
    return dig_T1, dig_T2, dig_T3

def compensate_temperature(adc_temp, dig_T1, dig_T2, dig_T3):
    # Apply Bosch compensation formula for temperature
    var1 = (((adc_temp / 16384.0) - (dig_T1 / 1024.0)) * dig_T2)
    var2 = (((adc_temp / 131072.0) - (dig_T1 / 8192.0)) * 
            ((adc_temp / 131072.0) - (dig_T1 / 8192.0))) * dig_T3
    t_fine = var1 + var2
    temperature = t_fine / 5120.0
    return temperature

def read_temperature():
    # Read temperature from the BME688 with debugging
    set_forced_mode()
    time.sleep(0.2)  # Increased delay for measurement completion
    raw_temp = read_register(BME688_TEMP_MSB, 3)
    print(f"Raw Temp Data: {raw_temp}")
    print(f"MSB: {raw_temp[0]:#X}, LSB: {raw_temp[1]:#X}, XLSB: {raw_temp[2]:#X}")
    
    # Convert raw data to temperature
    adc_temp = (raw_temp[0] << 12) | (raw_temp[1] << 4) | (raw_temp[2] >> 4)
    print(f"ADC Temp Value: {adc_temp}")
    
    # Read calibration coefficients
    dig_T1, dig_T2, dig_T3 = read_calibration_data()
    print(f"Calibration Data - T1: {dig_T1}, T2: {dig_T2}, T3: {dig_T3}")
    
    # Apply compensation formula
    temperature = compensate_temperature(adc_temp, dig_T1, dig_T2, dig_T3)
    
    return temperature

if __name__ == "__main__":
    if not check_chip_id():
        spi.close()
        exit(1)
    soft_reset()
    disable_gas_sensor()  # Ensure gas sensor is disabled
    time.sleep(0.1)
    print("Reading Temperature...")
    temp = read_temperature()
    print(f"Temperature: {temp:.2f} °C")
    spi.close()
