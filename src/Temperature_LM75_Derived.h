#include "Temperature_LM75_Derived_Attributes.h"

#include <Arduino.h>
#include <Wire.h>

#ifndef TEMPERATURE_LM75_DERIVED_H
#define TEMPERATURE_LM75_DERIVED_H

struct Temperature_LM75_Derived {
	// The layout of registers accessed through the I2C protocol.
	// Attributes about a device or family of devices (if the attributes are shared).
	stxp	uint8_t 		DEFAULT_I2C_ADDRESS				= 0x48;	// The typical I2C address for any device.

	TwoWire 				* bus							= {};
	uint8_t 				i2c_address						= {};
	uint8_t 				Resolution						= {};
	uint16_t 				resolution_mask					= {};
	uint8_t 				Temperature_frac_width			= {};
	float 					temperature_frac_factor			= {};
	const sai::Attributes 	* attributes					= {};

							Temperature_LM75_Derived		(TwoWire *bus, uint8_t i2c_address, const sai::Attributes * attributes) {
		this->bus				= bus;
		this->i2c_address		= i2c_address;
		this->attributes		= attributes;
		setInternalResolution(attributes->default_temperature_resolution);
		setInternalTemperatureFracWidth(attributes->default_temperature_frac_width);
	}
	int16_t					readIntegerTemperatureRegister	(uint8_t register_index);
	void					writeIntegerTemperatureRegister	(uint8_t register_index, int16_t value);
	ndsx	float 			convertCtoF						(float c)		{ return c * 1.8 + 32; }
	ndsx	float 			convertFtoC						(float f)		{ return (f - 32) / 1.8; }
	inline float 			convertIntegerTemperature		(int16_t value)	{ return (float)value * temperature_frac_factor; }
	inline int16_t 			convertFloatTemperature			(float value)	{ return (int16_t)(value / temperature_frac_factor); }
	inline float 			readTemperatureC				()				{ return convertIntegerTemperature(readIntegerTemperatureRegister(attributes->registers->temperature)); }
	inline float 			readTemperatureF				()				{ return convertCtoF(readTemperatureC()); }
	inline float 			readTemperatureLowC				()				{ return convertIntegerTemperature(readIntegerTemperatureRegister(attributes->registers->temperature_low)); }
	inline float 			readTemperatureLowF				()				{ return convertCtoF(readTemperatureLowC()); }
	inline float 			readTemperatureHighC			()				{ return convertIntegerTemperature(readIntegerTemperatureRegister(attributes->registers->temperature_high)); }
	inline float 			readTemperatureHighF			()				{ return convertCtoF(readTemperatureHighC()); }
	inline void 			setTemperatureLowC				(float value)	{ writeIntegerTemperatureRegister(attributes->registers->temperature_low, convertFloatTemperature(value)); }
	inline void 			setTemperatureHighC				(float value)	{ writeIntegerTemperatureRegister(attributes->registers->temperature_high, convertFloatTemperature(value)); }
	inline void 			setTemperatureHighF				(float value)	{ setTemperatureHighC(convertFtoC(value)); }
	inline void 			setTemperatureLowF				(float value)	{ setTemperatureLowC(convertFtoC(value)); }
	// Set the internal resolution of the temperature sensor, which affects conversions and which bits are discarded.
	void					setInternalResolution			(uint8_t resolution) {
		this->Resolution		= resolution;
		this->resolution_mask	= ~(uint16_t)((1 << (attributes->temperature_width - Resolution)) - 1);
	}
	void					setInternalTemperatureFracWidth	(uint8_t temperature_frac_width) {
		Temperature_frac_width	= temperature_frac_width;
		temperature_frac_factor	= 1.0 / (float)(1 << Temperature_frac_width);
	}
};

struct Generic_LM75_Compatible : public Temperature_LM75_Derived {
	enum ConfigurationBits {
		FaultQueueSize				= 3, // mask 0x18, length 2 bits
		AlertPolarity				= 2, // mask 0x04, length 1 bit
		ThermostatMode				= 1, // mask 0x02, length 1 bit
		Shutdown					= 0, // mask 0x01, length 1 bit
	};
	enum FaultQueueLength
		{ FaultQueueLength_1_fault	= 0
		, FaultQueueLength_2_faults	= 1
		, FaultQueueLength_4_faults	= 2
		, FaultQueueLength_6_faults	= 3
		};

	inline				Generic_LM75_Compatible			(TwoWire *bus, uint8_t i2c_address, const sai::Attributes *attributes)	: Temperature_LM75_Derived(bus, i2c_address, attributes) { }

	uint8_t				readConfigurationRegister		();
	void				writeConfigurationRegister		(uint8_t configuration);
	void				setConfigurationBits			(uint8_t bits);
	void				clearConfigurationBits			(uint8_t bits);
	void				setConfigurationBitValue		(uint8_t value, uint8_t start, uint8_t width);
	ndin	uint8_t		readConfigurationBits			(uint8_t bits)															{ return readConfigurationRegister() & bits; }
	ndin	bool		checkConfigurationBits			(uint8_t bits)															{ return (readConfigurationRegister() & bits) == bits; }
	inline	void		setFaultQueueLength				(enum FaultQueueLength faults)											{ setConfigurationBitValue	(faults, FaultQueueSize, 2);	}
	inline	void		setAlertActiveLow				()																		{ clearConfigurationBits	(bit(AlertPolarity));	}
	inline	void		setAlertActiveHigh				()																		{ setConfigurationBits		(bit(AlertPolarity));	}
	inline	void		setThermostatComparatorMode		()																		{ clearConfigurationBits	(bit(ThermostatMode));	}
	inline	void		setThermostatInterruptMode		()																		{ setConfigurationBits		(bit(ThermostatMode));	}
	inline	void		enableShutdownMode				()																		{ setConfigurationBits		(bit(Shutdown));	}
	inline	void		disableShutdownMode				()																		{ clearConfigurationBits	(bit(Shutdown));	}
};
struct Generic_LM75 : public Generic_LM75_Compatible {
	inline				Generic_LM75			(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_Compatible(bus, i2c_address, &sai::Generic_LM75_Attributes) { };
	inline				Generic_LM75			(uint8_t i2c_address)												: Generic_LM75_Compatible(&Wire, i2c_address, &sai::Generic_LM75_Attributes) { };
};
struct Generic_LM75_10Bit : public Generic_LM75_Compatible {
	inline				Generic_LM75_10Bit		(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_Compatible(bus, i2c_address, &sai::Generic_LM75_10Bit_Attributes) { };
	inline				Generic_LM75_10Bit		(uint8_t i2c_address)												: Generic_LM75_Compatible(&Wire, i2c_address, &sai::Generic_LM75_10Bit_Attributes) { };
};
struct Generic_LM75_11Bit : public Generic_LM75_Compatible  {
	inline				Generic_LM75_11Bit		(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_Compatible(bus, i2c_address, &sai::Generic_LM75_11Bit_Attributes) { };
	inline				Generic_LM75_11Bit		(uint8_t i2c_address)												: Generic_LM75_Compatible(&Wire, i2c_address, &sai::Generic_LM75_11Bit_Attributes) { };
};
struct Generic_LM75_12Bit : public Generic_LM75_Compatible {
	inline				Generic_LM75_12Bit		(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_Compatible(bus, i2c_address, &sai::Generic_LM75_12Bit_Attributes) { };
	inline				Generic_LM75_12Bit		(uint8_t i2c_address)												: Generic_LM75_Compatible(&Wire, i2c_address, &sai::Generic_LM75_12Bit_Attributes) { };
};
struct OnSemi_NCTx75 : public Generic_LM75_12Bit {
	enum ConfigurationBits
		{ Shutdown			= 0	// Mask 0x01, length 1 bit
		, OneShot			= 5	// Mask 0x20, length 1 bit
		};
	inline				OnSemi_NCTx75			(TwoWire *bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_12Bit(bus, i2c_address) { };
	inline				OnSemi_NCTx75			(uint8_t i2c_address)												: Generic_LM75_12Bit(&Wire, i2c_address) { };

	void				enableOneShotMode		() {
		setConfigurationBits	(bit(ConfigurationBits::OneShot));
		clearConfigurationBits	(bit(ConfigurationBits::Shutdown));
	}
	void				startOneShotConversion	() {
		bus->beginTransmission(i2c_address);
		bus->write(0x04); // OneShot trigger register
		bus->write(1); // Writing anything triggers the oneshot
		bus->endTransmission();
	}
};
struct Generic_LM75_9_to_12Bit_Compatible : public Generic_LM75_Compatible {
	enum ConfigurationBits	{ ConfigurationByteWidth = 5, }; // mask 0x60, length 2 bits
	enum ByteWidth
		{	ByteWidth_9_bits	= 0
		,	ByteWidth_10_bits	= 1
		,	ByteWidth_11_bits	= 2
		,	ByteWidth_12_bits	= 3
		};
	inline				Generic_LM75_9_to_12Bit_Compatible			(TwoWire * bus, uint8_t i2c_address, const sai::Attributes * attributes)	: Generic_LM75_Compatible(bus, i2c_address, attributes) { };
	inline	void		setResolution								(ByteWidth resolution)														{ setConfigurationBitValue(resolution, ConfigurationByteWidth, 2); }
};
struct Generic_LM75_9_to_12Bit : public Generic_LM75_9_to_12Bit_Compatible {
	inline				Generic_LM75_9_to_12Bit						(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)			: Generic_LM75_9_to_12Bit_Compatible(bus, i2c_address, &sai::Generic_LM75_12Bit_Attributes) { };
	inline				Generic_LM75_9_to_12Bit						(uint8_t i2c_address)														: Generic_LM75_9_to_12Bit_Compatible(&Wire, i2c_address, &sai::Generic_LM75_12Bit_Attributes) { };
};
struct Generic_LM75_9_to_12Bit_OneShot_Compatible : public Generic_LM75_9_to_12Bit_Compatible {
	enum ConfigurationBits { ConfigurationOneShot = 7, }; // mask 0x80, length 1 bit
	inline				Generic_LM75_9_to_12Bit_OneShot_Compatible	(TwoWire *bus, uint8_t i2c_address, const sai::Attributes * attributes)		: Generic_LM75_9_to_12Bit_Compatible(bus, i2c_address, attributes) { };
	inline	void		startOneShotConversion						()																			{ setConfigurationBits(bit(ConfigurationOneShot)); }
	ndin	bool		checkConversionReady						()																			{ return checkConfigurationBits(bit(ConfigurationOneShot)); }
};
struct Generic_LM75_9_to_12Bit_OneShot : public Generic_LM75_9_to_12Bit_OneShot_Compatible {
	inline				Generic_LM75_9_to_12Bit_OneShot				(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)			: Generic_LM75_9_to_12Bit_OneShot_Compatible(bus  , i2c_address, &sai::Generic_LM75_12Bit_Attributes) { }
	inline				Generic_LM75_9_to_12Bit_OneShot				(uint8_t i2c_address)														: Generic_LM75_9_to_12Bit_OneShot_Compatible(&Wire, i2c_address, &sai::Generic_LM75_12Bit_Attributes) { }
};
struct TI_TMP102_Compatible : public Generic_LM75_9_to_12Bit_OneShot_Compatible {
	enum ExtendedConfiguration
		{ ConfigurationFrequency		= 6 // 0xc0 // 6 // mask length 2 bits
		, ConfigurationAlert			= 5 // 0x20 // 5 // mask length 1 bit
		, ConfigurationExtendedMode		= 4 // 0x10 // 4 // mask length 1 bit
		};
	enum ConversionFrequency
		{ ConversionFrequency_0_25Hz	= 0
		, ConversionFrequency_1_Hz		= 1
		, ConversionFrequency_4_Hz		= 2
		, ConversionFrequency_8_Hz		= 3
		};
	inline				TI_TMP102_Compatible				(TwoWire * bus = &Wire, uint8_t i2c_address = DEFAULT_I2C_ADDRESS)	: Generic_LM75_9_to_12Bit_OneShot_Compatible(bus, i2c_address, &sai::TI_TMP102_Attributes) { };
	inline				TI_TMP102_Compatible				(uint8_t i2c_address)												: Generic_LM75_9_to_12Bit_OneShot_Compatible(&Wire, i2c_address, &sai::TI_TMP102_Attributes) { };

	inline	void		setConversionRate                 	(ConversionFrequency rate)											{ setExtendedConfigurationBitValue(rate, ConfigurationFrequency, 2); }
	ndin	bool		checkAlert                        	()																	{ return checkExtendedConfigurationBits(bit(ConfigurationAlert)); }
	ndin	bool		checkExtendedConfigurationBits    	(uint16_t bits) 													{ return (readExtendedConfigurationRegister() & bits) == bits; }
	ndin	uint16_t	readExtendedConfigurationBits     	(uint16_t bits) 													{ return readExtendedConfigurationRegister() & bits; }

	uint16_t			readExtendedConfigurationRegister 	();
	void				writeExtendedConfigurationRegister	(uint16_t configuration);
	void				setExtendedConfigurationBits      	(uint16_t bits);
	void				clearExtendedConfigurationBits    	(uint16_t bits);
	void				setExtendedConfigurationBitValue  	(uint16_t value, uint8_t start, uint8_t width);
	void				enableExtendedMode                	();
	void				disableExtendedMode               	();

};

typedef	Generic_LM75_9_to_12Bit				Maxim_DS1775        	;
typedef	Generic_LM75_9_to_12Bit				Maxim_DS7505        	;
typedef	Generic_LM75_9_to_12Bit				Maxim_DS75LV        	;
typedef	Generic_LM75_9_to_12Bit				Maxim_DS75LX        	;
typedef	Generic_LM75_9_to_12Bit_OneShot		Microchip_AT30TS750A	;
typedef	Generic_LM75_9_to_12Bit_OneShot		Microchip_MCP9800   	;
typedef	Generic_LM75_9_to_12Bit_OneShot		Microchip_MCP9801   	;
typedef	Generic_LM75_9_to_12Bit_OneShot		Microchip_MCP9802   	;
typedef	Generic_LM75_9_to_12Bit_OneShot		Microchip_MCP9803   	;
typedef	Generic_LM75_11Bit					NXP_LM75A           	;
typedef	Generic_LM75_11Bit					NXP_LM75B           	;
typedef	Generic_LM75_11Bit					NXP_PCT2075         	;
typedef	Generic_LM75_12Bit					NXP_SE95            	;
typedef	OnSemi_NCTx75						ON_NCT75            	;
typedef	OnSemi_NCTx75						ON_NCT375           	;
typedef	Generic_LM75						ST_STCN75           	;
typedef	Generic_LM75						ST_STLM75           	;
typedef	Generic_LM75_9_to_12Bit_OneShot		ST_STTS75           	;
typedef	Generic_LM75						TI_LM75             	;
typedef	Generic_LM75						TI_LM75A            	;
typedef	Generic_LM75						TI_LM75B            	;
typedef	Generic_LM75						TI_LM75C            	;
typedef	Generic_LM75_9_to_12Bit_OneShot		TI_TMP100           	;
typedef	Generic_LM75_9_to_12Bit_OneShot		TI_TMP101           	;
typedef	TI_TMP102_Compatible				TI_TMP102           	;
typedef	TI_TMP102_Compatible				TI_TMP112           	;
typedef	Generic_LM75_9_to_12Bit_OneShot		TI_TMP175           	;
typedef	Generic_LM75_9_to_12Bit_OneShot		TI_TMP275           	;
typedef	Generic_LM75_9_to_12Bit_OneShot		TI_TMP75            	;

#endif // TEMPERATURE_LM75_DERIVED_H
